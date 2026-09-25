#include <stdlib.h>
void grow(void){ int *p=(int*)malloc(8); p=(int*)realloc(p,64); free(p); }
