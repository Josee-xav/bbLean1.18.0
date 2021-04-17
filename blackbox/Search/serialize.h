#pragma once
#include <iosfwd>
#include <sstream>

namespace bb { namespace search {

	unsigned const version11 = 0x00010001;

	template<typename T>
	std::istream & readpod (std::istream & is, T & t)
	{
		return is.read(reinterpret_cast<char *>(&t), sizeof(T));
	}
	inline std::istream & read (std::istream & is, int & t) { return readpod<int>(is, t); }
	inline std::istream & read (std::istream & is, unsigned & t) { return readpod<unsigned>(is, t); }
	inline std::istream & read (std::istream & is, float & t) { return readpod<float>(is, t); }
	inline std::istream & read (std::istream & is, tstring & t)
	{
		unsigned n = 0;
		if (readpod<unsigned>(is, n).fail())
			return is;
		t.resize(n);
		is.read(&t[0], n);
		return is;
	}

	template <typename T>
	std::istream & read (std::istream & is, std::vector<T> & t)
	{
		unsigned n = 0;
		if (!read(is, n))
			return is;
		t.resize(n);
		for (unsigned i = 0; i < n; ++i)
			if (!read(is, t[i]))
				return is;
		return is;
	}

	/////////////////////////////////////////////////////////////////////////////
	// SAVE
	template<typename T>
	std::ostream & writepod (std::ostream & is, T const & t)
	{
		return is.write(reinterpret_cast<char const *>(&t), sizeof(T));
	}
	inline std::ostream & write (std::ostream & is, int const & t) { return writepod<int>(is, t); }
	inline std::ostream & write (std::ostream & is, unsigned const & t) { return writepod<unsigned>(is, t); }

	inline std::ostream & write (std::ostream & is, tstring const & t)
	{
		char const * ptr = t.c_str();
		unsigned const n = static_cast<unsigned>(t.size() * sizeof(TCHAR));

		if (write(is, n).fail())
			return is;
		return is.write(ptr, n);
	}

	template <typename T>
	std::ostream & write (std::ostream & s, std::vector<T> const & t)
	{
		unsigned const n = static_cast<unsigned>(t.size());
		if (!write(s, n)) return s;
		for (unsigned i = 0; i < n; ++i)
			if (write(s, t[i]).fail())
				return s;
		return s;
	}
}}

