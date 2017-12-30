#include <stdio.h>
int main()
{
	int i = 0;
	scanf("%d\n", &i);
	if (i == 120) { printf("OK\n"); return 0; }
	else { printf("FAIL\n"); return 1;};
}
