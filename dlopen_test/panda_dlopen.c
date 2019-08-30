#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <assert.h>
#include <unistd.h>


#define PANDA "/home/andrew/git/panda/build/i386-softmmu/libpanda-i386.so"
# define N_ARGS 10
char *pargv[N_ARGS] = { \
  "/home/andrew/git/panda/build/i386-softmmu/qemu-system-i386", \
  "-m", "128M", \
  "-display", "none", \
  "-L", "/home/andrew/git/panda/build/pc-bios", \
  "-os", "linux-32-debian:3.2.0-4-686-pae", \
  "/home/andrew/.panda/wheezy_panda2.qcow2"};


void cleanup(void* handle) {
  void (*fn)(void);

  fn = dlsym(handle, "panda_stop_all_threads");
  if (!fn) {fputs (dlerror(), stderr); exit(1);}
  fn();

  fn = dlsym(handle, "kill_rcu_thread");
  if (!fn) {fputs (dlerror(), stderr); exit(1);}
  fn();

  fn = dlsym(handle, "kill_tcg_thread");
  if (!fn) {fputs (dlerror(), stderr); exit(1);}
  fn();
}

int main(int argc, char **argv) {
		void *handle1;
		void *handle2;
		char *error;
		void (*set_os_name)(char*);
		void (*panda_init)(int argc, char** argv, char** envp);

    handle1 = dlopen (PANDA, RTLD_NOW | RTLD_LOCAL); 
    if (!handle1) {fputs (dlerror(), stderr); exit(1);}

    // Setup library: os_name + panda_init with args
    set_os_name = dlsym(handle1, "panda_set_os_name");
    if (!set_os_name) {fputs (dlerror(), stderr); exit(1);}
    set_os_name("linux-32-debian:3.2.0-4-686-pae");

    panda_init = dlsym(handle1, "panda_init");
    if (!panda_init) {fputs (dlerror(), stderr); exit(1);}

    panda_init(N_ARGS, pargv, NULL);

    cleanup(handle1);

    dlclose(handle1);

    printf("\nSECOND INSTANCE\n\n");
    sleep(5);
    printf("\n LOAD NEW\n");

    // Now open NEW panda instance
    handle2 = dlopen (PANDA, RTLD_NOW | RTLD_LOCAL); 
    if (!handle2) {fputs (dlerror(), stderr); exit(1);}

    // Setup library: os_name + panda_init with args
    set_os_name = dlsym(handle2, "panda_set_os_name");
    if (!set_os_name) {fputs (dlerror(), stderr); exit(1);}
    set_os_name("linux-32-debian:3.2.0-4-686-pae");

    panda_init = dlsym(handle2, "panda_init");
    if (!panda_init) {fputs (dlerror(), stderr); exit(1);}

    panda_init(N_ARGS, pargv, NULL);

    dlclose(handle2);
}
