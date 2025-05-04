#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"config.h"
char**parse_help(struct cpbuild_options*options,char**first,char**last,int deep)
{
	unsigned nxtarg=0,nxtcnt=0;
	char*arg,*artifact=NULL;
	struct program_options*currops=NULL;
	char**it;
	for(it=first;it!=last;++it)
	{
		arg=*it;
		if(nxtcnt==0)
			nxtarg=0;
		switch(nxtarg)
		{
			case 7:
				options->compilerpp=malloc(strlen(arg)+1);
				strcpy(options->compilerpp,arg);
				--nxtcnt;
				break;
			case 6:
				options->compiler=malloc(strlen(arg)+1);
				strcpy(options->compiler,arg);
				--nxtcnt;
				break;
			case 5:
				options->objdir=arg;
				--nxtcnt;
				break;
			case 4:
				currops=&options->linkerops;
				break;
			case 3:
				currops=&options->compilerppops;
				break;
			case 2:
				currops=&options->compilerops;
				break;
			case 1:
				artifact=options->artifact=arg;
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
							options->boolops |= BOOLOPS_FORCE;
							break;
						case's':
							options->boolops|=BOOLOPS_DISPLAY_COMMAND;
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
					last=it--;
				break;
		}
		if(currops!=NULL)
		{
			currops->options=it;
			currops->len=nxtcnt;
			nxtcnt=0;
			it+=currops->len-1;
			currops=NULL;
		}
	}
	return it;
}
void read_config(struct cpbuild_options*options,const char*fname)
{}
char**parse_args(int argl,char**argv,struct cpbuild_options*options)
{
	return parse_help(options,argv,argv+argl,1);
}
