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
#ifndef DialogManager_h__
#define DialogManager_h__

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include "BrowserHost.h"


typedef boost::function<void (const std::string&)> PathCallback;

namespace FB { class PluginWindow; }

class DialogManager
{
public:

    static DialogManager* get();
	
    virtual void OpenFolderDialog(const FB::BrowserHostPtr& host, FB::PluginWindow* win, const PathCallback& cb, const int fileType, const bool multiple, const int fileOperation) = 0;
    virtual void OpenFileDialog(const FB::BrowserHostPtr& host, FB::PluginWindow* win, const PathCallback& cb, const int fileType, const bool multiple, const int fileOperation) = 0;
protected:
    DialogManager() {}
    virtual ~DialogManager() {}
};

#endif // DialogManager_h__
