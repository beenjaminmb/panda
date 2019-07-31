#include <stdio.h>

int foo(int x) {
  return x + 1;
}

static void con() __attribute__((constructor));
void con() {
      puts("MYLIB CONSTRUCTOR\t");
}

static void des() __attribute__((destructor));
void des() {
      puts("MYLIB DESTRUCTOR\t");
}
