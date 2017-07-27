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
import os

from cas.util import UtilBase
from cas.cas_subprocess import Popen, PIPE, call

class CoreException(Exception):
    pass

class CoreBase(object):
    def __init__(self):
        self.util = UtilBase()
        
    def guess_format(self, data, fname):
        "Return a good default Operation, judging by the first 300 bytes or so."
        suffix_map = {  "tar" : ["tar", "xvf"],
                        "tgz" : ["tar", "xvzf"],
                        "gz"  : ["gunzip", "-q"],
                        "tbz" : ["tar", "xvjf"],
                        "bz2" : ["bunzip2", "-q"],
                        "zip" : ["unzip", "-f"]}
        def string(offset, match):
            return data[offset:offset + len(match)] == match

        # Archives
        if string(257, 'ustar\0') or string(257, 'ustar\040\040\0'):
            return suffix_map["tar"]
        if string(0, 'PK\003\004'): return suffix_map["zip"]
        if string(0, 'PK00'): return suffix_map["zip"]

        # Compressed streams
        if string(0, '\037\213'):
            if fname.endswith('.tar.gz') or fname.endswith('.tgz'):
                return suffix_map["tgz"]
            return suffix_map["gz"]
        if string(0, 'BZh') or string(0, 'BZ'):
            if fname.endswith('.tar.bz') or fname.endswith('.tar.bz2') or \
               fname.endswith('.tbz') or fname.endswith('.tbz2'):
                return suffix_map["tbz"]
            return suffix_map["bz2"]
        return False

    def extractCore(self, filepath):
        """ utility to extract archive and pull out core
        """
        # Making an assumption that if we are extracting a file then
        # the contents of that file are placed wherever cas is run from
        self.dst = os.path.realpath(os.curdir)
        self.filepath = filepath
        fd = open(self.filepath, 'rb')
        data = os.read(fd.fileno(), 1000)
        format = self.guess_format(data, self.filepath)
        if not format:
            raise CoreException("Can not determine compression format.")
        else:
            format.append(self.filepath)
            # TODO: figure out someway to print some status to the screen
            # during extraction
            # FIXME: failing to determine corefile after extraction, running
            # on extracted core works. possible problem being looking in the wrong
            # directory
            p = Popen(format, stdout=PIPE, stderr=PIPE)
            err = p.stderr.read()
            out = p.stdout.read()
            if err:
                raise CoreException("Unable to extract file: %s" % (err,))
            for root, dirs, files in self.util.directoryList(self.dst):
                for file in files:
                    if self.isCorefile(file):
                        return os.path.join(root,file)
        raise CoreException("Can not determine a corefile from tarball : %s" % (self.filepath,))

    def isCorefile(self, corefile):
        cmd = ["file","-i",corefile]
        p = Popen(cmd, stdout=PIPE, stderr=PIPE)
        txt = p.stdout.read()
        items = ['application/x-coredump',
                 'application/octet-stream',
                 'application/x-executable']
        for i in items:
            if i in txt:
                return True
        return False

    def timestamp(self, path, blksize=None):
        """ captures fingerprint from core
        """
        match='Linux\sversion.*20\d{1,2}|#1\s.*20\d{1,2}'
        try:
            fd=open('%s' % (path))
        except IOError:
            return False
        fd.seek(0)
        if not blksize or blksize == "":
            blksize = 540000000
        b = os.read(fd.fileno(),blksize)
        out = self.util.regexSearch(match, b)
        if out:
            return out
        raise CoreException("Unable to retrieve timestamp from: %s" % (path,))
