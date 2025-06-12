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
	char*thisdirp=thisdir;
	int succ=initialize_global_file_data();
	if(succ==0)
	{
		memset(&defops,0,sizeof defops);
		char**argstart=argv+1;
		char*conffile=NULL;
		char*emptyargs[]={"-b",conffile};
		int freearti=1;
		struct program_args targetArray={NULL,0,0};
		if(argl==1)
		{
			get_default_artifact(&conffile,5);
			if(conffile!=NULL)
			{
				strcat(conffile,".conf");
				emptyargs[1]=conffile;
				if(access(conffile,F_OK)==0)
				{
					argstart=emptyargs;
					argl=2;
					targetArray=parse_args(argv[0],argl,argstart,&defops);
				}
			}
		}
		else
		{
			--argl;
			targetArray=parse_args(argv[0],argl,argstart,&defops);
		}
		freearti=defops.artifact==NULL;
		if(fill_default_options(&defops))
		{
			perror("fill_default_options failed:");
			argstart=NULL;
		}
		char**ptr;
		unsigned len;
		if(targetArray.len==0)
		{
			ptr=&thisdirp;
			len=1;
		}
		else
		{
			ptr=targetArray.options;
			len=targetArray.len;
		}
		if(!defops.helped)
		{
			succ=cpbuild(ptr,len,&defops);
			wait_children();
		}
		if(targetArray.options!=NULL)
		{
			free(targetArray.options);
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
