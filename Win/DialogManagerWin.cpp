/*
* Copyright 2012 Ryan Ackley (ryanackley@gmail.com)
*
* This file is part of NPAPI Chrome File API
*
* NPAPI Chrome File API is free software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "atlbase.h"

//#include "win_common.h"
//#include <commdlg.h>

#include "atlapp.h"
#include "atldlgs.h"
#include <string>

#include <boost/thread.hpp>
#include "utf8_tools.h"
#include "Win/PluginWindowlessWin.h"
#include "Win/PluginWindowWin.h"

#include "DialogManagerWin.h"
#include <shlobj.h>

#include <objbase.h>      // For COM headers
#include <shobjidl.h>     // for IFileDialogEvents and IFileDialogControlEvents
#include <shlwapi.h>
#include <knownfolders.h> // for KnownFolder APIs/datatypes/function headers

#include <strsafe.h>      // for StringCchPrintfW
#include <shtypes.h>      // for COMDLG_FILTERSPEC

//#include "precompiled_headers.h" // Anything before this is PCH on windows

#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

const COMDLG_FILTERSPEC c_rgSaveTypes[] =
{
    {L"Word Document (*.doc)",       L"*.doc"},
    {L"Web Page (*.htm; *.html)",    L"*.htm;*.html"},
    {L"Text Document (*.txt)",       L"*.txt"},
    {L"All Documents (*.*)",         L"*.*"},
    {L"Files and Folders (*)",       L"*.*"}
};

// Indices of file types
#define INDEX_WORDDOC 1
#define INDEX_WEBPAGE 2
#define INDEX_TEXTDOC 3

// Controls
#define CONTROL_GROUP           2000
#define CONTROL_LIST			 2
#define CONTROL_RADIOBUTTON1    1
#define CONTROL_RADIOBUTTON2    2       

// IDs for the Task Diaog Buttons
#define IDC_BASICFILEOPEN                       100
#define IDC_ADDITEMSTOCUSTOMPLACES              101
#define IDC_ADDCUSTOMCONTROLS                   102
#define IDC_SETDEFAULTVALUESFORPROPERTIES       103
#define IDC_WRITEPROPERTIESUSINGHANDLERS        104
#define IDC_WRITEPROPERTIESWITHOUTUSINGHANDLERS 105

/* File Dialog Event Handler *****************************************************************************************************/

class CDialogEventHandler : public IFileDialogEvents,
                            public IFileDialogControlEvents
{
public:
    // IUnknown methods
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        static const QITAB qit[] = {
            QITABENT(CDialogEventHandler, IFileDialogEvents),
            QITABENT(CDialogEventHandler, IFileDialogControlEvents),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        long cRef = InterlockedDecrement(&_cRef);
        if (!cRef)
            delete this;
        return cRef;
    }

    // IFileDialogEvents methods
    IFACEMETHODIMP OnFileOk(IFileDialog *) { return S_OK; };
    IFACEMETHODIMP OnFolderChange(IFileDialog *) { return S_OK; };
    IFACEMETHODIMP OnFolderChanging(IFileDialog *, IShellItem *) { return S_OK; };
    IFACEMETHODIMP OnHelp(IFileDialog *) { return S_OK; };
    
	IFACEMETHODIMP OnSelectionChange(IFileDialog *pfd)
    {
        IShellItem *psi;
        HRESULT hr = pfd->GetCurrentSelection(&psi);
        if (SUCCEEDED(hr))
        {
            SFGAOF attr;
            hr = psi->GetAttributes(SFGAO_FOLDER, &attr);
            if (SUCCEEDED(hr) && (SFGAO_FOLDER == attr))
            {
				LPWSTR folderName = NULL;
				psi->GetDisplayName(SIGDN_FILESYSPATH, &folderName);
				//pfd->SetFileName(folderName);
				
				//m_strPathList += folderName;

				//m_strPathList += L"::";
            }
            else
            {
            }
            psi->Release();
        }
        return S_OK;
    }

	//IFACEMETHODIMP OnSelectionChange(IFileDialog *pfd){ return S_OK; }
    IFACEMETHODIMP OnShareViolation(IFileDialog *, IShellItem *, FDE_SHAREVIOLATION_RESPONSE *) { return S_OK; };
    IFACEMETHODIMP OnTypeChange(IFileDialog *pfd);
    IFACEMETHODIMP OnOverwrite(IFileDialog *, IShellItem *, FDE_OVERWRITE_RESPONSE *) { return S_OK; };

    // IFileDialogControlEvents methods
	IFACEMETHODIMP OnItemSelected(IFileDialogCustomize *pfdc, DWORD dwIDCtl, DWORD dwIDItem) { return S_OK;};
    IFACEMETHODIMP OnButtonClicked(IFileDialogCustomize *, DWORD) { return S_OK; };
    IFACEMETHODIMP OnCheckButtonToggled(IFileDialogCustomize *, DWORD, BOOL) { return S_OK; };
    IFACEMETHODIMP OnControlActivating(IFileDialogCustomize *, DWORD) { return S_OK; };

    CDialogEventHandler() : _cRef(1) { };
private:
    ~CDialogEventHandler() { };
    long _cRef;
};

