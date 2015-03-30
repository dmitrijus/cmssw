#!/usr/bin/env python

import argparse
import subprocess
import select, signal, time
import sys, os
import zlib

def log(s):
    sys.stderr.write("m: " + s + "\n");
    sys.stderr.flush()

class GZipLog(object):
    def __init__(self, log_file):
        self.file = None

        self.last_flush = time.time()
        self.zstream = zlib.compressobj(6, zlib.DEFLATED, zlib.MAX_WBITS | 16)

        # 'gzip' block will be flushed if we reach max bytes or the timeout
        self.block = []
        self.block_size = 0
        self.block_size_max = 2*1024*1024 # bytes
        self.block_timeout = 15 # seconds

        self.file = open(log_file, "wb+")
        self.file_min_size = 2*1024*1024    # this amount of space we keep after truncating
        self.file_max_size = 64*1024*1024   # we truncate if file gets bigger

        self.file_truncate_pos = None
        self.file_truncate_state = None

    def write_block(self, data):
        self.file.write(self.zstream.compress(data))
        self.file.write(self.zstream.flush(zlib.Z_FULL_FLUSH))

    def flush_block(self):
        data, size, = "".join(self.block), self.block_size
        self.block, self.block_size = [], 0

        if size == 0:
            # nothing to do
            return

        # check if we need to truncate the file
        # 'size' refers to uncompressed data, so the file be truncated at max_size -+ max_block_size
        # there is no way know compressed size without collapsing the zlib state
        if (self.file.tell() + size) > self.file_max_size:
            if self.file_truncate_pos is not None:
                # tell it to cleanup
                self.zstream.flush(zlib.Z_FINISH)

                self.file.seek(self.file_truncate_pos, 0)
                self.file.truncate()
                self.zstream = self.file_truncate_state.copy()

                self.write_block("\n\n--- file was cut at this point ---\n\n")

        # write it
        self.write_block(data)

        # check if we can save the ptr into truncate_position
        if self.file_truncate_pos is None:
            if self.file.tell() >= self.file_min_size:
                self.file_truncate_pos = self.file.tell()
                self.file_truncate_state = self.zstream.copy()

    def finish(self):
        self.flush_block()
        self.file.write(self.zstream.flush(zlib.Z_FINISH))
        self.file.close()
        self.file = None

    def try_flush(self):
        now = time.time()

        timeout = (now - self.last_flush) > self.block_timeout
        large_block = self.block_size > self.block_size_max

        if timeout or large_block:
            self.last_flush = now
            self.flush_block()

    def write(self, bytes):
        if bytes:
            self.block.append(bytes)
            self.block_size += len(bytes)

        self.try_flush()

    def handle_timeout(self):
        self.try_flush()


def capture(fd, args):
    log_handler = GZipLog(log_file=args.log)

    try:
        while True:
            rlist, wlist, xlist = select.select([fd], [], [], 5)
            if not rlist:
                log_handler.handle_timeout()
                continue

            bytes = os.read(fd, 4096)
            if (bytes == ''):
                break

            log_handler.write(bytes)
            if not args.q:
                sys.stdout.write(bytes)
    finally:
        log_handler.finish()

def handle_signal(signum, frame):
    for proc in CURRENT_PROC:
        proc.send_signal(signum)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Take all input and write a compressed log file.")
    parser.add_argument('-q', action='store_true', help="Don't write to stdout, just the log file.")
    parser.add_argument("log", type=str, help="Filename to write.", metavar="<logfile.gz>")
    args = parser.parse_args()

    if args.q:
        fd = sys.stdout.fileno()
        sys.stdout.close()
        os.close(fd)

    capture(sys.stdin.fileno(), args)
