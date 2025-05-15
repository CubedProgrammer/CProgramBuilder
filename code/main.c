#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include"config.h"
#include"cpbuild.h"
int main(int argl,char**argv)
{
	struct cpbuild_options defops;
	char thisdir[]=".";
	char*thisdirp[]={thisdir,NULL};
	int succ=initialize_global_file_data();
	if(succ==0)
	{
		memset(&defops,0,sizeof defops);
		char**argstart=parse_args(argl-1,argv+1,&defops);
		int freearti=defops.artifact==NULL;
		fill_default_options(&defops);
		succ=argstart==argv+argl?cpbuild(thisdirp,&defops):cpbuild(argstart,&defops);
		if(freearti)
			free(defops.artifact);
		free_global_file_data();
	}
	else
		perror("initialize_global_file_data failed: malloc failed");
	return succ;
}