HRESULT CDialogEventHandler_CreateInstance(REFIID riid, void **ppv)
{
    *ppv = NULL;
    CDialogEventHandler *pDialogEventHandler = new (std::nothrow) CDialogEventHandler();
    HRESULT hr = pDialogEventHandler ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = pDialogEventHandler->QueryInterface(riid, ppv);
        pDialogEventHandler->Release();
    }
    return hr;
}

HRESULT CDialogEventHandler::OnTypeChange(IFileDialog *pfd)
{
    IFileSaveDialog *pfsd;
    HRESULT hr = pfd->QueryInterface(&pfsd);
    if (SUCCEEDED(hr))
    {
        UINT uIndex;
        hr = pfsd->GetFileTypeIndex(&uIndex);   // index of current file-type
        if (SUCCEEDED(hr))
        {
            IPropertyDescriptionList *pdl = NULL;

            switch (uIndex)
            {
            case INDEX_WORDDOC:
                hr = 1;
				if (SUCCEEDED(hr))
                {
                    // FALSE as second param == do not show default properties.
                    hr = pfsd->SetCollectedProperties(pdl, FALSE);
                    pdl->Release();
                }
                break;

            case INDEX_WEBPAGE:
				hr = 1;
                if (SUCCEEDED(hr))
                {
                    // FALSE as second param == do not show default properties.
                    hr = pfsd->SetCollectedProperties(pdl, FALSE);
                    pdl->Release();
                }
                break;

            case INDEX_TEXTDOC:
                // When .txt is selected, let's ask for some other arbitrary property, say Author.
              //  hr = PSGetPropertyDescriptionListFromString(L"prop:System.Author", IID_PPV_ARGS(&pdl));
                hr =1;
				if (SUCCEEDED(hr))
                {
                    // TRUE as second param == show default properties as well, but show Author property first in list.
                    hr = pfsd->SetCollectedProperties(pdl, TRUE);
                    pdl->Release();
                }
                break;
            }
        }
        pfsd->Release();
    }
    return hr;
}


DialogManager* DialogManager::get()
{
	static DialogManagerWin inst;
    return &inst;

}

void DialogManagerWin::OpenFileDialog(const FB::BrowserHostPtr& host, FB::PluginWindow* win, const PathCallback& cb, const int fileType, const bool multiple, const int fileOperation) {
    FB::PluginWindowWin* wndWin = dynamic_cast<FB::PluginWindowWin*>(win);
    FB::PluginWindowlessWin* wndlessWin = dynamic_cast<FB::PluginWindowlessWin*>(win);

    HWND browserWindow = wndWin ? wndWin->getBrowserHWND() : wndlessWin->getHWND();
    //HWND browserWindow =  wndWin->getBrowserHWND();
	boost::thread dlgThread(&DialogManagerWin::_showFileDialog, this, browserWindow, "*.*", "Select Files", cb, multiple, fileOperation);
	
}

