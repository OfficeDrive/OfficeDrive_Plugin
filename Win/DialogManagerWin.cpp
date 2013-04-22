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

#include "win_common.h"
//#include <commdlg.h>

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
	boost::thread dlgThread(&DialogManagerWin::_showFolderDialog2, this, browserWindow, "*.*", "Select Folders", cb, multiple);
	
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
			out = FB::wstring_to_utf8(std::wstring(opf.lpstrFile));
		} 
    }

    cb(out);
}
void DialogManagerWin::_showFolderDialog2(HWND wnd, const std::string& path, const std::string& filter, const PathCallback& cb, const bool multiple)
{
    wchar_t Filestring[MAX_PATH * 512];
	memset(&Filestring, 0, sizeof(Filestring));

	memset(&m_strPath, 0, sizeof(m_strPathList));
	memset(&m_strPathList, 0, sizeof(m_strPathList));
	m_folderChange = false;

	std::string out;

    std::wstring wFilter(FB::utf8_to_wstring(filter));
    std::wstring wPath(FB::utf8_to_wstring(filter));
 
	OPENFILENAME opf;
	opf.lStructSize = sizeof(OPENFILENAME);
    
	//memset(&opf, 0, sizeof(OPENFILENAME));

	//opf.lStructSize =  OPENFILENAME_SIZE_VERSION_400W; 
	//opf.lpTemplateName = NULL;
	opf.hwndOwner = wnd;
    //opf.lpstrFilter = L"All Files\0*.*\0\0";
    opf.lpstrFilter = L"Folders Only\0___.__\0";
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
		opf.Flags =  OFN_ENABLESIZING | OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_NOVALIDATE | OFN_ENABLEHOOK | OFN_ENABLEINCLUDENOTIFY | OFN_HIDEREADONLY;
		opf.lpstrTitle = L"Select folders...";
	} else {
	    opf.Flags =  OFN_ENABLESIZING | OFN_NOVALIDATE | OFN_EXPLORER | OFN_ENABLEHOOK | OFN_ENABLEINCLUDENOTIFY | OFN_HIDEREADONLY;
		opf.lpstrTitle = L"Select a folder...";
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
			out = FB::wstring_to_utf8(std::wstring(opf.lpstrFile));
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
		case WM_COMMAND:
			return TRUE;
	
		case WM_INITDIALOG:
			
			//CommDlg_OpenSave_SetControlText(hwnd, psh2, LPCSTR("Fupsh1"));
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
			HWND m_listView = GetDlgItem(pNmhdr->hwndFrom, lst1);

			if (pNmhdr->code == CDN_SELCHANGE)
			{ 
				/*
				TCHAR dummyBuffer = 1;

				UINT nfiles = CommDlg_OpenSave_GetSpec(pNmhdr->hwndFrom, &dummyBuffer, 1);
				UINT nfolders = CommDlg_OpenSave_GetFolderPath(pNmhdr->hwndFrom, &dummyBuffer, 1);
				
				wchar_t * m_fileBuffer = new wchar_t[(nfiles + 1)];
				wchar_t * m_folderBuffer = new wchar_t[(nfolders + 1)];
				
				memset(&m_fileBuffer, 0, nfiles +1);
				memset(&m_folderBuffer, 0, nfolders +1);

				CommDlg_OpenSave_GetFolderPath(pNmhdr->hwndFrom, m_folderBuffer, nfolders);
				CommDlg_OpenSave_GetSpec(pNmhdr->hwndFrom, m_fileBuffer, nfiles);
				
				
				LPWSTR m_strInclude = NULL;
				wsprintf(m_strInclude, L"nfolders: %d - nfiles: %d", nfolders, nfiles);
				lpofn->lpOFN->lpstrTitle = m_strInclude;
				*/

				if (m_folderChange)	{
					if (m_strPath != m_curDir) {
						m_strPathList = m_strPath;
						
					} else {
						memset(&m_strPathList,0, sizeof(m_strPathList));
					}
					m_folderChange = false;
				}
				CommDlg_OpenSave_SetControlText(pNmhdr->hwndFrom, edt1, m_strPathList.c_str());
				memset(&m_strPathList,0, sizeof(m_strPathList));
				
			}

			if (pNmhdr->code == CDN_INCLUDEITEM)
			{
				
				memset(&m_strPath, 0, sizeof(m_strPath));
				m_strPath = GetPath(hwnd, CDM_GETFILEPATH);
				
				if ( m_strPath.length() > 1 && m_strPath != m_curDir) {
						m_strPathList.append(m_strPath);
					if (lpofn->lpOFN->Flags & OFN_ALLOWMULTISELECT)  {
						m_strPathList.append( _T(":"));
					}
				} else {
					memset(&m_strPathList,0, sizeof(m_strPathList));
				}
			
			}
			else if (pNmhdr->code == CDN_FOLDERCHANGE)
			{
				//CommDlg_OpenSave_SetControlText(pNmhdr->hwndFrom, edt1, m_strPath.c_str());
				m_folderChange = true;
				m_curDir = m_strPath;
				memset(&m_strPath, 0, sizeof(m_strPath));
				memset(&m_strPathList, 0, sizeof(m_strPathList));

					
			}
			return FALSE;
	}
	return TRUE;
}

std::wstring GetPath(HWND hwnd, UINT nMessage)
{
	LPCTSTR strPath = _T("");
	
	HWND pWnd = GetParent(hwnd);

	if (pWnd && IsWindow(pWnd))
	{
		TCHAR szDir[MAX_PATH*3];
		memset(&szDir,0, sizeof(szDir));
		szDir[0] = _T('\0');
		SendMessage(pWnd, nMessage, sizeof(szDir)/sizeof(TCHAR)-2,
			(LPARAM)(LPCTSTR)szDir);
		szDir[sizeof(szDir)/sizeof(TCHAR)-1] = _T('\0');
		strPath = szDir;
	};
	
	return strPath;
}
