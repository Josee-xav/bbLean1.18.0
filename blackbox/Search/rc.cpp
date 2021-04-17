#include "rc.h"
#include <BBApi.h>
#include <Search/tmp.h>

namespace bb { namespace search {

	void loadConfig (tstring const & path, Config & cfg)
	{
		if (!readRC(path + TEXT("search.rc"), cfg))
			defaultConfig(cfg);
	}

	bool readRC (tstring const & fpath, Config & cfg)
	{
		cfg.clear();
		TCHAR rckey[128];
		bool got_some = false;
		for (size_t i = 0; i < 64; ++i)
		{
			_stprintf(rckey, TEXT("search.location.%d.dir"), i);
			char const * dir = ReadString(fpath.c_str(), rckey, nullptr);
			if (!dir)
				break;
			tstring dir_path(dir);
			_stprintf(rckey, TEXT("search.location.%d.includes"), i);
			tstring const includes = ReadString(fpath.c_str(), rckey, nullptr);
			_stprintf(rckey, TEXT("search.location.%d.excludes"), i);
			tstring const excludes = ReadString(fpath.c_str(), rckey, nullptr);
			_stprintf(rckey, TEXT("search.location.%d.recursive"), i);
			bool const recursive = ReadBool(fpath.c_str(), rckey, true);
			cfg.m_locations.push_back(SearchLocationInfo(dir_path, includes, excludes, recursive));
			got_some = true;
		}

		return got_some;
	}

	bool writeRC (tstring const & fpath, Config const & cfg)
	{
		for (size_t i = 0, ie = cfg.m_locations.size(); i < ie; ++i)
		{
			//search.location.0.dir: C:\Program Files\
			//search.location.0.includes: .*\.exe;.*\.com;.*\.bat;.*\.cmd
			//search.location.0.excludes: C:\Program Files\NVIDIA Corporation\Installer2
			//search.location.0.recursive: true
			
			TCHAR rckey[128];
			_stprintf(rckey, TEXT("search.location.%d.dir"), i);
			WriteString(fpath.c_str(), rckey, cfg.m_locations[i].m_dir_path.c_str());
			tstring join;
			for (tstring const & s : cfg.m_locations[i].m_includes)
			{
				join += s;
				join += TEXT(";");
			}
			_stprintf(rckey, TEXT("search.location.%d.includes"), i);
			WriteString(fpath.c_str(), rckey, join.c_str());
			join.clear();
			for (tstring const & s : cfg.m_locations[i].m_excludes)
			{
				join += s;
				join += TEXT(";");
			}
			_stprintf(rckey, TEXT("search.location.%d.excludes"), i);
			WriteString(fpath.c_str(), rckey, join.c_str());
			_stprintf(rckey, TEXT("search.location.%d.recursive"), i);
			WriteBool(fpath.c_str(), rckey, cfg.m_locations[i].m_recursive);
		}
		return true;
	}
}}