void DialogManagerWin::OpenFolderDialog(const FB::BrowserHostPtr& host, FB::PluginWindow* win, const PathCallback& cb, const int fileType, const bool multiple, const int fileOperation) {
    FB::PluginWindowWin* wndWin = dynamic_cast<FB::PluginWindowWin*>(win);
    FB::PluginWindowlessWin* wndlessWin = dynamic_cast<FB::PluginWindowlessWin*>(win);

    HWND browserWindow = wndWin ? wndWin->getBrowserHWND() : wndlessWin->getHWND();
    //HWND browserWindow =  wndWin->getBrowserHWND();
	boost::thread dlgThread(&DialogManagerWin::_showFolderDialog2, this, browserWindow, "*.*", fileType, cb, multiple, fileOperation);
	
}


void DialogManagerWin::_showFolderDialog(HWND wnd, const PathCallback& cb) {
    BROWSEINFO bi = { 0 };
    bi.lpszTitle = _T("Select a folder");
    bi.hwndOwner = wnd;
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
//	LPITEMIDLIST pidl = 0;
  //bool gotfn = GetFileNameFromBrowse (wnd, L"", 0, L"", L"", L"", L"");  
	if ( pidl != 0 )
    {
        std::wstring out;
        // get the name of the folder
        TCHAR path[MAX_PATH];
        if ( SHGetPathFromIDList ( pidl, path ) )
        {
            out = path;
        }

        // free memory used
        IMalloc * imalloc = 0;
        if ( SUCCEEDED( SHGetMalloc ( &imalloc )) )
        {
            imalloc->Free ( pidl );
            imalloc->Release ( );
        }
        cb(FB::wstring_to_utf8(path));
    } else {
        cb("");
    }
}

