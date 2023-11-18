#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<unistd.h>
#include"cpbuild.h"
#include"utils.h"
const char cpb_default_option_list[] = "cc\0c++";
const char cpb_accepted_extensions[] = "c\0c++\0cpp\0cxx\0";
void build_callback(const char *file, void *arg)
{
	struct cpbuild_options *opt = arg;
	char *periodptr = strrchr(file, '.');
	size_t len = strlen(file);
	if(strcontains(cpb_accepted_extensions, periodptr + 1))
	{
		char *filename = malloc(len + 1), *objname = changeext(file, "o");
		if(objname == NULL || filename == NULL)
			perror("malloc failed");
		else
		{
			memcpy(filename, file, len + 1);
			append_program_arg(&opt->linkerargs, objname);
			buildfile(filename, objname, opt);
			free(filename);
		}
	}
}
int cpbuild(char **targets, struct cpbuild_options *opt)
{
	struct stat fdat;
	char outputop[] = "-o";
	unsigned short argcapa = 16;
	if(opt->linkerops.len + 3 > argcapa)
		argcapa = opt->linkerops.len + 3;
	init_program_args(&opt->linkerargs, argcapa);
	opt->linkerargs.options[0] = opt->compiler;
	opt->linkerargs.options[1] = outputop;
	opt->linkerargs.options[2] = opt->artifact;
	memcpy(opt->linkerargs.options + 3, opt->linkerops.options, sizeof(char*) * opt->linkerops.len);
	opt->linkerargs.len = opt->linkerops.len + 3;
	for(char **it = targets; *it != NULL; ++it)
	{
		if(stat(*it, &fdat))
			perror("stat failed");
		else if(S_ISDIR(fdat.st_mode))
			iterate_directory(*it, &build_callback, opt);
		else
		{
			if(append_program_arg(&opt->linkerargs, changeext(*it, "o")) == 0)
				buildfile(*it, opt->linkerargs.options[opt->linkerargs.len - 1], opt);
			else
			{
				fprintf(stderr, "Adding %s", *it);
				perror(" to linker array failed");
			}
		}
	}
	runprogram(opt->linkerargs.options);
	for(unsigned short i = 3; i < opt->linkerargs.len; ++i)
		free(opt->linkerargs.options[i]);
	free(opt->linkerargs.options);
	return 0;
}
int buildfile(char *filename,char*outfile,const cpbuild_options_t*opt)
{
	int succ = 1;
	struct stat fdat, odat;
	unsigned short len = opt->compilerops.len;
	char*compiler=opt->compiler;
	char recompile = (opt->boolops & BOOLOPS_FORCE) == BOOLOPS_FORCE;
	char outputop[] = "-o", compileop[] = "-c";
	char**compilerops=opt->compilerops.options;
	char*fileext=strrchr(filename,'.');
	if(fileext!=NULL&&strcontains(cpb_accepted_extensions+2,fileext+1))
	{
		compiler=opt->compilerpp;
		compilerops=opt->compilerppops.options;
		len=opt->compilerppops.len;
		opt->linkerargs.options[0]=opt->compilerpp;
	}
	char**args=malloc((len+6)*sizeof(*args));
	if(args == NULL)
		perror("malloc failed");
	else
	{
		args[0] = compiler;
		args[1] = compileop;
		memcpy(args+2,compilerops,len*sizeof(char*));
		args[len + 2] = filename;
		args[len + 3] = outputop;
		args[len + 4] = outfile;
		args[len + 5] = NULL;
		if(!recompile)
		{
			if(stat(outfile, &odat))
				recompile = 1;
			else if(stat(filename, &fdat) == 0)
				recompile = fdat.st_mtime > odat.st_mtime;
		}
		if(recompile)
			succ=runprogram(args);
		free(args);
	}
	return succ;
}
int init_program_args(struct program_args *arr, unsigned short capa)
{
	int succ = 0;
	arr->options = malloc(capa * sizeof(*arr->options));
	if(arr->options == NULL)
		succ = -1;
	else
	{
		arr->len = 0;
		arr->capa = capa;
	}
	return succ;
}
int append_program_arg(struct program_args *arr, char *arg)
{
	int succ = 0;
	if(arr->len == arr->capa)
	{
		char **new = malloc(arr->capa + (arr->capa >> 1));
		if(new != NULL)
		{
			arr->capa += arr->capa >> 1;
			memcpy(new, arr->options, arr->len * sizeof(char*));
			free(arr->options);
			arr->options = new;
		}
		else
			succ = -1;
	}
	if(succ == 0)
		arr->options[arr->len++] = arg;
	return succ;
}
int fill_default_options(cpbuild_options_t*opt)
{
	int succ=0;
	char cbuf[4096];
	if(opt->artifact==NULL)
	{
		if(realpath(".", cbuf)==NULL)
			succ=-1;
		else
		{
			unsigned off=0,len;
			for(const char*it=cbuf;*it!='\0';++len,++it)
			{
				if(*it=='/')
				{
					off=it+1-cbuf;
					len=0;
				}
			}
			opt->artifact=malloc(len);
			if(opt->artifact==NULL)
				succ=-1;
			else
				memcpy(opt->artifact,cbuf+off,len);
		}
	}
	if(opt->compiler==NULL)
	{
		opt->compiler=malloc(3);
		if(opt->compiler==NULL)
			succ=-1;
		else
			memcpy(opt->compiler,cpb_default_option_list,3);
	}
	if(opt->compilerpp==NULL)
	{
		opt->compilerpp=malloc(4);
		if(opt->compilerpp==NULL)
			succ=-1;
		else
			memcpy(opt->compilerpp,cpb_default_option_list+3,4);
	}
	return succ;
}
