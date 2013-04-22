#!/usr/bin/env python
#
# OfficeDrive Daemon 
#

"""
OfficeDrive Daemon - file io over http, https and webdav
encryption, status
"""

import os
import sys
import socket
import re
import time

import pygtk
import gtk
import gobject

#import pygst
#import gst

import json

class OfficeDriveStatusIcon(gtk.StatusIcon):
    scriptpath, scriptname = os.path.split(sys.argv[0])
    scriptpath = os.path.realpath(scriptpath)
    try:
        f = open('%s/officedrive_prefs.json' %scriptpath)
	conf = json.load(f)
        f.close()
    except IOError,e:
	print e
	exit(1)

    username = conf.get('username') 
    password = conf.get('password')
    transponder = conf.get('transponder')
    me = conf.get('me')
 
    def __init__(self):
        gtk.StatusIcon.__init__(self)
        self.isatty = sys.stdout.isatty()

        self.appname = os.path.splitext( self.scriptname )[0]
        self.author = [ '<simon.engelsman@gmail.com>' ]
        self.messageicon = os.path.join( self.scriptpath, 'officedrive.png' )
        self.idleicon = os.path.join( self.scriptpath, 'officedrive_idle.png')

	self.messages = []
        self.cmenu = gtk.Menu()
        self.build_menu()        
        self.set_from_file(self.idleicon)

        self.connect( 'popup-menu', self.popup_menu, self.cmenu )
        self.connect( 'activate', self.view_messages)

        self.messageview = MessageConsole(self)
       
        self.recv_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        #self.recv_socket.setsockopt( socket.SOL_SOCKET, socket.SO_REUSEADDR, 1 )
        self.recv_socket.bind( tuple( self.me[1] ))
        self.recv_socket.listen(1)
        
        sockwatch = gobject.io_add_watch( self.recv_socket.fileno(), gobject.IO_IN, self.handle_sock_io )
        
        message_sound = 'soundcomputersound5.wav'
        idle_sound = 'soundcommunicator.wav'
        soundpath = os.path.realpath(os.path.split(sys.argv[0])[0])
        self.message_sound = '%s/%s' % (soundpath, message_sound)
        self.idle_sound = '%s/%s' % (soundpath, idle_sound)
		# self.sound_player = SoundPlayer(self.message_sound, 50)
        
    def show_message( self, message ):
        
        if 'ack_all' in message:
            messageicon = self.idleicon
            ack = True
        else:
            ack = False
            messageicon = self.messageicon
        
        self.messageview.update(self.transponder[0], message)
        self.messageview.show_all()

        self.set_from_file(messageicon)
        self.messages.append ([time.strftime( '%H:%M:%S' ),message, 'in' ])
        tooltip = ''
  
        for msg in self.messages:
            tooltip += "%s\t%s" % (msg[0],msg[1])
        
        self.set_tooltip( "%s\n%s" % ( self.appname, tooltip ))

    def close(self, *args):
        try:
	    print 'DEBUG: terminating daemon'
	    #if self.sound_player:
	    #    self.sound_player.command( 'quit' )
            #    self.sound_player._mplayer.terminate()
            #    self.sound_player._clean_up()
        except Exception,e:
            print e
            pass
        gtk.main_quit()
        
    def play_sound (self, soundfile=None):
        """
        play a soundfile
        """
        if soundfile:
            soundfile = soundfile
	else:
	    soundfile = self.idle_sound

        self.sound_player.set_state(gst.STATE_PLAYING)

        print 'playing soundfile: %s'  % (soundfile)
        

    def handle_sock_io(self, fd, condition):
        self.peer_socket, self.peer_address = self.recv_socket.accept()
        self.recv_socket.close()
        self.peer_socket.setblocking(0)
        
        data = buffer(self.peer_socket.recv(1024))
        if data:
            if 'cmd_ping' in data:
                self.ping(data)
                #self.play_sound(soundfile=self.idle_sound)
            else:
                self.show_message(data)
                #self.play_sound(soundfile=self.message_sound)   
        return True
            
    def acknowledge_all(self,data):
        s = socket.socket()
        try:
            s.connect(tuple(self.transponder[1]))
            s.send( 'ack_all\n' )
            s.close()
        except socket.error, e:
            print 'fail :-( %s' %e
            return -1
        
        return False
        
    def clear(self, data):
        self.messages = []
        self.set_tooltip( None )
        self.set_from_file( self.idleicon )
    
    def copy_messages(self, data):    
        text = str(self.get_tooltip_text())
        if text:
            self.copy_to_clipboard( text )

    def copy_to_clipboard(self, data):
        clipboard = gtk.clipboard_get()
        clipboard.set_text( data )
        clipboard.store()
     
    def about (self, data):
        about = gtk.AboutDialog()
        about.set_authors( self.author )
        about.set_program_name( self.appname )
        about.set_title( "About " + self.appname )
        about.set_comments(__doc__)
        about.set_logo (self.get_pixbuf())
        response = about.run()
        about.destroy()

    
    def ping(self, data, cmd=''):
        p = socket.socket()
        if 'msg:' in cmd:
            ping_msg = '%s\n' %self.messageview.msg_entry.get_text() 
            self.messageview.update(self.me[0], ping_msg)
            self.messageview.msg_entry.set_text('')
        else:
            ping_msg = 'ping from %s :-)\n' %self.me[0]
        try:
            p.connect(tuple( self.transponder[1] ))
            p.send(ping_msg)
            p.close()
            self.messages.append([ time.strftime( "%H:%M:%S" ), ping_msg , 'out'] )
            self.set_from_file(self.idle_icon)
        except socket.error, e:
            print 'fail :-( %s' %e
            self.messageview.messagebuffer.insert_at_cursor( 'fail :-(\n%s\n'  %e )
            return -1
        return True
        
       
    def popup_menu(self, icon, button, time, menu):
        if self.cmenu:
            self.cmenu.show_all()
            self.cmenu.popup( None, None, None, 0, time )

    def view_messages(self, data):
        if self.messageview:
            self.messageview.show_all()
            return
        text = ''
        for msg in self.messages:
            if msg[-1] == 'in':
                text += '%s <%s> \t%s' %(msg[0], self.transponder[0], msg[1])
            else:
                text += '%s <%s> \t%s' %(msg[0], self.me[0], msg[1])

        self.messageview = MessageConsole(self, text)

    def build_menu(self):
        menu_item = gtk.ImageMenuItem( gtk.STOCK_ABOUT )
        menu_item.connect( 'activate', self.about )
        self.cmenu.append( menu_item )
        menu_item = gtk.ImageMenuItem( 'Ping <%s>' %self.transponder[0], gtk.STOCK_NEW )
        menu_item.connect( 'activate', self.ping )
        self.cmenu.append( menu_item )
        menu_item = gtk.ImageMenuItem( 'Ping + txt <%s>' %self.transponder[0], gtk.STOCK_NEW )
        menu_item.connect( 'activate', self.view_messages )
        self.cmenu.append( menu_item )
        menu_item = gtk.MenuItem( 'Acknowledge all' )
        menu_item.connect( 'activate', self.acknowledge_all )
        self.cmenu.append( menu_item )
        menu_item = gtk.ImageMenuItem( gtk.STOCK_CLEAR )
        menu_item.connect( 'activate', self.clear )
        self.cmenu.append( menu_item )
        # self.disable_sound = gtk.CheckMenuItem('Disable sound')
        # self.cmenu.append( self.disable_sound )
        menu_item = gtk.SeparatorMenuItem()
        self.cmenu.append( menu_item )
        
        menu_item = gtk.SeparatorMenuItem()
        self.cmenu.append( menu_item )
        
        menu_item = gtk.ImageMenuItem( gtk.STOCK_QUIT )
        menu_item.connect( 'activate', self.close )
        self.cmenu.append( menu_item )

