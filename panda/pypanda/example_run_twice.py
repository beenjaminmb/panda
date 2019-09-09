#!/usr/bin/env python3
# Run a command, stop the guest. Then run another command with a plugin and stop the guest

from pypanda import Panda, blocking

@blocking
def first_cmd(): # Run a command at the root snapshot, then end .run()
    panda.revert_sync("root")
    print('WHOAMI:', panda.run_serial_cmd("whoami"))
    panda.stop_run()

@blocking
def second_cmd(): # Run a command at the root snapshot, then end .run()
    panda.revert_sync("root")
    print("UNAME:", panda.run_serial_cmd("uname -a"))
    panda.stop_run() # Finish panda.run

panda = Panda(generic='i386')

print("======= Run first =========")
panda.queue_async(first_cmd)
panda.run()
print("======= Finish first =========")

# Now enable a callback for 2nd run
blocks = set()
@panda.cb_before_block_exec(name="foo")
def before_block_exec(env, tb):
    global blocks
    pc = panda.current_pc(env)
    blocks.add(pc)
    return 0


print("======= Run second =========")
panda.queue_async(second_cmd) # Run it
panda.run()
print("======= Finish second =========\n Saw {} distinct basic blocks".format(len(blocks)))
