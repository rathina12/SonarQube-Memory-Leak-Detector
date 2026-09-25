#include <stdlib.h>
void branch(int fail) { int *p=(int*)malloc(32); if(fail){ return; } free(p); }
