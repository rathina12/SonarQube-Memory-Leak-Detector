#include <stdlib.h>
int* make_value(void){ int *p=(int*)malloc(sizeof(int)); return p; }
void consume(void){ int *p=make_value(); free(p); }
