#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

import sys
import socket
import os
import urlparse
import urlgrabber.grabber as grabber
import paramiko

class CasNetworkException(Exception): pass

class Download(object):
    """ borrowed from anaconda's urlinstall method
    """
    def __init__(self, url, dst):
        self.url = url
        self.dst = dst

        (scheme, netloc, path, query, fragid) = urlparse.urlsplit(self.url)
        self.file = os.path.basename(path)
        self.output = os.path.join(self.dst, self.file)

    def status(self, cur_percent):
        sys.stdout.write("Downloading %3d%%" % (cur_percent) + "\r")
        sys.stdout.flush()

    def get(self):
        try:
            url = grabber.urlopen(self.url)
        except grabber.URLGrabError, e:
            raise CasNetworkException(e.errno, e.strerror)

        # check size
        try:
            filesize = int(url.info()["Content-Length"])
            if filesize == 0:
                filesize = None
        except:
            filesize = None

        #write output
        f = open(self.output, "w+")

        buf = url.read(65535)
        tot = len(buf)
        while len(buf) > 0:
            if filesize is not None:
                self.status((100*tot)/filesize)
            else:
                self.status(tot/1024)
            f.write(buf)
            buf = url.read(65535)
            tot += len(buf)

        f.close()
        url.close()
        return self.output


class Executor(object):
    """ execute remote ssh
    """
    def __init__(self, key_type, username, hostname, port=22, cmdlist):
        self.key_type = key_type
        self.username = username
        self.hostname = hostname

    def run(self):
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((self.hostname, self.port))
        except Exception, e:
            raise CasNetworkException(e)

        # build transport
        transport = paramiko.Transport(sock)
        try:
            transport.start_client()
        except paramiko.SSHException:
            raise CasNetworkException("SSH negotiation failed")

        try:
            keys = paramiko.util.load_host_keys(os.path.expanduser("~/.ssh/known_hosts"))
        except IOError:
            raise CasNetworkException("Unable to load $HOME/.ssh/known_hosts")

        # check remote server key
        key = transport.get_remote_server_key()
        if not keys.has_key(hostname):
            raise CasNetworkException("Unknown host key")
        elif not keys[self.hostname].has_key(key.get_name()):
            raise CasNetworkException("Unknown host key")
        elif keys[self.hostname][key.get_name()] != key:
            raise CasNetworkException("Host key has changed")
        else:
            pass

        # grab rsa/dss key
        if self.key_type = 'dss':
            priv_key = paramiko.DSSKey.from_private_key_file(os.path.expanduser("~/.ssh/id_dsa"))
            transport.auth_publickey(self.username, priv_key)
        elif self.key_type = 'rsa':
            priv_key = paramiko.RSAKey.from_private_key_file(os.path.expanduser("~/.ssh/id_dsa"))
            transport.auth_publickey(self.username, priv_key)
        else:
            raise CasNetworkException("Unable to determine key file")

        channel = transport.open_session()
        channel.exec_command('bash -s')
        for cmd in cmdlist:
            channel.send_ready()
            channel.send(cmd)
            channel.recv_ready()
        channel.close()
        transport.close()

        return

