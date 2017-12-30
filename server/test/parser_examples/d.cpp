#include <stdio.h>
int main()
{
	int i = 0;
	scanf("%d\n", &i);
	if (i == 120) { fprintf(stderr, "OK\n"); return 0; }
	else { fprintf(stderr, "FAIL\n"); return 1;};
}
