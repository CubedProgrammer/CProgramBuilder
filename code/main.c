#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include"config.h"
#include"cpbuild.h"
#include"utils.h"
int main(int argl,char**argv)
{
	struct cpbuild_options defops;
	char thisdir[]=".";
	char*thisdirp[]={thisdir,NULL};
	int succ=initialize_global_file_data();
	if(succ==0)
	{
		memset(&defops,0,sizeof defops);
		char**argstart=argv+1;
		char**oldstart=argstart;
		char*conffile=NULL;
		char*emptyargs[]={"-b",conffile,NULL};
		int freearti=1;
		if(argl==1)
		{
			fill_default_options(&defops);
			size_t artilen=strlen(defops.artifact);
			conffile=malloc(artilen + 6);
			if(conffile!=NULL)
			{
				memcpy(conffile,defops.artifact,artilen);
				memcpy(conffile+artilen,".conf",6);
				emptyargs[1]=conffile;
				if(access(conffile,F_OK)==0)
				{
					oldstart=argstart=emptyargs;
					argl=2;
					argstart=parse_args(argv[0],argl,argstart,&defops);
				}
			}
		}
		else
		{
			--argl;
			argstart=parse_args(argv[0],argl,argstart,&defops);
			freearti=defops.artifact==NULL;
			if(fill_default_options(&defops))
			{
				perror("fill_default_options failed:");
				argstart=NULL;
			}
		}
		if(argstart!=NULL)
		{
			if(argstart==oldstart+argl)
			{
				argstart=thisdirp;
			}
			succ=cpbuild(argstart,&defops);
			wait_children();
		}
		if(conffile!=NULL)
		{
			free(conffile);
		}
		if(freearti)
			free(defops.artifact);
		free_global_file_data();
	}
	else
		perror("initialize_global_file_data failed: malloc failed");
	return succ;
}
