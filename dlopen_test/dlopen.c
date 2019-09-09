#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <assert.h>

	int main(int argc, char **argv) {
			void *handle1;
			void *handle2;
			void (*init1)(void);
			void (*init2)(void);
			char *error;
      int* foo;

      handle1 = dlopen ("/home/andrew/git/panda/dlopen_test/mylib.so", RTLD_NOW | RTLD_LOCAL); 
      if (!handle1) {fputs (dlerror(), stderr); exit(1);}
      init1 = dlsym(handle1, "init");
      foo = dlsym(handle1, "init_count");
      printf("Init1 is at %p, init_count at %p\n", &init1, &foo);
      init1();
      dlclose(handle1);

      handle2 = dlopen ("/home/andrew/git/panda/dlopen_test/mylib.so", RTLD_NOW | RTLD_LOCAL); 
      if (!handle2) {fputs (dlerror(), stderr); exit(1);}
      init2 = dlsym(handle2, "init");
      printf("Init2 is at %p, init_count at %p\n", &init2, &foo);
      init2();
      dlclose(handle2);
	}
