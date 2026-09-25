#include <stdlib.h>
void overwrite(void) { int *p=(int*)malloc(16); p=(int*)malloc(32); free(p); }
