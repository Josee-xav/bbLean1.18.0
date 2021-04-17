#include <ShlDisp.h>
#include <ShlGuid.h>
#include <atlbase.h>
#include <atlcom.h>
#include <vector>
#include "unicode.h"

// custom auto complete
class StringAutoComplete : public IEnumString
{
	tstrings m_list;
	ULONG m_current;
	ULONG m_ref;
	bool m_bound;
	CComPtr<IAutoComplete> m_pac;

public:
	StringAutoComplete ()
		: m_current(0), m_ref(0), m_bound(false)
	{
		m_list.reserve(256);
	}

	StringAutoComplete (tstrings const & list)
		: m_list(list), m_current(0), m_ref(0), m_bound(false)
	{ }

	~StringAutoComplete () { }

	bool Bind (HWND hwnd, DWORD p_dwOptions = 0, LPCTSTR p_lpszFormatString = NULL)
	{
		ATLASSERT(::IsWindow(hwnd));

		if (m_bound || m_pac)
			return false;

		HRESULT hr = m_pac.CoCreateInstance(CLSID_AutoComplete);
		if (SUCCEEDED(hr))
		{
			if (p_dwOptions)
			{
				CComQIPtr<IAutoComplete2> pAC2(m_pac);
				ATLASSERT(pAC2);
				hr = pAC2->SetOptions(p_dwOptions);
				pAC2.Release();
			}
			USES_CONVERSION;
			LPCWSTR arg = T2W(p_lpszFormatString);
			hr = m_pac->Init(hwnd, this, NULL, arg);

			if (SUCCEEDED(hr))
			{
				m_bound = true;
				return true;
			}
		}
		return false;
	}

	void Unbind ()
	{
		if (!m_bound)
			return;

		ATLASSERT(m_pac);
		if (m_pac)
		{
			m_pac.Release();
			m_bound = false;
		}
	}

	void SetList (tstrings const & list) { m_list = list; }

	bool AddItem (tstring const & item)
	{
		//@TODO: find dup
		m_list.push_back(item);
		return true;
	}

	bool RemoveItem (tstring const & item) { return true; }

	bool Clear ()
	{
		m_list.clear();
		m_current = 0;
		return true;
	}
	bool IsBound () const { return m_bound; }

public:
	//	IUnknown implementation
	STDMETHODIMP_(ULONG) AddRef () { return ::InterlockedIncrement(reinterpret_cast<LONG*>(&m_ref)); }
	STDMETHODIMP_(ULONG) Release()
	{
		ULONG nCount = (ULONG) ::InterlockedDecrement(reinterpret_cast<LONG*>(&m_ref));
		if (nCount == 0)
			delete this;
		return nCount;
	}

	STDMETHODIMP QueryInterface (REFIID riid, void * * ppvObject)
	{
		HRESULT hr = E_NOINTERFACE;
		if (ppvObject != NULL)
		{
			*ppvObject = NULL;
			if (IID_IUnknown == riid)
				*ppvObject = static_cast<IUnknown*>(this);
			if (IID_IEnumString == riid)
				*ppvObject = static_cast<IEnumString*>(this);
			if (*ppvObject != NULL)
			{
				hr = S_OK;
				((LPUNKNOWN)*ppvObject)->AddRef();
			}
		}
		else
			hr = E_POINTER;
		return hr;
	}

public:
	//	IEnumString implementation
	STDMETHODIMP Next (ULONG celt, LPOLESTR * rgelt, ULONG * pceltFetched)
	{
		if (celt < 0) return E_INVALIDARG;

		size_t fetched = 0;
		tstrings tmp;
		for (size_t i = m_current, ie = m_current + celt; i < ie; ++i)
			if (i < m_list.size())
				tmp.push_back(m_list[i]);
		size_t const n = tmp.size();
		fetched = n;

		for (size_t i = 0; i < n; ++i)
		{
			rgelt[i] = (LPWSTR)::CoTaskMemAlloc((tmp[i].length() + 1) * sizeof(wchar_t));
			ZeroMemory(rgelt[i], (tmp[i].length() + 1) * sizeof(wchar_t));

			USES_CONVERSION;
			wcscpy(rgelt[i], T2OLE(m_list[m_current].c_str()));
			++m_current;
		}

		if (pceltFetched)
			*pceltFetched = fetched;

		if (celt == fetched)
			return S_OK;
		return S_FALSE;
	}

	STDMETHODIMP Skip (ULONG celt)
	{
		m_current += celt;
		if (m_current >= m_list.size())
			return S_FALSE;
		return S_OK;
	}

	STDMETHODIMP Reset ()
	{
		m_current = 0;
		return S_OK;
	}

	STDMETHODIMP Clone (IEnumString * * ppenum)
	{
		if (!ppenum)
			return E_POINTER;
		StringAutoComplete * pnew = new StringAutoComplete();
		pnew->AddRef();
		*ppenum = pnew;
		return S_OK;
	}

private:
	void InternalInit()
	{
		m_current = 0;
		m_ref = 0;
		m_bound = false;
	}
};

StringAutoComplete * autocomplete = 0;

bool enableCompletion (HWND hwnd, tstrings const & keys)
{
	if (autocomplete) // already exists
		return false;

	// Initialize Autocomplete object and source object
	IAutoComplete * iac = 0;
	if (!SUCCEEDED(CoCreateInstance(CLSID_AutoComplete, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&iac))))
		return false;

	autocomplete = new StringAutoComplete(keys);
	if (!SUCCEEDED(autocomplete->Bind(hwnd)))
		return false;

	// initialize the IUnknown Interface
	IUnknown * iu = 0;
	if (!SUCCEEDED(autocomplete->QueryInterface(IID_PPV_ARGS(&iu))))
		return false;

	if (!SUCCEEDED(iac->Init(hwnd, iu, NULL, NULL)))
		return false;

	IAutoComplete2 * iac2 = 0;
	if (SUCCEEDED(iac->QueryInterface(IID_PPV_ARGS(&iac2))))
	{
		iac2->SetOptions(ACO_AUTOSUGGEST | ACO_AUTOAPPEND);
		return true;
	}
	return false;
}

void disableCompletion ()
{
	if (autocomplete)
	{
		autocomplete->Unbind();
		autocomplete->Release();
		autocomplete = 0;
	}
}

bool iSCompletionEnabled ()
{
	if (autocomplete && autocomplete->IsBound())
		return true;
	return false;
}

