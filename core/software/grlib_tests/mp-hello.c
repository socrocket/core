#include "stdlib.h"
#include "stdio.h"

int get_cpu_id() {
    unsigned int result = 0;
    asm(
        "rd  %%asr17, %[result]"
        : [result] "=r" (result)
    );
    return result >> 28;
}


int main() {
	  printf("Hello World %d\n", get_cpu_id());
    fflush(stdout);
    return 0;
}

