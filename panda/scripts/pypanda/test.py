#!/usr/bin/env python3
from pypanda import Panda, blocking
from time import sleep
import pdb

#pdb.set_trace()

arch = "i386"

@blocking
def quit():
    print("Stop")
    panda.stop_run()

# Single python thread exists prior to here
panda = Panda(generic=arch, extra_args = "")
pdb.set_trace()

panda.init()
panda.queue_async(quit)
panda.run()

print("UNLOAD")
panda.unload()
print("UNLOADED")


# Now lets do it again
print("\n\n AGAIN\n\n")
panda.init()
panda.queue_async(quit)
panda.run()
panda.unload()
