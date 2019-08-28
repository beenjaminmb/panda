import re

# Autogenerate panda_datatypes.py and include/panda_datatypes.h
#
# Both of these files contain info in or derived from stuff in
# panda/include/panda.  Here, we autogenerate the two files so that we
# never have to worry about how to keep them in sync with the info in
# those include files.  See panda/include/panda/README.pypanda for 
# so proscriptions wrt those headers we use here. They need to be kept
# fairly clean if we are to be able to make sense of them with this script
# which isn't terriby clever.
#

# First, create panda_datatypes.py from panda/include/panda/panda_callback_list.h
#

with open("panda_datatypes.py", "w") as pdty:

    pdty.write("""
# NOTE: panda_datatypes.py is auto generated by the script create_panda_datatypes.py
# Please do not tinker with it!  Instead, fix the script that generates it
""")

    pdty.write("""
from enum import Enum
from ctypes import *
from collections import namedtuple
from cffi import FFI

ffi = FFI()
pyp = ffi

# XXX  DEBUGGING
ffi.cdef("void kill_rcu_thread();")

ffi.cdef("typedef uint32_t target_ulong;")
ffi.cdef(open("include/pthreadtypes.h").read())
ffi.cdef(open("include/panda_x86_support.h").read())
ffi.cdef(open("include/panda_qemu_support.h").read())
ffi.cdef(open("include/panda_datatypes.h").read())
ffi.cdef(open("include/panda_osi.h").read())
ffi.cdef(open("include/panda_osi_linux.h").read())

# so we need access to some data structures, but don't actually
# want to open all of libpanda yet because we don't have all the
# file information. So we just open libc to access this.
C = ffi.dlopen(None)

class PandaState(Enum):
    UNINT = 1
    INIT_DONE = 2
    IN_RECORD = 3
    IN_REPLAY = 4
""")

    cbn = 0
    cb_list = {}
    with open ("../../include/panda/panda_callback_list.h") as fp:
        for line in fp:
            foo = re.search("^(\s+)PANDA_CB_([^,]+)\,", line)
            if foo:
                cbname = foo.groups()[1]
                cbname_l = cbname.lower()
                cb_list[cbn] = cbname_l
                cbn += 1
    cb_list[cbn] = "last"
    cbn += 1


    pdty.write('\nPandaCB = namedtuple("PandaCB", "init \\\n')
    for i in range(cbn-1):
        pdty.write(cb_list[i] + " ")
        if i == cbn-2:
            pdty.write('")\n')
        else:
            pdty.write("\\\n")

    in_tdu = False
    cb_types = {}
    with open ("../../include/panda/panda_callback_list.h") as fp:
        for line in fp:
            foo = re.search("typedef union panda_cb {", line)
            if foo:
                in_tdu = True
                continue
            foo = re.search("} panda_cb;", line)
            if foo:
                in_tdu = False
                continue
            if in_tdu:
                # int (*before_block_translate)(CPUState *env, target_ulong pc);
                for i in range(cbn):
                    foo = re.search("^\s+(.*)\s+\(\*%s\)\((.*)\);" % cb_list[i], line)
                    if foo:
                        rvtype = foo.groups()[0]
                        params = foo.groups()[1]
                        partypes = []
                        for param in (params.split(',')):
                            j = 1
                            while True:
                                c = param[-j]
                                if not (c.isalpha() or c.isnumeric() or c=='_'):
                                    break
                                if j == len(param):
                                    break
                                j += 1
                            if j == len(param):
                                typ = param
                            else:
                                typ = param[:(-j)+1].strip()
                            partypes.append(typ)
                        cb_typ = rvtype + " (" +  (", ".join(partypes)) + ")"
                        cb_name = cb_list[i]
                        cb_types[i] = (cb_name, cb_typ)

    # Sanity check: input files must match
    for i in range(cbn-1):
        if i not in cb_types:
            print(cb_types[i-1])
            raise RuntimeError("Callback #{} missing from cb_types- is it in panda_callback_list.h's panda_cb_type enum AND as a prototype later in the source?".format(i))

    pdty.write("""

pcb = PandaCB(init = pyp.callback("bool(void*)"), 
""")

    for i in range(cbn-1):
        pdty.write('%s = pyp.callback("%s")' % cb_types[i])
        if i == cbn-2:
            pdty.write(")\n")
        else:
            pdty.write(",\n")

    pdty.write("""

pandacbtype = namedtuple("pandacbtype", "name number")

""")


    pdty.write("""

callback_dictionary = {
pcb.init : pandacbtype("init", -1),
""")


    for i in range(cbn-1):
        cb_name = cb_list[i]
        cb_name_up = cb_name.upper()
        pdty.write('pcb.%s : pandacbtype("%s", C.PANDA_CB_%s)' % (cb_name, cb_name, cb_name_up))
        if i == cbn-2:
            pdty.write("}\n")
        else:
            pdty.write(",\n")


#########################################################
#########################################################
# second, create panda_datatypes.h by glomming together
# files in panda/include/panda

with open("include/panda_datatypes.h", "w") as pdth:

    pdth.write("""
// NOTE: panda_datatypes.h is auto generated by the script create_panda_datatypes.py
// Please do not tinker with it!  Instead, fix the script that generates it

#define PYPANDA 1

""")

    def read_but_exclude_garbage(filename):
        nongarbage = []
        with open(filename) as thefile:
            for line in thefile:
                keep = True
                foo = re.search("^\s*\#", line)
                if foo:
                    foo = re.search("^\s*\#define [^_]", line)
                    if not foo:
                        keep = False
                if keep:
                    nongarbage.append(line)
            return nongarbage
    pn = 1
    def include_this(fn):
        global pn
        pdth.write("\n\n// -----------------------------------\n")
        pdth.write("// Pull number %d from %s\n" % (pn,fn))
        for line in read_but_exclude_garbage(fn):
            pdth.write(line)
        pn += 1
    include_this("../../include/panda/panda_callback_list.h")
    include_this("../../include/panda/panda_plugin_mgmt.h")
    include_this("../../include/panda/panda_args.h")
    include_this("../../include/panda/panda_api.h")
    include_this("../../include/panda/panda_os.h")
    # probably a better way... 
    pdth.write("typedef target_ulong target_ptr_t;\n")
    include_this("../../include/panda/panda_common.h")



    
