#pragma once
#include <functional>
#include <windows.h>
#include <vector>
#include "unicode.h"
#include <../3rd_party/cedar/cedar.h>
#include "serialize.h"
#include "config.h"
#include "rc.h"

namespace bb { namespace search {

struct Props
{
	tstring m_fname; /// filename part only
	std::vector<tstring> m_fpath; /// file name with complete path

	Props (tstring const & fname, tstring const & fpath)
		: m_fname(fname)
	{
		m_fpath.push_back(fpath);
	}
	Props () { }
};
inline std::istream & read (std::istream & is, Props & t)
{
	if (!read(is, t.m_fname)) return is;
	if (!read(is, t.m_fpath)) return is;
	return is;
}
inline std::ostream & write (std::ostream & os, Props const & t)
{
	if (!write(os, t.m_fname)) return os;
	if (!write(os, t.m_fpath)) return os;
	return os;
}

typedef std::vector<Props> props_t; // vecor
typedef cedar::da<int> trie_t;
typedef tstrings forget_t;

void makeIndex (trie_t & trie, props_t & props, bool & abort, Config const & cfg);
bool loadIndex (trie_t & t, tstring const & fpath);
bool saveIndex (trie_t const & t, tstring const & fpath);
bool searchIndex (trie_t & t, tstring const & str, std::function<void(tstring const &, tstring const &)> on_match);
bool loadProps (props_t & t, tstring const & fpath);
bool saveProps (props_t const & t, tstring const & fpath);
bool loadForget (forget_t & t, tstring const & fpath);
bool saveForget (forget_t const & t, tstring const & fpath);

struct Index
{
	trie_t m_trie;
	props_t m_props;
	forget_t m_forget; //@TODO: better container
	tstring m_path;
	tstring m_name;
	Config m_cfg;
	bool m_abort;

	Index (tstring const & path, tstring const & name, Config const & cfg) : m_path(path), m_name(name), m_cfg(cfg), m_abort(false) { }
	bool IsLoaded () const { return m_props.size() > 0; }

	bool Forget (tstring const & s)
	{
		m_forget.push_back(s);
		return true;
	}

	bool RemoveFromIndex (tstring const & fname, tstring const & fpath)
	{
		for (size_t i = 0; i < m_props.size(); ++i) // @TODO: efficiency
		{
			if (m_props[i].m_fname == fname)
			{
				tstrings & paths = m_props[i].m_fpath;
				paths.erase(std::remove_if(paths.begin(), paths.end()
							, [&fpath] (tstring const & lhs) { return boost::iequals(lhs, fpath); }), paths.end());

				// @NOTE: following erase crashes, *sigh*
				//if (paths.size() == 0)
				//	m_trie.erase(fname.c_str());
				return true;
			}
		}
		return false;
	}

	bool Find (tstring const & what, std::vector<tstring> & results, size_t max_results = 128)
	{
		results.clear();
		results.reserve(max_results);
		return SearchIndex(what, max_results,
				[&results] (tstring const & fname, tstring const & fpath)
				{
					results.push_back(fpath);
				});
	}

	bool Suggest (tstring const & what, std::vector<tstring> & keywords, std::vector<tstring> & results, size_t max_results = 128)
	{
		results.clear();
		results.reserve(max_results);
		keywords.clear();
		keywords.reserve(max_results);
		return SearchIndex(what, max_results,
				[&results, &keywords] (tstring const & fname, tstring const & fpath)
				{
					keywords.push_back(fname);
					results.push_back(fpath);
				});
	}

	bool Load ()
	{
		tstring const fgt_fpath = m_path + m_name + TEXT(".forget");
		loadForget(m_forget, fgt_fpath);
		tstring const idx_fpath = m_path + m_name + TEXT(".index");
		tstring const prop_fpath = m_path + m_name + TEXT(".props");
		if (loadIndex(m_trie, idx_fpath))
			if (loadProps(m_props, prop_fpath))
			{
				_tprintf(TEXT("Load OK.\n"));
				return true;
			}

		m_trie.clear();
		m_props.clear();
		return false;
	}

	bool Save ()
	{
		tstring const idx_fpath = m_path + m_name + TEXT(".index");
		saveIndex(m_trie, idx_fpath);
		tstring const prop_fpath = m_path + m_name + TEXT(".props");
		saveProps(m_props, prop_fpath);
		SaveForget();
		return true; // ehm
	}

	bool SaveForget ()
	{
		tstring const fgt_fpath = m_path + m_name + TEXT(".forget");
		saveForget(m_forget, fgt_fpath);
		return true; // ehm
	}

	void Clear ()
	{
		m_trie.clear();
		m_props.clear();
		m_forget.clear();
	}

	void DeleteFiles ()
	{
		tstring const idx_fpath = m_path + m_name + TEXT(".index");
		DeleteFile(idx_fpath.c_str());
		tstring const prop_fpath = m_path + m_name + TEXT(".props");
		DeleteFile(prop_fpath.c_str());
	}

	void AbortIndexing ()
	{
		m_abort = true;
	}

	bool Rebuild ()
	{
		//_tprintf(TEXT("*** Rebuilding index ***\n"));
		m_abort = false;
		m_trie.clear();
		m_props.clear();
		m_cfg.clear();
		loadConfig(m_path, m_cfg);
		makeIndex(m_trie, m_props, m_abort, m_cfg);
		if (m_abort)
		{
			m_trie.clear();
			m_props.clear();
			DeleteFiles();
			m_abort = false;
		}
		else
		{
			Save();
		}
		return IsLoaded();
	}

protected:
	bool SearchIndex (tstring const & str, size_t max_results, std::function<void(tstring const &, tstring const &)> on_match)
	{
		if (!IsLoaded())
			return false;

		/*trie_t::result_pair_type * const result_pair = static_cast<trie_t::result_pair_type *>(alloca(max_results * sizeof(trie_t::result_pair_type)));
		if (const size_t n = m_trie.commonPrefixSearch(str.c_str(), result_pair, max_results))
		{
			for (size_t i = 0; i < n && i < max_results; ++i)
			{
				//m_trie.suffix(suffix, result_triple[i].length, result_triple[i].id);
				for (tstring const & s : m_props[result_pair[i].value].m_fpath)
					dbg_printf("found %s:%ld", m_props[result_pair[i].value].m_fname, result_pair[i].length);
					//on_match(m_props[result_triple[i].value].m_fname, s);
			}
			//return true;
		}*/

		trie_t::result_triple_type * const result_triple = static_cast<trie_t::result_triple_type *>(alloca(max_results * sizeof(trie_t::result_triple_type)));
		TCHAR suffix[1024];
		if (const size_t n = m_trie.commonPrefixPredict(str.c_str(), result_triple, max_results))
		{
			for (size_t i = 0; i < n && i < max_results; ++i)
			{
				m_trie.suffix(suffix, result_triple[i].length, result_triple[i].id);
				for (tstring const & s : m_props[result_triple[i].value].m_fpath)
				{
					//dbg_printf("%s: prefix found, [%d/%d] %s\n", str.c_str(), n, i, s.c_str());
					on_match(m_props[result_triple[i].value].m_fname, s);
				}
			}
			return true;
		}
		return false;
	}
};

inline std::istream & read (std::istream & is, Index & t)
{
	unsigned v = 0;
	if (!read(is, v)) return is;
	if (v != version11) return is;

	if (!read(is, t.m_props)) return is;
	return is;
}
inline std::ostream & write (std::ostream & os, Index const & i)
{
	if (!write(os, version11)) return os;
	if (!write(os, i.m_props)) return os;
	return os;
}

}}