void DialogManagerWin::_showFileDialog(HWND wnd, const std::string& path, const std::string& filter, const PathCallback& cb, bool multiple, const int fileOperation)
{
    wchar_t Filestring[500*MAX_PATH];
	memset(&Filestring, 0, sizeof(Filestring));
    std::string out;

    std::wstring wFilter(FB::utf8_to_wstring(filter));
    std::wstring wPath(FB::utf8_to_wstring(filter));
    
	
	OPENFILENAME opf;
       
	opf.hwndOwner = wnd;
    //opf.lpstrFilter = wFilter.c_str();
    opf.lpstrCustomFilter = 0;
    opf.nMaxCustFilter = 0L;
    opf.nFilterIndex = 1L;
	opf.lpstrFile = Filestring;
    opf.lpstrFile[0] = '\0';
    opf.nMaxFile = 512 * MAX_PATH;
    opf.lpstrFileTitle = 0;
    opf.nMaxFileTitle= 260;
    opf.lpstrInitialDir = wPath.c_str();
    opf.lpstrTitle = L"Select files";
    opf.nFileOffset = 0;
    opf.nFileExtension = 0;
    opf.lpstrDefExt = L"*.*";
    opf.lpfnHook = NULL;
    opf.lCustData = 0;
	opf.lpstrFilter = L"All\0*.*\0Text\0*.TXT\0";
    opf.lStructSize = sizeof(OPENFILENAME);
	
	if ( multiple && fileOperation != 3 ) {
		opf.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_NOVALIDATE;
	} else {
	    opf.Flags = (OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT| OFN_NOVALIDATE) & ~OFN_ALLOWMULTISELECT;
	}
	if (fileOperation == 2) {
		bool result;
		result = GetSaveFileName(&opf);
		if (!result ) {
			out = "Failed.";
			cb(out);
			return;
		}
	}

	if(GetOpenFileName(&opf))
    {
		bool bMultipleFileSelected = (opf.lpstrFile[opf.nFileOffset - 1] == '\0');

		if (bMultipleFileSelected)
		{
			bool bDirectoryIsRoot = (opf.lpstrFile[wcslen(opf.lpstrFile) - 1] == '\\');
	      
			for (wchar_t *szTemp = opf.lpstrFile + opf.nFileOffset; *szTemp;	szTemp += (wcslen(szTemp) + 1))
			{
				size_t dwLen = wcslen(opf.lpstrFile) + wcslen(szTemp) + 2;
				wchar_t * szFile = new wchar_t[dwLen];
				wcscpy_s(szFile, dwLen, opf.lpstrFile);
				if (!bDirectoryIsRoot)
				{
					wcscat_s(szFile, dwLen, L"\\"); 
				}
				wcscat_s(szFile, dwLen, szTemp);   
				out += FB::wstring_to_utf8(std::wstring(szFile));
				out += "::";
				delete szFile;
			}
		} else {
			out = FB::wstring_to_utf8(std::wstring(Filestring));
		} 
    }

    cb(out);
}
void DialogManagerWin::_showFolderDialog2(HWND wnd, const std::string& path, const int fileType, const PathCallback& cb, const bool multiple, const int fileOperation)
{
	wchar_t Filestring[MAX_PATH * 512];
	
	memset(&Filestring, 0, sizeof(Filestring));
	memset(&m_strPathList, 0, sizeof(m_strPathList));
	m_strPathList = L"";
	std::string out;

	LPCTSTR lpszDefExt = NULL;
	DWORD dwFlags =  (FOS_FORCEFILESYSTEM | FOS_NOVALIDATE) &~(FOS_STRICTFILETYPES | FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST);
	LPWSTR pszFilePath = NULL;
	
	std::wstring lpszFileName; 
	lpszFileName = Filestring;
	IFileOpenDialog * ptr = NULL;
	HRESULT hr = NULL;
	
	if (fileOperation == 2) {
		IFileSaveDialog * ptr = NULL;
		HRESULT foobar = CoInitialize(ptr);
		HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&ptr));
		
	} else {
		HRESULT foobar = CoInitialize(ptr);
		HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&ptr));
		
	}
	if (SUCCEEDED(hr))
        {	
			// Create an event handling object, and hook it up to the dialog.
			IFileDialogEvents *pfde = NULL;
			hr = CDialogEventHandler_CreateInstance(IID_PPV_ARGS(&pfde));
			
			// Set up a Customization.
			IFileDialogCustomize *pfdc = NULL;
			hr = ptr->QueryInterface(IID_PPV_ARGS(&pfdc));
			
			// Create a Visual Group.
			hr = pfdc->StartVisualGroup(CONTROL_GROUP, L"OfficeDrive");
			// hr = pfdc->AddControlItem(CONTROL_LIST, lst1, L"Customize");
            // hr = pfdc->SetControlState(CONTROL_LIST, CDCS_VISIBLE | CDCS_ENABLED);

		if (SUCCEEDED(hr)) 
			{
			// Hook up the event handler.
			DWORD dwCookie;
			hr = ptr->Advise(pfde, &dwCookie);
		
            if (SUCCEEDED(hr))
            {
                // Set the options on the dialog.
                hr = ptr->GetOptions(&dwFlags);
                if (SUCCEEDED(hr))
                {
                    // In this case, get shell items only for file system items.
					if ( multiple ) {
						  hr = ptr->SetOptions(dwFlags | FOS_ALLOWMULTISELECT);
					}
					if (fileType == 2) {
						  hr = ptr->GetOptions(&dwFlags);					 
						  hr = ptr->SetOptions(dwFlags | FOS_PICKFOLDERS);
					}
					// hr = ptr->SetOptions(dwFlags | FOS_FORCEFILESYSTEM | FOS_NOVALIDATE | FOS_ALLOWMULTISELECT);
                    if (SUCCEEDED(hr))
						//hr = ptr->SetFileTypes(ARRAYSIZE(c_rgSaveTypes), c_rgSaveTypes);
						if (SUCCEEDED(hr))
                        {
                          //  hr = ptr->SetFileTypeIndex(4);
                            if (SUCCEEDED(hr))
                            {
                                //hr = ptr->SetDefaultExtension(L"");
                                if (SUCCEEDED(hr))
                                {
                                    // Show the dialog
					
                                    hr = ptr->Show(wnd);
                                    if (SUCCEEDED(hr))
                                    {
                                       
										IShellItemArray *psiResults;
										ptr->GetResults(&psiResults);

										IEnumShellItems *ppenum;
										psiResults->EnumItems(&ppenum);
										
										IShellItem * psiItem;

										hr = ppenum->Next(1, &psiItem,0);
										while (hr == S_OK) {
											//psiResults->GetItemAt(i, &psiItem);
											//psiItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
											psiItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pszFilePath);
											lpszFileName += pszFilePath;
											lpszFileName += L"::";
											hr = ppenum->Next(1, &psiItem,0);
										}


                                        if (SUCCEEDED(hr))
                                        {	
											if (fileType == 3 && multiple){
												out = FB::wstring_to_utf8(std::wstring(m_strPathList));
												out += FB::wstring_to_utf8(lpszFileName);
											} else {
												out = FB::wstring_to_utf8(lpszFileName);
											}
											cb(out);
                                            psiResults->Release();
                                        }
									}
                                }
                            }
                        }
                    }
                }
                // Unhook the event handler.
               ptr->Unadvise(dwCookie);
			}
			pfde->Release();
		} 
        ptr->Release();
	}

	//out = FB::wstring_to_utf8(std::wstring(pszFilePath));
    
    //   {
	//	bool bMultipleFileSelected = multiple;

	//	if (bMultipleFileSelected)
	//	{
	//		bool bDirectoryIsRoot = (opf.lpstrFile[wcslen(opf.lpstrFile) - 1] == '\\');
	//      
	//		for (wchar_t *szTemp = opf.lpstrFile + opf.nFileOffset; *szTemp;	szTemp += (wcslen(szTemp) + 1))
	//		{
	//			size_t dwLen = wcslen(opf.lpstrFile) + wcslen(szTemp) + 2;
	//			wchar_t * szFile = new wchar_t[dwLen];
	//			wcscpy_s(szFile, dwLen, opf.lpstrFile);
	//			if (!bDirectoryIsRoot)
	//			{
	//				wcscat_s(szFile, dwLen, L"\\"); 
	//			}
	//			wcscat_s(szFile, dwLen, szTemp);   
	//			out += FB::wstring_to_utf8(std::wstring(szFile));
	//			out += "::";
	//			delete szFile;
	//		}
	//} else { 
	//	out = FB::wstring_to_utf8(std::wstring(Filestring));
	//	} 
	//}
	
