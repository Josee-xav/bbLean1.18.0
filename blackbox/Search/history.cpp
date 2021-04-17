#include "history.h"
#include <fstream>

namespace bb { namespace search {

	bool History::Load ()
	{
		tstring const item_fpath = m_path + m_name + TEXT(".props");
		if (load(m_items, item_fpath))
		{
			_tprintf(TEXT("Load OK.\n"));
			return true;
		}

		m_items.clear();
		return false;
	}

	bool History::Save ()
	{
		tstring const prop_fpath = m_path + m_name + TEXT(".props");
		save(m_items, prop_fpath);
		return true; // ehm
	}


	bool History::load (items_t & t, tstring const & fpath)
	{
		std::fstream f(fpath.c_str(), std::ios::in | std::ios::binary);
		if (!f)
			return false;

		unsigned v = 0;
		if (read(f, v).fail())
		{
			f.close();
			return false;
		}
		if (v != version11)
		{
			f.close();
			return false;
		}

		if (read(f, t).fail())
		{
			f.close();
			return false;
		}

		f.close();
		return true;
	}

	bool History::save (items_t const & t, tstring const & fpath)
	{
		std::fstream f(fpath.c_str(), std::ios::out | std::ios::binary);
		if (!f)
			return false;

		if (write(f, version11).fail())
		{
			f.close();
			return false;
		}
		if (write(f, t).fail())
		{
			f.close();
			return false;
		}
		f.close();
		return true;
	}
}}

