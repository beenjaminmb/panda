#
# QAPI command marshaller generator
#
# Copyright IBM, Corp. 2011
# Copyright (C) 2014-2015 Red Hat, Inc.
#
# Authors:
#  Anthony Liguori <aliguori@us.ibm.com>
#  Michael Roth    <mdroth@linux.vnet.ibm.com>
#  Markus Armbruster <armbru@redhat.com>
#
# This work is licensed under the terms of the GNU GPL, version 2.
# See the COPYING file in the top-level directory.

from qapi import *
import re


def gen_command_decl(name, arg_type, ret_type):
    argstr = ''
    if arg_type:
        for memb in arg_type.members:
            if memb.optional:
                argstr += 'bool has_%s, ' % c_name(memb.name)
            argstr += '%s %s, ' % (memb.type.c_type(is_param=True),
                                   c_name(memb.name))
    return mcgen('''
%(c_type)s qmp_%(c_name)s(%(args)sError **errp);
''',
                 c_type=(ret_type and ret_type.c_type()) or 'void',
                 c_name=c_name(name),
                 args=argstr)


def gen_err_check(err):
    if not err:
        return ''
    return mcgen('''
if (%(err)s) {
    goto out;
}
''',
                 err=err)


def gen_call(name, arg_type, ret_type):
    ret = ''

    argstr = ''
    if arg_type:
        for memb in arg_type.members:
            if memb.optional:
                argstr += 'has_%s, ' % c_name(memb.name)
            argstr += '%s, ' % c_name(memb.name)

    lhs = ''
    if ret_type:
        lhs = 'retval = '

    push_indent()
    ret = mcgen('''

%(lhs)sqmp_%(c_name)s(%(args)s&local_err);
''',
                c_name=c_name(name), args=argstr, lhs=lhs)
    if ret_type:
        ret += gen_err_check('local_err')
        ret += mcgen('''

qmp_marshal_output_%(c_name)s(retval, ret, &local_err);
''',
                     c_name=c_name(name))
    pop_indent()
    return ret


def gen_marshal_vars(arg_type, ret_type):
    ret = mcgen('''
    Error *local_err = NULL;
''')

    push_indent()

    if ret_type:
        ret += mcgen('''
%(c_type)s retval;
''',
                     c_type=ret_type.c_type())

    if arg_type:
        ret += mcgen('''
QmpInputVisitor *mi = qmp_input_visitor_new_strict(QOBJECT(args));
QapiDeallocVisitor *md;
Visitor *v;
''')

        for memb in arg_type.members:
            if memb.optional:
                ret += mcgen('''
bool has_%(c_name)s = false;
''',
                             c_name=c_name(memb.name))
            ret += mcgen('''
%(c_type)s %(c_name)s = %(c_null)s;
''',
                         c_name=c_name(memb.name),
                         c_type=memb.type.c_type(),
                         c_null=memb.type.c_null())
        ret += '\n'
    else:
        ret += mcgen('''

(void)args;
''')

    pop_indent()
    return ret


def gen_marshal_input_visit(arg_type, dealloc=False):
    ret = ''

    if not arg_type:
        return ret

    push_indent()

    if dealloc:
        errparg = 'NULL'
        errarg = None
        ret += mcgen('''
qmp_input_visitor_cleanup(mi);
md = qapi_dealloc_visitor_new();
v = qapi_dealloc_get_visitor(md);
''')
    else:
        errparg = '&local_err'
        errarg = 'local_err'
        ret += mcgen('''
v = qmp_input_get_visitor(mi);
''')

    for memb in arg_type.members:
        if memb.optional:
            ret += mcgen('''
visit_optional(v, &has_%(c_name)s, "%(name)s", %(errp)s);
''',
                         c_name=c_name(memb.name), name=memb.name,
                         errp=errparg)
            ret += gen_err_check(errarg)
            ret += mcgen('''
if (has_%(c_name)s) {
''',
                         c_name=c_name(memb.name))
            push_indent()
        ret += mcgen('''
visit_type_%(c_type)s(v, &%(c_name)s, "%(name)s", %(errp)s);
''',
                     c_name=c_name(memb.name), name=memb.name,
                     c_type=memb.type.c_name(), errp=errparg)
        ret += gen_err_check(errarg)
        if memb.optional:
            pop_indent()
            ret += mcgen('''
}
''')

    if dealloc:
        ret += mcgen('''
qapi_dealloc_visitor_cleanup(md);
''')
    pop_indent()
    return ret


