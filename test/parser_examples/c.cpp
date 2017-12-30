#include <stdio.h>
#include <string.h>
int main()
{
	int i = 0;
	char c[100] = {0};
	scanf("%s\n",c);
	printf("C - Read %s\n", c);
	if (strcmp(c, "OK") == 0)
	{
		printf("C - OK\n"); return 0;
	}
	else
	{
		printf("C - FAIL\n"); return 1;
	}
}
