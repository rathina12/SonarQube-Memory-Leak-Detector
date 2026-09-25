#include <stdlib.h>
void safe(void) { int *p = (int*)malloc(sizeof(int)); *p = 7; free(p); }
int main(void){ safe(); return 0; }
