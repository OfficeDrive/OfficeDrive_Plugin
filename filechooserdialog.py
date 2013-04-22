import pygtk
import gtk

defd = gtk.gdk.display_get_default()
defs = defd.get_default_screen()

rootw = defs.get_root_window()

toplevel = rootw.get_effective_toplevel()

# fc = gtk.FileChooserDialog()
fc = gtk.FileChooserDialog('Open File', 
                            None, gtk.FILE_CHOOSER_ACTION_SELECT_FOLDER, 
                            (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL, gtk.STOCK_OPEN, gtk.RESPONSE_ACCEPT))

fc.realize()

fc.window.reparent(toplevel,0, 0)

fc.show()
