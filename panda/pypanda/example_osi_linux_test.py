#!/usr/bin/env python3
'''
example_osi_linux_test.py

This example shows off the functionality of osi_linux in pypanda. Modeled after
the original osi_linux panda plugin. 

Runs with: python3 example_osi_linux_test.py i386 /path/to/recording
'''
from pypanda import *
from sys import argv

#  arg of arch, defaults to i386
arch = "i386" if len(argv) <= 2 else argv[1]
panda = Panda(generic=arch)

recording = argv[-1]

assert(len(argv) >= 2), f"USAGE: {argv[0]}, [arch] recording_name"

print(f"Running with arch {arch}, recording {recording}")


def process_name_convert(name):
	if name == ffi.NULL:
		return ""
	a = ""
	for i in range(16):
		char = name[i].decode()
		if ord(char) == 0:
			break
		a += char
	return a	

@panda.callback.init
def init(handle):
    # Register a python before-BB callback
	panda.register_callback(handle, panda.callback.asid_changed, asid_changed)
#	panda.register_callback(handle, panda.callback.before_block_exec,\
#							before_block_exec)
#	panda.register_callback(handle, panda.callback.after_block_exec, \
#							after_block_exec)
	return True

@panda.callback.before_block_exec
def before_block_exec(cpustate, transblock):
	current = panda.get_current_process(cpustate)
	if current != ffi.NULL:
		progress("Current process: %s PID: %d PPD: %d" \
			%(process_name_convert(current.name), current.pid, current.ppid))
	ps = panda.get_processes(cpustate)
	if current != ffi.NULL:
		for i in range(ps.num):
			name = process_name_convert(ps.proc[i].name)
			progress(" %s\t %d\t %d" % (name, ps.proc[i].pid, ps.proc[i].ppid))
	return 0

@panda.callback.after_block_exec
def after_block_exec(cpustate, transblock):
	current = panda.get_current_process(cpustate)
	ms = panda.get_libraries(cpustate,current)
	if ms == ffi.NULL:
		progress("No mapped dynamic libraries")
	else:
		progress("Dynamic libraries list (%d libs):" % ms.num)
		for i in range(ms.num):
			mod = ms.module[i]
			name = process_name_convert(mod.name)
			file_name = process_name_convert(mod.file)
			progress("\t 0x%s \t 0x%s \t %s %s" \
					%(hex(mod.base),hex(mod.size),name,file_name))
	
	kms = panda.get_modules(cpustate)
	if kms == ffi.NULL:
		progress("No mapped kernel modules")
	else:
		progress("Kernel module list (%d modules):" % kms.num)
		for i in range(kms.num):
			mod = kms.module[i]
			name = process_name_convert(mod.name)
			file_name = process_name_convert(mod.file)
			progress("\t 0x%s \t 0x%s \t %s %s" \
					%(hex(mod.base),hex(mod.size),name,file_name))
	return 0


@panda.callback.asid_changed
def asid_changed(cpustate, old_asid, new_asid):
	tb = ffi.cast("TranslationBlock*", ffi.NULL)
	before_block_exec(cpustate, tb)
	after_block_exec(cpustate, tb)
	return 0

# Register a python plugin, the init function above
panda.load_python_plugin(init,"on-init")

# Register a c plugin, coverage
panda.begin_replay(recording)
# Start running
panda.run()
