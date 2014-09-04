#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int main(int argc, char *argv[]) {
  int i = 0, j = 1;
  for(i = 0; i < argc; i++) {
      printf("%d: %s\n", i, argv[i]);
      switch(i) {
          case 1:
            j += (strcmp("one", argv[i])!=0) ? 0 : 1;
            break;
          case 2:
            j += (strcmp("two", argv[i])!=0) ? 0 : 1;
            break;
          case 3:
            j += (strcmp("three", argv[i])!=0) ? 0 : 1;
            break;
          case 4:
            j += (strcmp("four", argv[i])!=0) ? 0 : 1;
            break;
          default:
            break;
      }
  }
  return !(j==argc && argc == 5);
}
