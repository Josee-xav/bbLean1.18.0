#pragma once
#include <windows.h>
#include <worker.h>
#include "history.h"
#include "index.h"
#include "jobmanager.h"
#include "rc.h"

namespace bb { namespace search {

struct ProgramLookup;
ProgramLookup & getLookup();
void startLookup (tstring const & path);
void stopLookup ();
void reconfigureLookup ();
void reindexLookup ();
void abortReindexLookup ();
void loadConfig (tstring const & path, Config & cfg);

struct RebuildJob : Runnable
{
	bb::search::Index & m_index;
	std::atomic<bool> m_finished;

	RebuildJob (bb::search::Index & i) : m_index(i), m_finished(false) { }
	virtual void Run ()
	{
		m_index.Rebuild();
		m_finished.store(true, std::memory_order_relaxed);
	}
	bool IsFinished () const
	{
		return m_finished.load(std::memory_order_relaxed);
	}

	void Restart ()
	{
		m_finished.store(false, std::memory_order_relaxed);
	}
};

struct ProgramLookup
{
	tstring m_path;
	History m_history;
	Index m_index;
	JobManager m_jobs;
	RebuildJob m_job;
	bool m_indexing;

	ProgramLookup (tstring const & path, Config const & cfg)
		: m_path(path)
		, m_history(path, TEXT("programs_history"))
		, m_index(path, TEXT("programs"), cfg)
		, m_job(m_index)
		, m_indexing(false)
	{ }

	void Stop ()
	{
		m_jobs.Stop(); // calls Reset also
	}

	bool Load ()
	{
		m_history.Load();
		return m_index.Load();
	}

	bool IsLoaded () const { return m_index.IsLoaded(); }

	bool IsIndexing () const
	{
		return m_indexing && !m_job.IsFinished();
	}

	void AbortIndexing ()
	{
		m_index.AbortIndexing();
	}

	void Reindex ()
	{
		Clear();
		m_history.DeleteFiles();
		m_index.DeleteFiles();
		ReloadOrBuild();
	}

	void Clear ()
	{
		m_history.Clear();
		m_index.Clear();
	}

	void ReloadOrBuild ()
	{
		if (IsIndexing())
		{
			AbortIndexing();
			while (!m_job.IsFinished())
				Sleep(1);
		}

		Clear();

		Config cfg;
		loadConfig(m_path, cfg);
		m_index.m_cfg.clear();
		m_index.m_cfg = cfg;

		LoadOrBuild(false);
	}

	bool LoadOrBuild (bool sync = false)
	{
		m_history.Load();
		if (!m_index.Load())
		{
			if (m_jobs.size() == 0)
				m_jobs.Create(1);
			m_indexing = true;
			m_job.Restart();
			m_jobs.AddJob(&m_job);
			if (sync)
			{
				unsigned c = 0;
				while (!m_job.IsFinished())
					delayExecution(c);
				m_indexing = false;
			}
			return false;
		}
		return true;
	}

	bool Save ()
	{
		if (m_history.Save())
			if (m_index.Save())
				return true;
		return false;
	}

	bool Find (tstring const & what, tstrings & hkeys, tstrings & hres, tstrings & ikeys, tstrings & ires)
	{
		std::vector<HistoryItem *> history_res; //@TODO: unlocal
		history_res.reserve(32);
		bool found_some = false;

		if (m_history.Find(what, history_res, 8))
		{
			for (HistoryItem const * h : history_res)
				hkeys.push_back(h->m_fname);
			for (HistoryItem const * h : history_res)
				hres.push_back(h->m_fpath);
			found_some = true;
		}
		tstring what_lwr = what;
		boost::algorithm::to_lower(what_lwr);
		if (m_index.Suggest(what_lwr, ikeys, ires, 64))
		{
			found_some = true;
		}
		return found_some;
	}
};

}}


