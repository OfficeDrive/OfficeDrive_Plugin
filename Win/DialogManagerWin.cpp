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
#include "precompiled_headers.h" // Anything before this is PCH on windows
// #include "XFolderDialog.cpp"
//#include "Win/XFolderDialog.h"

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
	boost::thread dlgThread(&DialogManagerWin::_showFileDialog, this, browserWindow, "*.*", "Select Files", cb, multiple);
	
}

void DialogManagerWin::OpenFolderDialog(const FB::BrowserHostPtr& host, FB::PluginWindow* win, const PathCallback& cb, const int fileType, const bool multiple, const int fileOperation) {
    FB::PluginWindowWin* wndWin = dynamic_cast<FB::PluginWindowWin*>(win);
    FB::PluginWindowlessWin* wndlessWin = dynamic_cast<FB::PluginWindowlessWin*>(win);

    HWND browserWindow = wndWin ? wndWin->getBrowserHWND() : wndlessWin->getHWND();
    //HWND browserWindow =  wndWin->getBrowserHWND();
	boost::thread dlgThread(&DialogManagerWin::_showFolderDialog2, this, browserWindow, "*.*", fileType, cb, multiple);
	
}


void DialogManagerWin::_showFolderDialog(HWND wnd, const PathCallback& cb) {
    BROWSEINFO bi = { 0 };
    bi.lpszTitle = _T("Select a folder to import");
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
void DialogManagerWin::_showFileDialog(HWND wnd, const std::string& path, const std::string& filter, const PathCallback& cb, bool multiple)
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
    opf.nMaxFile = 500 * MAX_PATH;
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
	
	if ( multiple ) {
		opf.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
	} else {
	    opf.Flags = (OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT) & ~OFN_ALLOWMULTISELECT;
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
				out += ":";
				delete szFile;
			}
		} else {
			out = FB::wstring_to_utf8(std::wstring(Filestring));
		} 
    }

    cb(out);
}
void DialogManagerWin::_showFolderDialog2(HWND wnd, const std::string& path, const int fileType, const PathCallback& cb, const bool multiple)
{
    wchar_t Filestring[MAX_PATH * 512];
	memset(&Filestring, 0, sizeof(Filestring));

	memset(&m_strPath, 0, sizeof(m_strPathList));
	memset(&m_strPathList, 0, sizeof(m_strPathList));
	m_folderChange = false;

	std::string out;

    //std::wstring wFilter(FB::utf8_to_wstring(filter));
    //std::wstring wPath(FB::utf8_to_wstring(filter));
 
	OPENFILENAME opf;
	opf.lStructSize = sizeof(OPENFILENAME);
    
	//memset(&opf, 0, sizeof(OPENFILENAME));

	//opf.lStructSize =  OPENFILENAME_SIZE_VERSION_400W; 
	//opf.lpTemplateName = NULL;
	opf.hwndOwner = wnd;

	if (fileType == 2) {
		opf.lpstrFilter = L"Folders Only\0___.__\0";
		opf.lpstrTitle = L"Select folders...";
	} else {
		opf.lpstrFilter = L"All Files\0*.*\0\0";
		opf.lpstrTitle = L"Select folders and files...";
	}			
    
	opf.lpstrCustomFilter = 0;
    opf.nMaxCustFilter = 0L;
    opf.nFilterIndex = 1L;
	
	opf.lpstrFile = Filestring;
	opf.lpstrFile[0] = '\0';
	opf.lpstrFileTitle = NULL;
	memset(&opf.lpstrFileTitle, 0, sizeof(opf.lpstrFileTitle));
    opf.nMaxFile = MAX_PATH * 512;
    opf.nMaxFileTitle= MAX_PATH * 512;
    opf.lpstrInitialDir = L"\0";
    
    opf.nFileOffset = 0;
    opf.nFileExtension = 0;
    opf.lpstrDefExt = L"*.*";
    opf.lCustData = NULL;
	opf.lpfnHook = FolderHook;	
    
	opf.FlagsEx |= ~OFN_EX_NOPLACESBAR;
	
	if ( multiple ) {
		opf.Flags =  OFN_ENABLESIZING | OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_NOVALIDATE | OFN_ENABLEHOOK | OFN_HIDEREADONLY;
		
	} else {
	    opf.Flags =  OFN_ENABLESIZING | OFN_NOVALIDATE | OFN_EXPLORER | OFN_ENABLEHOOK | OFN_HIDEREADONLY;

	}
 
	if(GetOpenFileName(&opf))
    {
		bool bMultipleFileSelected = multiple;

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
/*
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
*/

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
				LVITEM m_listItem = {0}; 
				m_listItem.iItem = 0;
				m_listItem.iSubItem = 0;
				m_listItem.mask = LVIF_TEXT;
				const int m_max = MAX_PATH * 512 - 1;
				m_listItem.cchTextMax = MAX_PATH * 512;
									
				int nSelItems;
				
				int i;

				nSelItems = ListView_GetSelectedCount(m_sysView);
				for (i = 0; i < nSelItems; i++) {
					m_listItem.iItem = i;
		
					wchar_t tempsz[m_max];
					m_listItem.pszText = LPWSTR(tempsz);

					ListView_GetItem(m_sysView, &m_listItem);
					m_strPathList += m_listItem.pszText;
				
					if (lpofn->lpOFN->Flags & OFN_ALLOWMULTISELECT)  {
						m_strPathList +=( _T("::"));
					}
				}
				
				CommDlg_OpenSave_SetControlText(pNmhdr->hwndFrom, edt1, m_strPathList.c_str());
				m_strPathList.clear();
			}

	
			else if (pNmhdr->code == CDN_FOLDERCHANGE)
			{
				//memset(&m_strPath, 0, sizeof(m_strPath));
				//memset(&m_strPathList, 0, sizeof(m_strPathList));
				m_strPath = GetPath(hwnd, CDM_GETFILEPATH);
				m_curDir = m_strPath;
				//CommDlg_OpenSave_SetControlText(pNmhdr->hwndFrom, edt1, m_strPathList.c_str());
		
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
		
		//
		//szDir[0] = _T('\0');
		SendMessage(pWnd, nMessage, sizeof(szDir)/sizeof(TCHAR)-2,
			(LPARAM)(LPCTSTR)szDir);
		szDir[sizeof(szDir)/sizeof(TCHAR)-1] = _T('\0');
		strPath = szDir;
	};
	
	return strPath;
}
