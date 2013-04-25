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

#ifndef DialogManagerWin_h__
#define DialogManagerWin_h__

#include <boost/noncopyable.hpp>
#include <string>
#include "../DialogManager.h"

UINT CALLBACK FolderHook(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

std::wstring GetPath(HWND hwnd, UINT nMessage);

std::wstring m_strPath;
std::wstring m_strPathList;
std::wstring m_curDir;

bool m_folderChange;

class DialogManagerWin : public DialogManager
{
public:
	
	void OpenFileDialog(const FB::BrowserHostPtr& host, FB::PluginWindow* win, const PathCallback& cb, const int fileType, const bool multiple, const int fileOperation);
	void OpenFolderDialog(const FB::BrowserHostPtr& host, FB::PluginWindow* win, const PathCallback& cb, const int fileType, const bool multiple, const int fileOperation);
	
protected:
    DialogManagerWin() {};
    ~DialogManagerWin() {};
	
    void _showFileDialog(HWND wnd, const std::string& path, const std::string& filter, const PathCallback& cb, const bool multiple, const int fileOperation);
    void _showFolderDialog(HWND wnd, const PathCallback& cb);
	void _showFolderDialog2(HWND wnd, const std::string& path, const int fileType, const PathCallback& cb, const bool multiple, const int fileOperation);
	
    friend class DialogManager;

};

//class CDialogEventHandler : public IFileDialogEvents,
//                            public IFileDialogControlEvents	{
//public:
//    // IUnknown methods
//    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv);
//
//};
//
#endif // DialogManagerWin_h__