UINT CALLBACK OFNHookProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hWp, hWlv;
	NMHDR *pnm;
	static BOOL init;
	LVFINDINFO lvfi;
	int indx;

	switch (msg)
	{
	
	case WM_NOTIFY:
		if (init) break;		// if already did highlite
		pnm = (NMHDR *)lParam;
		if (pnm->code == CDN_SELCHANGE)
		{
			init = true;			// first time notification after list view created
			hWp = GetParent(hWnd);	// get parent
			hWlv = GetDlgItem(hWp, lst2);	// get list view shell window
			if (hWlv == NULL) break;
			hWlv = GetDlgItem(hWlv, 1);		// get "SysListView32" window
			if (hWlv == NULL) break;
			lvfi.flags = LVFI_STRING;
			//lvfi.psz = path;
			indx = ListView_FindItem(hWlv, -1, &lvfi);	// find filename in list
			if (indx < 0) break;
			ListView_SetItemState(hWlv, indx,			// if found, select it
				LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			ListView_EnsureVisible(hWlv, indx, true);	// scroll into view
			SetFocus(hWlv);
			return true;
		}
		break;
		
	}
	return false;
}

UINT CALLBACK FolderHook (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		//case WM_COMMAND:
			//return TRUE;
	
		case WM_INITDIALOG:
			memset(&m_strPathList,0, sizeof(m_strPathList));
			memset(&m_strPath,0, sizeof(m_strPath));
			m_strPath = GetPath(hwnd, CDM_GETFILEPATH);
			m_curDir = m_strPath;
			
			return FALSE;

		case WM_CLOSE:	
			EndDialog(hwnd, 0);
			break;
			default:
			return FALSE;
	
		case WM_NOTIFY:

			LPOFNOTIFYW lpofn = (LPOFNOTIFYW)lParam;
			//memcpy(&lpofn, &lpofnEx, sizeof(lpofn));
			//lpofn->lpOFN->lStructSize = sizeof(lpofn);
			
			NMHDR * pNmhdr = &lpofn->hdr;
			HWND m_edit = GetDlgItem(pNmhdr->hwndFrom, edt1);
			HWND m_listView = GetDlgItem(pNmhdr->hwndFrom, lst2);
			HWND m_sysView = GetDlgItem(m_listView, 1);

			if (pNmhdr->code == CDN_SELCHANGE)
			{
				memset(&m_strPathList, 0, sizeof(m_strPathList));
				m_strPathList = L"";
				LVITEM m_listItem = {0}; 

				m_listItem.iItem = 0;
				m_listItem.iSubItem = 0;
				m_listItem.mask = LVIF_TEXT|LVIF_STATE;
				m_listItem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
				const int m_max = MAX_PATH * 512 - 1;
				m_listItem.cchTextMax = MAX_PATH * 512;
									
				int nSelItems;
				
				int i;

				nSelItems = ListView_GetItemCount(m_sysView);
				for (i = 0; i < nSelItems; i++) {
					m_listItem.iItem = i;
		
					wchar_t tempsz[m_max];
					m_listItem.pszText = LPWSTR(tempsz);

					ListView_GetItem(m_sysView, &m_listItem);
					//ListView_GetItemState(m_sysView, &m_listItem,LVIS_SELECTED);
					if (m_listItem.state & LVIS_SELECTED) {
						m_strPathList += m_listItem.pszText;

						if (lpofn->lpOFN->Flags & OFN_ALLOWMULTISELECT)
						{
							m_strPathList +=( _T("::"));
						}
						
					}
				}
				CommDlg_OpenSave_SetControlText(pNmhdr->hwndFrom, edt1, m_strPathList.c_str());
			}

			else if (pNmhdr->code == CDN_FOLDERCHANGE)
			{
				//memset(&m_strPath, 0, sizeof(m_strPath));
				//memset(&m_strPathList, 0, sizeof(m_strPathList));
				m_strPath = GetPath(hwnd, CDM_GETFILEPATH);
				m_curDir = m_strPath;
			
				memset(&m_strPathList, 0, sizeof(m_strPathList));

				CommDlg_OpenSave_SetControlText(pNmhdr->hwndFrom, edt1, m_strPathList.c_str());
		
			}
			return FALSE;
	}
	return TRUE;
}

std::wstring GetPath(HWND hwnd, UINT nMessage)
{
	LPCTSTR strPath = _T("");
	TCHAR szDir[MAX_PATH * 512];
	memset(&szDir,0, sizeof(szDir));	
	memset(&strPath,0, sizeof(strPath));
	
	HWND pWnd = GetParent(hwnd);

	if (pWnd && IsWindow(pWnd))
	{
		//szDir[0] = _T('\0');
		SendMessage(pWnd, nMessage, sizeof(szDir)/sizeof(TCHAR)-2,
			(LPARAM)(LPCTSTR)szDir);
		szDir[sizeof(szDir)/sizeof(TCHAR)-1] = _T('\0');
		strPath = szDir;
	};
	
	return strPath;
}
