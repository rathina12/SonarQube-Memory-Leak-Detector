#include <stdlib.h>
void uaf(void) { int *p=(int*)malloc(16); free(p); *p=5; }
