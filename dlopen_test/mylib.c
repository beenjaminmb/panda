#include <stdio.h>
#include <assert.h>

static int init_count = 0;

void init() {
  assert(init_count++ == 0); // Can't initialize same instance more than once
  printf("INIT OK\n");
}

static void con() __attribute__((constructor));
void con() {
      //puts("MYLIB CONSTRUCTOR\t");
}

static void des() __attribute__((destructor));
void des() {
      //puts("MYLIB DESTRUCTOR\t");
}
