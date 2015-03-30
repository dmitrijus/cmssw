#!/usr/bin/env python

import argparse
import subprocess
import socket, fcntl, select, atexit, signal
import sys, os, time, datetime
import collections
import json
import zlib

def log(s):
    sys.stderr.write("m: " + s + "\n");
    sys.stderr.flush()

def dt2time(dt):
    # convert datetime timstamp to unix
    return time.mktime(dt.timetuple())

class JsonEncoder(json.JSONEncoder):
        def default(self, obj):
            if hasattr(obj, 'to_json'):
                x = obj.to_json()
                return json.JSONEncoder.encode(self, x)

            return json.JSONEncoder.default(self, obj)

class ElasticReport(object):
    def __init__(self, pid, args):
        self.s_path = "/tmp/dqm_monitoring/"

        self.last_make_report = None
        self.make_report_timer = 30
        self.seq = 0
        self.args = args
        
        self.doc = {
            "pid": pid,
            "hostname": socket.gethostname(),
            "sequence": self.seq,
            "cmdline": args.pargs,
        }

        self.defaults()

    def defaults(self):
        self.id_format = u"%(type)s-run%(run)06d-host%(hostname)s-pid%(pid)06d"
        self.doc["type"] = "dqm-source-state"
        self.doc["run"] = 0

        # figure out the tag
        c = self.doc["cmdline"]
        for l in c:
            if l.endswith(".py"):
                t = os.path.basename(l)
                t = t.replace(".py", "")
                t = t.replace("_cfg", "")
                self.doc["tag"] = t

            pr = l.split("=")
            if len(pr) > 1 and pr[0] == "runNumber" and pr[1].isdigit():
                run = long(pr[1])
                self.doc["run"] = run

        self.make_id()

        if os.environ.has_key("GZIP_LOG"):
            self.doc["stdlog_gzip"] = os.environ["GZIP_LOG"]

        self.update_doc({ "extra": {
            "environ": dict(os.environ)
        }})

    def make_id(self):
        id = self.id_format % self.doc
        self.doc["_id"] = id
        return id

    def update_doc_recursive(self, old_obj, new_obj):
        for key, value in new_obj.items():
            if (old_obj.has_key(key) and 
                isinstance(value, dict) and 
                isinstance(old_obj[key], dict)):

                self.update_doc_recursive(old_obj[key], value)
            else:
                old_obj[key] = value

    def update_doc(self, keys):
        self.update_doc_recursive(self.doc, keys)

    def update_ps_status(self):
        try:
            pid = int(self.doc["pid"])
            fn = "/proc/%d/status" % pid
            f = open(fn, "r")
            d = {}
            for line in f:
                k, v = line.strip().split(":", 1)
                d[k.strip()] = v.strip()
            f.close()

            self.update_doc({ 'extra': { 'ps_info': d } })
        except:
            pass

    def update_mem_status(self):
        try:
            key = str(time.time())

            pid = int(self.doc["pid"])
            fn = "/proc/%d/statm" % pid
            f = open(fn, "r")
            dct = { key: f.read().strip() }
            f.close()

            self.update_doc({ 'extra': { 'mem_info': dct } })
        except:
            pass

    def make_report(self):
        self.last_make_report = time.time()
        self.doc["report_timestamp"] = time.time()
        self.make_id()

        if not os.path.isdir(self.s_path):
            # don't make a report if the directory is not available
            return

        self.update_ps_status()
        self.update_mem_status()

        fn_id = self.doc["_id"] + ".jsn"

        fn = os.path.join(self.s_path, fn_id) 
        fn_tmp = os.path.join(self.s_path, fn_id + ".tmp") 

        with open(fn_tmp, "w") as f:
            json.dump(self.doc, f, indent=True, cls=JsonEncoder)

        os.rename(fn_tmp, fn)

        if self.args.debug:
            log("File %s written." % fn)

    def try_update(self):
        # first time
        if self.last_make_report is None:
            return self.make_report()

        now = time.time()
        delta = now - self.last_make_report
        if delta > self.make_report_timer:
            return self.make_report()

    def ping(self):
        self.try_update()

class LineHistoryEnd(object):
    def __init__(self, max_bytes=64*1024, max_lines=1024):
        self.max_bytes = max_bytes
        self.max_lines = max_lines

        self.buf = collections.deque()
        self.size = 0

    def pop(self):
        elm = self.buf.popleft()
        self.size -= len(elm)

    def push(self, rbuf):
        self.buf.append(rbuf)
        self.size += len(rbuf)

    def write(self, line):
        line_size = len(line)

        while (self.size + line_size) > self.max_bytes:
            self.pop()

        while (len(self.buf) + 1) > self.max_lines:
            self.pop()

        self.push(line)

    def dump(self):
        return "".join(self.buf)

    def to_json(self):
        return self.dump()

class LineHistoryStart(LineHistoryEnd):
    def __init__(self, *kargs, **kwargs):
        LineHistoryEnd.__init__(self, *kargs, **kwargs)
        self.done = False

    def write(self, line):
        if self.done:
            return

        if ((self.size + len(line)) > self.max_bytes):
            self.done = True
            return

        if (len(self.buf) > self.max_lines):
            self.done = True
            return

        self.push(line)

