#include <stdio.h>

static int x = 0;
int y = 0;

int inc_x(int off) {
    x+=off;
    return x;
}

int inc_y(int off) {
  y+=off;
  return y + 1;
}

static void con() __attribute__((constructor));
void con() {
      puts("MYLIB CONSTRUCTOR\t");
}

static void des() __attribute__((destructor));
void des() {
      puts("MYLIB DESTRUCTOR\t");
}
