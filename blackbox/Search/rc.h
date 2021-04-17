#pragma once
#include "config.h"

namespace bb { namespace search {

	bool writeRC (tstring const & fpath, Config const & cfg);
	bool readRC (tstring const & fpath, Config & cfg);

	void loadConfig (tstring const & path, Config & cfg);
}}

