#include <stdio.h>
int main()
{
	printf("HELLO\n");
	int i = 0;
	scanf("%d", &i);
	if (i == 120) { printf("OK\n"); return 0; }
	else { printf("FAIL\n"); return 1;};
}
