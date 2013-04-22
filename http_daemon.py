#!/bin/sh -
"exec" "python" "-O" "$0" "$@"

__doc__ = """http Daemon

This module implements GET, HEAD, POST, PUT and DELETE methods
on BaseHTTPServer,

"""

__version__ = "0.2.1-se"

import BaseHTTPServer, select, socket, SocketServer, urlparse, os, sys

class HTTPRequestHandler (BaseHTTPServer.BaseHTTPRequestHandler):
    __base = BaseHTTPServer.BaseHTTPRequestHandler
    __base_handle = __base.handle

    server_version = 'OfficeDrive Daemon' + __version__
    rbufsize = 0                        # self.rfile Be unbuffered
        
    def handle(self):
        (ip, port) =  self.client_address
        if hasattr(self, 'allowed_clients') and ip not in self.allowed_clients:
            self.raw_requestline = self.rfile.readline()
            if self.parse_request(): self.send_error(403)
        else:
            self.__base_handle()

  
    def do_CONNECT(self):
        soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            self.log_request(200)
            self.wfile.write(self.protocol_version +
                             " 200 Connection established\r\n")
            self.wfile.write("Proxy-agent: %s\r\n" % self.version_string())
            self.wfile.write("\r\n")
            self._read_write(soc,150)
        finally:
            sys.stdout.write('|')
            sys.stdout.flush()
            soc.close()
            self.connection.close()
         

    def do_GET(self):
        (scm, netloc, path, params, query, fragment) = urlparse.urlparse(
            self.path, 'http')
        if scm != 'http' or fragment:
            self.send_error(400, "bad url %s" % self.path)
            return
        soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.log_request(200)
         #self.wfile.write(self.protocol_version +
         #                    " 200 Connection established\r\n")
        #soc.send("%s %s %s\r\n" % (
        #     self.command,
        #        urlparse.urlunparse(('', '', path, params, query, '')),
        #        self.request_version))
        #self.headers['Connection'] = 'close'
        for key_val in self.headers.items():
            self.connection.send("%s: %s\r\n" % key_val)
        self.connection.send("%s\r\n" %self.request_version)
        self.connection.send("%s\r\n" %self.params)
        self._read_write(soc)
        sys.stdout.write('|')
        sys.stdout.flush()
        soc.close()
        self.connection.close()
       
    def _read_write(self, soc, max_idling=10):
        iw = [self.connection, soc]
        ow = []
        count = 0
        
        while 1:
            count += 1
            (ins, _, exs) = select.select(iw, ow, iw, 0.1)
            if exs: break
            if ins:
                for i in ins:
                    if i is soc:
                        out = self.connection
                    else:
                        out = soc
                    
                    data = i.recv(8192)
                    if data:
                        out.send(data)

			count = 0
            else:
                sys.stdout.write('.')
                sys.stdout.flush()
                print "\t" "idle", count
            if count == max_idling: break

    do_HEAD = do_GET
    do_POST = do_GET
    do_PUT  = do_GET
    do_DELETE=do_GET

class ThreadingHTTPServer (SocketServer.ThreadingMixIn,
                           BaseHTTPServer.HTTPServer): pass

if __name__ == '__main__':
    from sys import argv
    if argv[1:] and argv[1] in ('-h', '--help'):
        print argv[0], "[port [allowed_client_name ...]]"
    else:
        if argv[2:]:
            allowed = []
            for name in argv[2:]:
                client = socket.gethostbyname(name)
                allowed.append(client)
                print "Accept: %s (%s)" % (client, name)
            HTTPRequestHandler.allowed_clients = allowed
            del argv[2:]
        else:
            print "All clients will be served..."
        try:
            BaseHTTPServer.test(HTTPRequestHandler, ThreadingHTTPServer)
        except KeyboardInterrupt:
            print '\nExit requested. Open connections will be served.\n'
