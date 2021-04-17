#pragma once
#include <vector>
#include "SearchLocationInfo.h"
#include "history.h"

namespace bb { namespace search {

struct Config
{
	//tstring m_home;
	//Config (tstring const & home) : m_home(home), { }

	//Config () : m_locations(), m_history(24) { }
	//Config () : m_locations(), m_history(24) { }

	std::vector<SearchLocationInfo> m_locations;
	//History m_history;
	//
	void clear ()
	{
		m_locations.clear();
	}
};

inline void defaultConfig (Config & cfg)
{
	cfg.clear();

	cfg.m_locations.push_back(SearchLocationInfo(TEXT("C:\\Program Files\\")
				, TEXT(".*\\.exe;.*\\.com;.*\\.bat;.*\\.cmd")
				, TEXT("C:\\Program Files\\NVIDIA Corporation\\Installer2")
				, true));
	cfg.m_locations.push_back(SearchLocationInfo(TEXT("C:\\Program Files (x86)\\")
				, TEXT(".*\\.exe;.*\\.com;.*\\.bat;.*\\.cmd")
				, TEXT("")
				, true));
	cfg.m_locations.push_back(SearchLocationInfo(TEXT("C:\\Windows\\")
				, TEXT(".*\\.exe;.*\\.com;.*\\.bat;.*\\.cmd")
				, TEXT("C:\\Windows\\winsxs\\;C:\\Windows\\Installer\\;C:\\Windows\\Microsoft.NET\\;C:\\Windows\\SoftwareDistribution\\")
				, true));
	// blackbox dir :)
	// c:\\bin
}

}}

