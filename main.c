#include<stdlib.h>
#include<string.h>
#include"config.h"
#include"cpbuild.h"
int main(int argl,char**argv)
{
	struct cpbuild_options defops;
	char thisdir[]=".";
	char*thisdirp[]={thisdir,NULL};
	struct program_options*currops=NULL;
	memset(&defops,0,sizeof defops);
	char**argstart=parse_args(argl-1,argv+1,&defops);
	int freearti=defops.artifact==NULL;
	fill_default_options(&defops);
	int succ=argstart==argv+argl?cpbuild(thisdirp,&defops):cpbuild(argstart,&defops);
	free(defops.compilerpp);
	free(defops.compiler);
	if(freearti)
		free(defops.artifact);
	return succ;
}