class FDManager(object):
    def __init__(self):
        self.fd_map = {}
        self.timeout_handlers = []

        self.poll = select.poll()
    
    def register_fd(self, fd, mask, handler):
        if self.fd_map.has_key(fd):
            raise Exception("FD Already registered.")

        self.poll.register(fd, mask)
        self.fd_map[fd] = handler

    def unregister_fd(self, fd):
        self.poll.unregister(fd)
        del self.fd_map[fd]

    def register_timeout(self, handler):
        if handler in self.timeout_handlers:
            raise Exception("Timeout handler already registered.")

        self.timeout_handlers.append(handler)

    def unregister_timeout(self, handler):
        self.timeout_handlers.remove(handler)

    def event_loop(self, timeout=30):
        while self.fd_map:
            evts = self.poll.poll(int(timeout*1000))

            if len(evts) == 0:
                for th in self.timeout_handlers:
                    th.handle_timeout()

            for fd, evt in evts:
                handler = self.fd_map[fd]
                handler(fd, evt)
                
class FDLineReader(object):
    def __init__(self, manager, fd):
        self.manager = manager
        self.fd = fd
        self.line_buf = []
        
        self.start_fd()

    def start_fd(self):
        self.manager.register_fd(self.fd, select.POLLIN, self.handle_read)

    def stop_fd(self):
        self.manager.unregister_fd(self.fd)

    def handle_line(self, line):
        log("Received line: %s" % repr(line))

    def handle_read(self, fd, event):
        rbuf = os.read(fd, 1024*16)
        if len(rbuf) == 0:
            # closing fd
            if len(self.line_buf):
                self.handle_line("".join(self.line_buf))
                self.line_buf = []
            self.stop_fd()

        self.line_buf.append(rbuf)
        if "\n" in rbuf:
            # split whatever we have
            spl = "".join(self.line_buf).split("\n")

            while len(spl) > 1:
                line = spl.pop(0)
                self.handle_line(line + "\n")

            if len(spl[0]):
                self.line_buf = [spl[0]]
            else:
                self.line_buf = []

    def fd_close(self):
        pass

class FDJsonInput(FDLineReader):
    def __init__(self, manager, fd, es):
        self.es = es

        FDLineReader.__init__(self, manager, fd)

    def handle_line(self, line):
        try:
            doc = json.loads(line)

            for k in ["pid", "run", "lumi"]:
                if doc.has_key(k):
                    doc[k] = int(doc[k])

            self.es.update_doc_recursive(self.es.doc, doc)
            self.es.ping()
        except:
            log("cannot deserialize json: %s" % line)

class FDHistory(FDLineReader):
    def __init__(self, manager, fd, es):
        self.es = es

        self.start = LineHistoryStart();
        self.end = LineHistoryEnd()

        self.es.update_doc({ 'extra': { 'stdlog': self.start } })
        self.es.update_doc({ 'extra': { 'stdlog_start': self.end } })

        FDLineReader.__init__(self, manager, fd)

    def handle_line(self, line):
        #log("Received log line: %s" % repr(line))

        sys.stdout.write(line)
        sys.stdout.flush()
        
        self.start.write(line)
        self.end.write(line)
        self.es.ping()

def create_fifo():
    prefix = "/tmp"
    if os.path.isdir("/tmp/dqm_monitoring"):
        prefix = "/tmp/dqm_monitoring"

    base = ".es_monitoring_pid%08d" % os.getpid()
    fn = os.path.join(prefix, base)

    if os.path.exists(fn):
        os.unlink(fn)

    os.mkfifo(fn, 0600)
    if not os.path.exists(fn):
        log("Failed to create fifo file: %s" % fn)
        sys.exit(-1)

    atexit.register(os.unlink, fn)
    return fn

CURRENT_PROC = []
def launch_monitoring(args):
    fifo = create_fifo()
    mon_fd = os.open(fifo, os.O_RDONLY | os.O_NONBLOCK)

    def preexec():
        # this should only be open on a parent
        os.close(mon_fd)

        # open fifo once (hack)
        # so there is *always* at least one writter
        # which closes with the executable
        os.open(fifo, os.O_WRONLY)

        try:
            # ensure the child dies if we are SIGKILLED
            import ctypes
            libc = ctypes.CDLL("libc.so.6")
            PR_SET_PDEATHSIG = 1
            libc.prctl(PR_SET_PDEATHSIG, signal.SIGKILL)
        except:
            log("Failed to setup PR_SET_PDEATHSIG.")
            pass

        env = os.environ
        env["DQMMON_UPDATE_PIPE"] = fifo

    p = subprocess.Popen(args.pargs, preexec_fn=preexec, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    CURRENT_PROC.append(p)

    mon_file = os.fdopen(mon_fd)

    manager = FDManager()
    es = ElasticReport(pid=p.pid, args=args)

    log_handler = FDHistory(manager, p.stdout.fileno(), es=es)
    json_handler = FDJsonInput(manager, mon_fd, es)

    log_handler.handle_line("-- starting process: %s --\n" % str(args.pargs))
    try:
        manager.event_loop(timeout=5)
    except select.error, e:
        # we have this on ctrl+c
        # just terminate the child
        log("Select error (we will terminate): " + str(e))
        p.terminate()

    # at this point the program is dead
    r =  p.wait()
    log_handler.handle_line("\n-- process exit: %s --\n" % str(r))

    es.update_doc({ "exit_code": r })
    es.make_report()

    CURRENT_PROC.remove(p)
    return r

def handle_signal(signum, frame):
    for proc in CURRENT_PROC:
        proc.send_signal(signum)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Monitor a child process and produce es documents.")
    parser.add_argument('--debug', '-d', action='store_true', help="Debug mode")
    parser.add_argument('pargs', nargs=argparse.REMAINDER)
    args = parser.parse_args()

    if not args.pargs:
        parser.print_help()
        sys.exit(-1)
    elif args.pargs[0] == "--":
        # compat with 2.6
        args.pargs = args.pargs[1:]

    # do some signal magic
    signal.signal(signal.SIGINT, handle_signal)
    signal.signal(signal.SIGTERM, handle_signal)

    sys.exit(launch_monitoring(args))
