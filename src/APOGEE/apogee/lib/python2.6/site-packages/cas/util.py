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

import cPickle
import commands
import os
import re
import sys
import logging
import cas.cas_shutil as shutil

from cas.cas_subprocess import Popen, PIPE, call

class Logging(object):
    def __init__(self, dst, logger_id, debug_level='DEBUG'):
        self.debug_level = debug_level
        self.dst = dst
        self.logger_id = logger_id
        self.logfile = logger_id+".log"
        self.logfile  = os.path.join(self.dst, self.logfile)
        self.log = logging.getLogger(self.logger_id)
        ch = logging.StreamHandler()
        # never fail if directory doesn't exist.
        if not os.path.exists(os.path.dirname(self.logfile)):
            os.makedirs(os.path.dirname(self.logfile))
        fh = logging.FileHandler(self.logfile)
        self.log.addHandler(ch)
        self.log.addHandler(fh)
        ch_fmt = logging.Formatter("%(message)s")
        fh_fmt = logging.Formatter("%(asctime)s %(process)d (%(levelname)s)\t: %(message)s")
        ch.setFormatter(ch_fmt)
        fh.setFormatter(fh_fmt)
        logging_level = {'DEBUG': logging.DEBUG,
                         'INFO' : logging.INFO}
        self.log.setLevel(logging_level[self.debug_level])

    def debug(self, msg):
        if self.debug_level == 'DEBUG':
            self.log.debug("[.cas.][debug] :: %s" % (msg,))
    
    def status(self, msg):
        """ function to print status messages
        """
        sys.stdout.write("[.cas.] :: " + msg + "\r")
        sys.stdout.flush()
        
    def info(self, msg):
        """ function to print procedure
        """
        self.log.info("[.cas.] :: %s" % (msg,))

class UtilException(Exception): pass

class UtilBase(object):
    def regexFindall(self, regex, string):
        ''' Return a list of all non overlapping matches in the string(s)
        '''
        out=[]
        reg=re.compile(regex, re.MULTILINE)
        for i in reg.findall(string):
            out.append(i)
        if len(out):
            return out
        return False

    def regexSearch(self, regex, string):
        reg=re.compile(regex, re.MULTILINE)
        try:
            return reg.search(string).group()
        except:
            return False

    def directoryList(self, filepath):
        """ file listing with symlink support
        """
        for root, dirs, files in os.walk(filepath):
            yield root, dirs, files
            for dir in dirs:
                dirpath = os.path.join(root, dir)
                if os.path.islink(dirpath):
                    for val in self.directoryList(dirpath):
                        yield val

    def make_exe(self, fn):
        ''' make fn executable
        '''
        if os.name == 'posix':
            oldmode = os.stat(fn).st_mode & 07777
            newmode = (oldmode | 0555) & 07777
            os.chmod(fn, newmode)
        return

    def save(self, obj, fname):
        ''' push data to pickle
        '''
        FILE=open(fname, 'w')
        cPickle.dump(obj, FILE)
        FILE.close()
        return

    def load(self, fname):
        ''' grab data from pickle file
        '''
        if os.path.isfile(fname):
            FILE=open(fname, 'r')
            out=cPickle.load(FILE)
            FILE.close()
        else:
            raise UtilException("%s : Unable to locate/load file." % (fname,))
        return out

    def getElfArch(self, debug):
        """ determine machine type for ELF file
        """
        supportArch = {"IBM S/390":"s390x",
                       "Intel 80386":"i686",
                       "Advanced Micro Devices X86-64" : "x86_64",
                       "Intel IA-64": "ia64",
                       "PowerPC64": "ppc64"}

        # readelf pulls in various information about elf object files
        # we only care about data in the elf header at the start of file
        # more specifically the machine type
        cmd = ["readelf", "-h", debug]
        cmd2 = ["grep", "Machine"]
        pipe = Popen(cmd, stdout=PIPE, stderr=PIPE)
        pipe2 = Popen(cmd2, stdin=pipe.stdout, stdout=PIPE, 
            stderr=PIPE).communicate()
        machine, sts = pipe2
        for k, v in supportArch.iteritems():
            if k in machine:
                return supportArch[k]
        return False

    def buildCrashFile(self, dst, vmcore, debug, file_in="crash.in",
                       crash_bin="/usr/bin/crash"):
        """ build crash and crash.in file with output from the snippets in 
        /var/lib/cas/snippets
        """
        # TODO: We should provide the environment variables mentioned in the
        # template snippet, CAS_ARCH, CAS_KERNEL, CAS_SIZE, and whatever else
        # makse sense so scripts can opt in based on the environmentals
        dir_listing=[]
        crashInputCmds=[]
        for root, dirs, files in os.walk("/var/lib/cas/snippets/"):
            for name in files:
                dir_listing.append(os.path.join(root, name))
        for a in dir_listing:
            crashInputCmds.append(Popen(a, stdout=PIPE).communicate()[0])
        crashInputCmds.append("exit\n")

        # Build crash input file
        crashInputPath = os.path.join(dst, file_in)
        crashInputFH = open(crashInputPath, "w")
        crashInputFH.write("".join(crashInputCmds))
        crashInputFH.close()

        vmcorePath = os.path.join(dst, vmcore)
        crashCmd = "#!/bin/sh\n%s %s %s $*\n" % (crash_bin, vmcorePath, 
                                                      debug )
        crashExe = os.path.join(dst,"crash")
        fh = open(crashExe,"w")
        fh.write(crashCmd)
        fh.close()
        self.make_exe(crashExe)
        return

