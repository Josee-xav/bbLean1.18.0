#pragma once
#include <algorithm>
#include <functional>
#include <vector>
#include <windows.h>
#include "unicode.h"
#include "regex.h"
#include "SearchLocationInfo.h"
#include <boost/algorithm/string/predicate.hpp>

namespace bb { namespace search {

inline bool matchExclude (std::vector<tstring> const & excludes, tstring const & file_name)
{
	for (tstring const & ws : excludes)
	{
		if (boost::iequals(ws, file_name))
			return true;
	}
	return false;
}

struct RAII
{
	HANDLE & m_handle;
	RAII (HANDLE & h) : m_handle(h) { }
	~RAII ()
	{
		if (m_handle != INVALID_HANDLE_VALUE)
		{
			::FindClose(m_handle); // Close handle
			m_handle = INVALID_HANDLE_VALUE;
		}
	}
};

inline int SearchDirectory (
				  tstring const & dir_path, std::vector<tstring> const & includes, std::vector<tstring> const & excludes, bool recursive, bool follow_symlinks
				, tstring const & file_name
				, std::function<bool(tstring const &, tstring const &)> compare
				, std::function<void(tstring const &, tstring const &)> on_match
				, bool & abortFlag)
{
	if (abortFlag) return ERROR_PRINT_CANCELLED;

	tstring filePath;
	tstring pattern = dir_path;
	if (!boost::algorithm::ends_with(pattern, TEXT("\\")))
		pattern += TEXT("\\");
	pattern += TEXT("*.*");

	WIN32_FIND_DATA fi;
	HANDLE hFile = ::FindFirstFile(pattern.c_str(), &fi);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		RAII destroy_on_return(hFile); // raii guard

		do
		{
			if (abortFlag) return ERROR_PRINT_CANCELLED;

			if (fi.cFileName[0] != '.')
			{
				filePath.erase();
				filePath = dir_path;
				if (!boost::algorithm::ends_with(filePath, TEXT("\\")))
					filePath += TEXT("\\");
				filePath += fi.cFileName;

				if (fi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) // @TODO: follow symlinks
				{
					if (recursive)
					{
						if (!matchExclude(excludes, filePath))
						{
							// descend into subdirectory
							if (int const iRC = SearchDirectory(filePath, includes, excludes, recursive, follow_symlinks, file_name, compare, on_match, abortFlag))
								return iRC;
						}
					}
				}
				else
				{
					if (includes.size() > 0)
					{
						for (tstring const & ws : includes)
						{
							if (abortFlag) return ERROR_PRINT_CANCELLED;

							if (std::regex_match(fi.cFileName, tregex(ws, std::tr1::regex_constants::icase)))
							{
								if (compare(fi.cFileName, file_name))
									on_match(fi.cFileName, filePath);
							}
						}
					}
					else
					{
						if (compare(fi.cFileName, file_name))
							on_match(fi.cFileName, filePath);
					}
				}
			}

			if (abortFlag) return ERROR_PRINT_CANCELLED;
		}
		while(::FindNextFile(hFile, &fi) == TRUE);

		DWORD dwError = ::GetLastError();
		if (dwError != ERROR_NO_MORE_FILES)
			return dwError;
	}
	return 0;
}

/*inline int SearchDirectory (SearchLocationInfo const & info, tstring const & file_name
				, std::vector<tstring> & matches)
{
	return SearchDirectory(info.m_dir_path, info.m_includes, info.m_excludes, info.m_recursive, info.m_follow_symlinks, file_name, matches);
}*/

}}

