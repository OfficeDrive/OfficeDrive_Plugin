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
#include <gtk/gtk.h>

#include "DialogManagerX11.h"
#include "BrowserHost.h"

DialogManager* DialogManager::get()
{
    static DialogManagerX11 inst;
    return &inst;
}

void DialogManagerX11::OpenFolderDialog(const FB::BrowserHostPtr& host, FB::PluginWindow* win, const PathCallback& cb)
{
    host->ScheduleOnMainThread(boost::shared_ptr<DialogManagerX11>(), boost::bind(&DialogManagerX11::_showFolderDialog, this, win, cb));
}

void DialogManagerX11::_showFolderDialog(FB::PluginWindow* win, const PathCallback& cb)
{
    std::string out;
    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new ("Open File",
				      NULL,
				      GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);
    //dialog->set_select_multiple(true);
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        //GSList *filenames;
        char *filename;
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        //out = (char*) filenames->data;
        out = filename;
        g_free (filename);
    }
    gtk_widget_destroy (dialog);
    cb(out);
    
}
