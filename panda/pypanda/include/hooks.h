// Hook functions must be of this type
typedef bool (*hook_func_t)(CPUState *, TranslationBlock *);

// XXX the extern is a single line (no curly braces) so it can be programatically removed
// when we use CFFI- see create_panda_datatypes.py for the replacement logic
extern "C" void add_hook(target_ulong addr, hook_func_t hook);
