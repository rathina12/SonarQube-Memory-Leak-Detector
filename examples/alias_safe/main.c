#include <stdlib.h>
void alias_safe(void) { int *p=(int*)malloc(32); int *q=p; free(q); }
