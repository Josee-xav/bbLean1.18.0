#include "lookup.h"

namespace bb { namespace search {

	bb::search::ProgramLookup * g_lookup = nullptr;

	ProgramLookup & getLookup() { return *g_lookup; }

	void startLookup (tstring const & path)
	{
		if (!g_lookup)
		{
			Config cfg;
			loadConfig(path, cfg);
			g_lookup = new bb::search::ProgramLookup(path, cfg);
		}
	}

	void stopLookup ()
	{
		if (g_lookup)
		{
			bb::search::getLookup().AbortIndexing();
			g_lookup->Stop();
			delete g_lookup;
			g_lookup = nullptr;
		}
	}

	void reconfigureLookup ()
	{
		if (g_lookup)
			g_lookup->ReloadOrBuild();
	}
	void reindexLookup ()
	{
		if (g_lookup)
			g_lookup->Reindex();
	}
	void abortReindexLookup ()
	{
		if (g_lookup)
			g_lookup->AbortIndexing();
	}
}}
