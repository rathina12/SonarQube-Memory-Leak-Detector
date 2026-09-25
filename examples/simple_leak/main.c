#include <stdlib.h>
void leak(void) { int *p = (int*)malloc(sizeof(int)); *p = 7; }
int main(void){ leak(); return 0; }
