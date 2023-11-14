#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"cpbuild.h"
int main(int argl, char *argv[])
{
	struct cpbuild_options defops;
	memset(&defops, 0, sizeof defops);
	fill_default_options(&defops);
	int succ = cpbuild(argv + 1, &defops);
	return succ;
}
