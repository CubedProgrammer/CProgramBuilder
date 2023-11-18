#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"cpbuild.h"
#include"utils.h"
int main(int argl, char *argv[])
{
	struct cpbuild_options defops;
	int argstart = argl;
	unsigned nxtarg = 0, nxtcnt = 0;
	char*arg,*artifact=NULL;
	char thisdir[]=".";
	char *thisdirp[] = {thisdir, NULL};
	memset(&defops, 0, sizeof defops);
	for(int i = 1; i < argstart; ++i)
	{
		arg = argv[i];
		if(nxtcnt == 0)
			nxtarg = 0;
		switch(nxtarg)
		{
			case 1:
				artifact=defops.artifact=arg;
				--nxtcnt;
				break;
			default:
				if(arg[0] == '-')
				{
					++arg;
					switch(*arg)
					{
						case'f':
							defops.boolops |= BOOLOPS_FORCE;
							break;
						case'A':
							nxtarg = 1;
							nxtcnt = 1;
							break;
						default:
							fprintf(stderr, "Unrecognized option %c will be ignored.\n", *arg);
							break;
					}
				}
				else
					argstart = i;
				break;
		}
	}
	fill_default_options(&defops);
	int succ=argstart==argl?cpbuild(thisdirp,&defops):cpbuild(argv+argstart,&defops);
	free(defops.compilerpp);
	free(defops.compiler);
	if(defops.artifact!=artifact)
		free(defops.artifact);
	return succ;
}
