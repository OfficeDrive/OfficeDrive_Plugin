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

#ifndef DialogManagerX11_h__
#define DialogManagerX11_h__

#include <boost/noncopyable.hpp>
#include <string>

#include "../DialogManager.h"

class DialogManagerX11 : public DialogManager
{
public:
    void OpenFolderDialog(const FB::BrowserHostPtr& host, FB::PluginWindow* win, const PathCallback& cb);
    void _showFolderDialog(FB::PluginWindow* win, const PathCallback& cb);
    
protected:
    DialogManagerX11() {};
    ~DialogManagerX11() {};
    friend class DialogManager;
};
#endif // DialogManagerX11_h__
