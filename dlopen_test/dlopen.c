	#include <stdlib.h>
	#include <stdio.h>
	#include <dlfcn.h>

	int main(int argc, char **argv) {
			void *handle;
			int (*foo)(int);
			char *error;

      puts("Opening...\t");
			handle = dlopen ("/home/andrew/git/panda/dlopen/mylib.so", RTLD_LAZY);
			if (!handle) {
					fputs (dlerror(), stderr);
					exit(1);
			}
      puts("Opened\n");

      // Get handle to foo
			foo = dlsym(handle, "foo");
			if ((error = dlerror()) != NULL)  {
					fputs(error, stderr);
					exit(1);
			}
      puts("got foo handle\n");

      puts("Calling foo...\t");
			printf ("%d\t", (*foo)(1));
      puts("Done with foo\n");

      puts("Closing...\t");
			dlclose(handle);
      puts("Closed\n");
	}
