#include <stdlib.h>
void double_free(void) { int *p=(int*)malloc(16); free(p); free(p); }
