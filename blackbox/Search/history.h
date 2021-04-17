#pragma once
#include <vector>
#include <windows.h>
#include <algorithm>
#include "unicode.h"
#include "serialize.h"

namespace bb { namespace search {

struct HistoryItem
{
	tstring m_typed;
	tstring m_fname;
	tstring m_fpath;
	tstrings m_args;
	unsigned m_ref;

	HistoryItem () : m_ref(0) { }
};

inline bool operator== (HistoryItem const & lhs, HistoryItem const & rhs)
{
	return lhs.m_typed == rhs.m_typed && lhs.m_fname == rhs.m_fname && lhs.m_fpath == rhs.m_fpath;
}

inline std::ostream & write (std::ostream & os, HistoryItem const & t)
{
	if (!write(os, t.m_typed)) return os;
	if (!write(os, t.m_fname)) return os;
	if (!write(os, t.m_fpath)) return os;
	if (!write(os, t.m_args)) return os;
	if (!write(os, t.m_ref)) return os;
	return os;
}
inline std::istream & read (std::istream & is, HistoryItem & t)
{
	if (!read(is, t.m_typed)) return is;
	if (!read(is, t.m_fname)) return is;
	if (!read(is, t.m_fpath)) return is;
	if (!read(is, t.m_args)) return is;
	if (!read(is, t.m_ref)) return is;
	return is;
}


struct History
{
	typedef std::vector<HistoryItem> items_t;
	items_t m_items;
	tstring m_path;
	tstring m_name;

	History (tstring const & path, tstring const & name) : m_path(path), m_name(name) { }
	bool IsLoaded () const { return m_items.size() > 0; }

	bool Find (tstring const & what, std::vector<HistoryItem *> & results, size_t max_results = 128)
	{
		results.clear();
		results.reserve(max_results);
		for (size_t i = 0; i < m_items.size(); ++i)
		{
			if (m_items[i].m_typed == what)
			{
				results.push_back(&m_items[i]);
				continue;
			}
			if (m_items[i].m_fname == what)
			{
				results.push_back(&m_items[i]);
				continue;
			}
		}
		return results.size() > 0;
	}

	void DeleteFiles ()
	{
		tstring const item_fpath = m_path + m_name + TEXT(".props");
		DeleteFile(item_fpath.c_str());
	}

	bool Insert (tstring const & typed, tstring const & fname, tstring const & fpath)
	{
		HistoryItem hi;
		hi.m_typed = typed;
		hi.m_fname = fname;
		hi.m_fpath = fpath;
		for (size_t i = 0; i < m_items.size(); ++i)
		{
			if (m_items[i] == hi)
			{
				++m_items[i].m_ref;
				return true;
			}
		}
		hi.m_ref = 1;
		m_items.push_back(hi);
		return false;
	}

	bool Remove (tstring const & typed, tstring const & fname, tstring const & fpath)
	{
		HistoryItem hi;
		hi.m_typed = typed;
		hi.m_fname = fname;
		m_items.erase(std::remove_if(m_items.begin(), m_items.end()
					, [&fname, &fpath] (HistoryItem const & lhs) { return lhs.m_fname == fname && lhs.m_fpath == fpath; }), m_items.end());
		return true;
	}

	void Clear () { m_items.clear(); }

	//bool Suggest (tstring const & what, std::vector<tstring> & keywords, std::vector<tstring> & results, size_t max_results = 128)
	bool Load ();
	bool Save ();
protected:
	bool load (items_t & t, tstring const & fpath);
	bool save (items_t const & t, tstring const & fpath);
};

}}

