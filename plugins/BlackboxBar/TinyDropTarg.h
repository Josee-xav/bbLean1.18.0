#pragma once
#include <shlobj.h>
#include <shellapi.h>
#include "BBApi.h"
#include "Workspaces.h"
#include "BlackboxBar.h"

class TinyDropTarget : public IDropTarget {
public:
	TinyDropTarget(HWND hwnd);
	virtual ~TinyDropTarget();
	// IUnknown methods
	STDMETHOD(QueryInterface)(REFIID iid, void** ppvObject);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	// IDropTarget methods
	STDMETHOD(DragEnter)(LPDATAOBJECT pDataObject, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
	STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
	STDMETHOD(DragLeave)();
	STDMETHOD(Drop)(LPDATAOBJECT pDataObject, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
	HWND m_hwnd;
	HWND task_over;
	void handle_task_timer(void);
private:
	DWORD m_dwRef;
	bool valid_data;

};

TinyDropTarget* init_drop_targ(HWND hwnd);
void exit_drop_targ(class TinyDropTarget* m_TinyDropTarget);
void handle_task_timer(class TinyDropTarget* m_TinyDropTarget);