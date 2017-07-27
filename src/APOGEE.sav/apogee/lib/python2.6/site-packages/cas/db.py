import os
import sys

if sys.version_info[:2] > (2,4):
    import sqlite3 as sqlite
else:
    try:
        import sqlite
    except ImportError:
        raise SystemExit('Unable to determine sqlite, please make sure it is installed.')

class CasStorageException(Exception): pass

class CasStorage(object):
    def __init__(self, database):
        """ setup database connection and return db cursor for
        traversing database """
        self.db = database
        self.conn = None
        self.cursor = None

    def connect(self):
        """ execute connection """
        try:
            self.conn = sqlite.connect(self.db)
            self.cursor = self.conn.cursor()
        except:
            raise CasStorageException('Cannot connect to database')
        return

    def commit(self):
        self.conn.commit()
        return

    def buildTable(self):
        # We want to add jobs to a database just in case
        # there is no structured form of purging old data.
        self.cursor.execute("""create table jobs (
            job_id integer primary key autoincrement,
            identifier integer,
            date text,
            email text)
            """)
        self.cursor.execute("""create table debuginfo (
            debug_id integer primary key autoincrement,
            rpm text)
            """)
        self.cursor.execute("""create table timestamp (
            timestamp_id integer primary key autoincrement,
            debugpath text,
            timestamp text,
            debug_id integer)
            """)
        self.cursor.execute("""create table server (
            server_id integer primary key autoincrement,
            arch text,
            port text,
            hostname text)
            """)
        self.commit()
        return

    # DEBUGINFO METHODS
    def addDebuginfoRPM(self, debuginfo):
        debuginfo = (debuginfo,)
        self.cursor.execute('SELECT * from debuginfo where rpm="%s"' % debuginfo)
        if not self.cursor.fetchone():
            self.cursor.execute('INSERT into debuginfo(rpm) values("%s")' % debuginfo)
        self.commit()
        return

    def getAllDebuginfoRPM(self):
        self.cursor.execute("SELECT * FROM debuginfo")
        return self.cursor.fetchall()

    # TIMESTAMP METHODS
    def addTimestamp(self, id, debug, timestamp):
        """ build relation to debuginfo rpm and add debug path, timestamp """
        values = (id, debug, timestamp)
        self.cursor.execute('SELECT * FROM timestamp where timestamp="%s"' % (timestamp,))
        if not self.cursor.fetchone():
            self.cursor.execute('''INSERT into timestamp (debug_id,debugpath,
        timestamp) values(%d,"%s","%s")''' % values)
        self.commit()
        return

    def getTimestampDebug(self, timestamp):
        """ return timestamp based on extracted core timestamp """
        values = (timestamp,)
        self.cursor.execute('''SELECT rpm,debugpath 
                               FROM debuginfo, timestamp 
                               where timestamp.timestamp LIKE "%%%s%%"
                               AND debuginfo.debug_id = timestamp.debug_id''' % values)
        return self.cursor.fetchone()

    # JOB METHODS
    def getAllJobs(self):
        """ all jobs """
        self.cursor.execute('SELECT * FROM jobs')
        return self.cursor.fetchall()

    def getJobById(self, id):
        """ single job """
        self.cursor.execute('SELECT * FROM jobs where id=%d' % (id,))
        return self.cursor.fetchone()

    def getJobRange(self, days):
        """ provides jobs based on creation date from
        $days back
        """
        pass

    def addJob(self, identifier, date, email):
        """ add job to db """
        if email is None:
            email = "cas@localhost"
        values = (str(date), int(identifier), str(email))
        self.cursor.execute('''INSERT into jobs (date,identifier,email)
        values ("%s",%d, "%s")''' % values)
        self.commit()
        return

    # SERVER METHODS
    def addServer(self, hostname, port, arch):
        """ add server/arch to db """
        values = (hostname, port, arch)
        self.cursor.execute('select * from server where hostname="%s"' % (hostname,))
        if not self.cursor.fetchone():
            self.cursor.execute('''INSERT into server (hostname, arch, port)
        values ("%s","%s","%s")''' % values)
        self.commit()
        return

    def getServers(self):
        self.cursor.execute('select hostname, port, arch from server')
        return self.cursor.fetchall()