def gen_marshal_output(name, ret_type):
    return mcgen('''

static void qmp_marshal_output_%(c_cmd_name)s(%(c_type)s ret_in, QObject **ret_out, Error **errp)
{
    Error *local_err = NULL;
    QmpOutputVisitor *mo = qmp_output_visitor_new();
    QapiDeallocVisitor *md;
    Visitor *v;

    v = qmp_output_get_visitor(mo);
    visit_type_%(c_name)s(v, &ret_in, "unused", &local_err);
    if (local_err) {
        goto out;
    }
    *ret_out = qmp_output_get_qobject(mo);

out:
    error_propagate(errp, local_err);
    qmp_output_visitor_cleanup(mo);
    md = qapi_dealloc_visitor_new();
    v = qapi_dealloc_get_visitor(md);
    visit_type_%(c_name)s(v, &ret_in, "unused", NULL);
    qapi_dealloc_visitor_cleanup(md);
}
''',
                 c_type=ret_type.c_type(), c_cmd_name=c_name(name),
                 c_name=ret_type.c_name())


def gen_marshal_proto(name):
    ret = 'void qmp_marshal_input_%s(QDict *args, QObject **ret, Error **errp)' % c_name(name)
    if not middle_mode:
        ret = 'static ' + ret
    return ret


def gen_marshal_decl(name):
    return mcgen('''
%(proto)s;
''',
                 proto=gen_marshal_proto(name))


def gen_marshal(name, arg_type, ret_type):
    ret = mcgen('''

%(proto)s
{
''',
                proto=gen_marshal_proto(name))

    ret += gen_marshal_vars(arg_type, ret_type)
    ret += gen_marshal_input_visit(arg_type)
    ret += gen_call(name, arg_type, ret_type)

    if re.search('^ *goto out;', ret, re.MULTILINE):
        ret += mcgen('''

out:
''')
    ret += mcgen('''
    error_propagate(errp, local_err);
''')
    ret += gen_marshal_input_visit(arg_type, dealloc=True)
    ret += mcgen('''
}
''')
    return ret


def gen_register_command(name, success_response):
    push_indent()
    options = 'QCO_NO_OPTIONS'
    if not success_response:
        options = 'QCO_NO_SUCCESS_RESP'

    ret = mcgen('''
qmp_register_command("%(name)s", qmp_marshal_input_%(c_name)s, %(opts)s);
''',
                name=name, c_name=c_name(name),
                opts=options)
    pop_indent()
    return ret


def gen_registry(registry):
    ret = mcgen('''

static void qmp_init_marshal(void)
{
''')
    ret += registry
    ret += mcgen('''
}

qapi_init(qmp_init_marshal);
''')
    return ret


class QAPISchemaGenCommandVisitor(QAPISchemaVisitor):
    def __init__(self):
        self.decl = None
        self.defn = None
        self._regy = None

    def visit_begin(self, schema):
        self.decl = ''
        self.defn = ''
        self._regy = ''

    def visit_end(self):
        if not middle_mode:
            self.defn += gen_registry(self._regy)
        self._regy = None

    def visit_command(self, name, info, arg_type, ret_type,
                      gen, success_response):
        if not gen:
            return
        self.decl += gen_command_decl(name, arg_type, ret_type)
        if ret_type:
            self.defn += gen_marshal_output(name, ret_type)
        if middle_mode:
            self.decl += gen_marshal_decl(name)
        self.defn += gen_marshal(name, arg_type, ret_type)
        if not middle_mode:
            self._regy += gen_register_command(name, success_response)


middle_mode = False

(input_file, output_dir, do_c, do_h, prefix, opts) = \
    parse_command_line("m", ["middle"])

for o, a in opts:
    if o in ("-m", "--middle"):
        middle_mode = True

c_comment = '''
/*
 * schema-defined QMP->QAPI command dispatch
 *
 * Copyright IBM, Corp. 2011
 *
 * Authors:
 *  Anthony Liguori   <aliguori@us.ibm.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.1 or later.
 * See the COPYING.LIB file in the top-level directory.
 *
 */
'''
h_comment = '''
/*
 * schema-defined QAPI function prototypes
 *
 * Copyright IBM, Corp. 2011
 *
 * Authors:
 *  Anthony Liguori   <aliguori@us.ibm.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.1 or later.
 * See the COPYING.LIB file in the top-level directory.
 *
 */
'''

(fdef, fdecl) = open_output(output_dir, do_c, do_h, prefix,
                            'qmp-marshal.c', 'qmp-commands.h',
                            c_comment, h_comment)

fdef.write(mcgen('''
#include "qemu-common.h"
#include "qemu/module.h"
#include "qapi/qmp/types.h"
#include "qapi/qmp/dispatch.h"
#include "qapi/visitor.h"
#include "qapi/qmp-output-visitor.h"
#include "qapi/qmp-input-visitor.h"
#include "qapi/dealloc-visitor.h"
#include "%(prefix)sqapi-types.h"
#include "%(prefix)sqapi-visit.h"
#include "%(prefix)sqmp-commands.h"

''',
                 prefix=prefix))

fdecl.write(mcgen('''
#include "%(prefix)sqapi-types.h"
#include "qapi/qmp/qdict.h"
#include "qapi/error.h"

''',
                  prefix=prefix))

schema = QAPISchema(input_file)
gen = QAPISchemaGenCommandVisitor()
schema.visit(gen)
fdef.write(gen.defn)
fdecl.write(gen.decl)

close_output(fdef, fdecl)
