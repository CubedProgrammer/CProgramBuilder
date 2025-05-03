#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"cpbuild.h"
#include"utils.h"
int main(int argl,char**argv)
{
	struct cpbuild_options defops;
	int argstart=argl;
	unsigned nxtarg = 0, nxtcnt = 0;
	char*arg,*artifact=NULL;
	char thisdir[]=".";
	char*thisdirp[]={thisdir,NULL};
	struct program_options*currops=NULL;
	memset(&defops,0,sizeof defops);
	for(int i=1;i<argstart;++i)
	{
		arg=argv[i];
		if(nxtcnt==0)
			nxtarg=0;
		switch(nxtarg)
		{
			case 7:
				defops.compilerpp=malloc(strlen(arg)+1);
				strcpy(defops.compilerpp,arg);
				--nxtcnt;
				break;
			case 6:
				defops.compiler=malloc(strlen(arg)+1);
				strcpy(defops.compiler,arg);
				--nxtcnt;
				break;
			case 5:
				defops.objdir = arg;
				--nxtcnt;
				break;
			case 4:
				currops=&defops.linkerops;
				break;
			case 3:
				currops=&defops.compilerppops;
				break;
			case 2:
				currops=&defops.compilerops;
				break;
			case 1:
				artifact=defops.artifact=arg;
				--nxtcnt;
				break;
			default:
				if(arg[0]=='-')
				{
					++arg;
					switch(*arg)
					{
						case'A':
							nxtarg=1;
							nxtcnt=1;
							break;
						case'C':
							nxtarg=3;
							break;
						case'L':
							nxtarg=4;
							break;
						case'c':
							nxtarg=2;
							break;
						case'f':
							defops.boolops |= BOOLOPS_FORCE;
							break;
						case's':
							defops.boolops|=BOOLOPS_DISPLAY_COMMAND;
							break;
						case'o':
							nxtarg = 5;
							nxtcnt = 1;
							break;
						case'-':
							if(strcmp(arg+1,"cc")==0)
							{
								nxtarg=6;
								nxtcnt=1;
							}
							else if(strcmp(arg+1,"c++")==0)
							{
								nxtarg=7;
								nxtcnt=1;
							}
							break;
						default:
							fprintf(stderr,"Unrecognized option %c will be ignored.\n",*arg);
							break;
					}
					if(nxtarg==2||nxtarg==3||nxtarg==4)
					{
						if(arg[1]=='\0')
							nxtcnt=1;
						else
							nxtcnt=atoi(arg+1);
					}
				}
				else
					argstart = i;
				break;
		}
		if(currops!=NULL)
		{
			currops->options=argv+i;
			currops->len=nxtcnt;
			nxtcnt=0;
			i+=currops->len-1;
			currops=NULL;
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