class MessageConsole(object):
    def __init__( self, statusicon, data='' ):    
        text = data
        self.statusicon = statusicon
        icon = self.statusicon.get_pixbuf()

        w = gtk.Window(gtk.WINDOW_TOPLEVEL)
        
        w.set_border_width(1)
        w.set_default_size(480, 240)
	w.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
        w.set_icon(icon)
	w.set_title(self.statusicon.appname)

        sw = gtk.ScrolledWindow()
        sw.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        box = gtk.VBox()

        self.textview = gtk.TextView()
        self.textview.set_editable(False)
        self.textview.set_cursor_visible(True)
        
        self.messagebuffer = self.textview.get_buffer()
        self.messagebuffer.set_text(text)

        sw.add( self.textview )
        box.pack_start( sw, True, True, 0 )        

        self.msg_entry = gtk.Entry()
        box.pack_end( self.msg_entry, False, False, 0 )
        self.msg_entry.connect('activate', self.statusicon.ping, 'msg:')
        self.msg_entry.grab_focus()
        
        w.add( box )
        self.show_all = w.show_all
        self.hide = w.hide
                    
        w.connect("delete_event", self.close )
        w.connect("destroy_event", self.close )
        
        self.messagemark = gtk.TextMark( 'messagemark', False)
        self.messagebuffer.add_mark(self.messagemark, self.messagebuffer.get_end_iter() )
	self.textview.scroll_to_mark( self.messagemark, 0.0 )
    
    def update(self, peer, message):
	self.messagebuffer.insert( self.messagebuffer.get_end_iter(), '%-9s <%s> %-s'  %( time.strftime( '%H:%M:%S' ), peer, message ))
        self.messagebuffer.move_mark( self.messagemark, self.messagebuffer.get_end_iter() )
	self.textview.scroll_to_mark( self.messagemark, 0.0 )
     
    def close(self, *args):
        self.hide()
        return True
"""    
class SoundPlayer:
    def __init__(self, uri, volume):
        self.player = gst.element_factory_make ('playbin', 'player0')
       
        if volume > 1.0:
            volume = 1.0
            
        self.player.set_property ('uri', uri)
        self.player.set_property ('volume', volume)
    
        bus = self.player.get_bus()
        bus.add_signal_watch()
        
        bus.enable_sync_message_emission()
        bus.connect('message', self.on_message)
   
    def on_message(self, bus, message):
        t = message.type
        if t == gst.MESSAGE_EOS:
            self.player.set_state(gst.STATE_NULL)
        elif t == gst.MESSAGE_ERROR:
            self.player.set_state(gst.STATE_NULL)
"""
if __name__ == "__main__" :

    app = OfficeDriveStatusIcon()
    gtk.main()
            
