#pragma once
#include <vector>
#include "unicode.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

namespace bb { namespace search {

struct SearchLocationInfo
{
	tstring m_dir_path;
	std::vector<tstring> m_includes;
	std::vector<tstring> m_excludes;
	bool m_recursive;
	bool m_follow_symlinks;

	SearchLocationInfo () { }

	SearchLocationInfo (tstring const & dir, tstring const & includes, tstring const & excludes, bool rec)
		: m_dir_path(dir)
		, m_includes()
		, m_excludes()
		, m_recursive(rec)
		, m_follow_symlinks(false)
	{
		std::vector<tstring> tok;
		split(tok, includes, boost::is_any_of(L";"));
		for(std::vector<tstring>::iterator tok_iter = tok.begin(); tok_iter != tok.end(); ++tok_iter)
			m_includes.push_back(*tok_iter);
		tok.clear();
		split(tok, excludes, boost::is_any_of(L";\n"));
		for (std::vector<tstring>::iterator tok_iter = tok.begin(); tok_iter != tok.end(); ++tok_iter)
		{
			tstring tmp(*tok_iter);
			boost::trim_right_if(tmp, boost::is_any_of(L"\\"));
			m_excludes.push_back(tmp);
		}
	}
};

}}

