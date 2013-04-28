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

#include <string>
#include <boost/thread.hpp>
#include <AppKit/AppKit.h>
#include <Cocoa/Cocoa.h>
#include "logging.h"

#include "DialogManagerMac.h"
#include "BrowserHost.h"

DialogManager* DialogManager::get()
{
    static DialogManagerMac inst;
    return &inst;
}


void DialogManagerMac::OpenFolderDialog(const FB::BrowserHostPtr& host, FB::PluginWindow* win, const PathCallback& cb, const int fileType, const bool multiple, const int fileOperation)
{
    host->ScheduleOnMainThread(boost::shared_ptr<DialogManagerMac>(), boost::bind(&DialogManagerMac::_showFolderDialog, this, win, cb, fileType, multiple));
}

void DialogManagerMac::OpenFileDialog(const FB::BrowserHostPtr& host, FB::PluginWindow* win, const PathCallback& cb, const int fileType, const bool multiple, const int fileOperation)
{
    host->ScheduleOnMainThread(boost::shared_ptr<DialogManagerMac>(), boost::bind(&DialogManagerMac::_showFolderDialog, this, win, cb, fileType, multiple));
}


void DialogManagerMac::_showFolderDialog(FB::PluginWindow* win, const PathCallback& cb, const int fileType, const bool multiple)
{
    FBLOG_INFO("DialogManagerMac", "Showing folder dialog");
    std::string out;
    int result;

    NSAutoreleasePool* pool = [NSAutoreleasePool new];
    NSOpenPanel *oPanel = [NSOpenPanel openPanel];

    [oPanel setCanChooseDirectories:YES];

    if (multiple) {
		[oPanel setAllowsMultipleSelection:YES];
    } else {
 		[oPanel setAllowsMultipleSelection:NO];
    }
	
	if (fileType == 2) {
	    [oPanel setCanChooseFiles:NO];
	} else {
	    [oPanel setCanChooseFiles:YES];
	}

    result = [oPanel runModalForDirectory:nil
                                     file:nil types:nil];
    
    if (result == NSOKButton) {
        NSArray *filesToOpen = [oPanel filenames];

    	int i = 0;
	NSString *aSelection = [NSString stringWithFormat:@"%s",""];
	for (i=0; i < filesToOpen.count; i++) {
    	    NSString *aFile = [filesToOpen objectAtIndex:i];
	    aSelection = [ aSelection stringByAppendingString:aFile];
	    aSelection = [ aSelection stringByAppendingFormat:@"%s","::"];
	}
     
		out = [aSelection cStringUsingEncoding:[NSString defaultCStringEncoding]];
        FBLOG_INFO("DialogManagerMac", "Folder selected: " << out);
    }
    [pool release];
    cb(out);
}
