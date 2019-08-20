#!/usr/bin/env python3
from pypanda import *
from sys import argv
import subprocess
import os

# Single arg of arch, defaults to i386
arch = "i386" if len(argv) <= 1 else argv[1]

print("Initialize a panda")
panda = Panda(generic=arch, extra_args = "")

# Record, then replay with plugins
@blocking
def runcmd():
    print("RUNCMD")
    panda.revert_sync("root")
    print("CMD 1:", panda.run_serial_cmd("uname -a"))
    panda.stop_run()

@blocking
def quit():
    panda.run_monitor_cmd("quit")

print("Start")
panda.queue_async(runcmd)
panda.queue_async(quit)
print("Queued... Starting run")
panda.run()
print("Finished run")

print("DO UNLOAD")
panda.unload()

import time
print("Sleeping 5s")
for _ in range(5):
    time.sleep(1)
    print(".", end="")

print("Now done")

@blocking
def runcmd2():
    print("RUNCMD2")
    panda.revert_sync("root")
    print("CMD 2:", panda.run_serial_cmd("ls /"))
    panda.stop_run()

print("Queue more")
panda.queue_async(runcmd2)
panda.queue_async(quit)
print("RUN AGAIN")
panda.run()
print("\n\nFINISHED\n")
