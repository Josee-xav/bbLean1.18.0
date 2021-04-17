#pragma once

inline tstring WINAPI getBlackboxPath ()
{
	TCHAR tmp[1024];
	GetBlackboxPath(tmp, 1024);
	return tstring(tmp);
}



