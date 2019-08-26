#!/usr/bin/env python3
from pypanda import Panda
from time import sleep
import pdb

arch = "i386"

print("\n\nFIRST TRY")
panda = Panda(generic=arch, extra_args = "")
print("Initialize")
panda.init()
print("Initialized")
panda.unload()
print("Unloaded")
pdb.set_trace()

print("\n\nSECOND TRY")
panda2 = Panda(generic=arch, extra_args = "")
print("Initialize")
panda2.init()
print("Initialized")
panda2.unload()
print("Unloaded")

"""
print("\nINIT PANDA OBJ")
with Panda(generic=arch, extra_args = "") as panda:
    pdb.set_trace()
    print("Have panda")
    [print('.') and sleep(1) for _ in range(10)]

print("\n\n--------------------------\n")
pdb.set_trace()

with Panda(generic=arch, extra_args = "") as panda2:
    print("SECOND PANDA")
    [print('.') and sleep(1) for _ in range(10)]
"""

print("Done")
