#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<unistd.h>
#include"cpbuild.h"
#include"utils.h"
const char cpb_default_option_list[] = "cc\0c++";
const char cpb_accepted_extensions[] = "c\0c++\0cpp\0cxx\0";
void build_callback(const char *file, const void *arg)
{
	const struct cpbuild_options *opt = arg;
	char *periodptr = strrchr(file, '.');
	size_t len = strlen(file);
	if(strcontains(cpb_accepted_extensions, periodptr + 1))
	{
		char *filename = malloc(len + 1);
		if(filename == NULL)
			perror("malloc failed");
		else
		{
			memcpy(filename, file, len + 1);
			buildfile(filename, opt);
			free(filename);
		}
	}
}
int cpbuild(char **targets, const struct cpbuild_options *opt)
{
	struct stat fdat;
	for(char **it = targets; *it != NULL; ++it)
	{
		if(stat(*it, &fdat))
			perror("stat failed");
		else if(S_ISDIR(fdat.st_mode))
			iterate_directory(*it, &build_callback, opt);
		else
			buildfile(*it, opt);
	}
	return 0;
}
int buildfile(char *filename, const struct cpbuild_options *opt)
{
	int succ = 1;
	size_t lastperiod, namelen;
	struct stat fdat, odat;
	unsigned short len = opt->compilerops.len;
	char *outfile, **args = malloc((len + 6) * sizeof(*args));
	char *compiler = opt->compiler;
	const char *periodptr;
	char recompile = 0;
	char outputop[] = "-o", compileop[] = "-c";
	if(args == NULL)
		perror("malloc failed");
	else
	{
		args[0] = compiler;
		args[1] = compileop;
		memcpy(args + 2, opt->compilerops.options, len * sizeof(char*));
		args[len + 2] = filename;
		args[len + 3] = outputop;
		periodptr = strrchr(filename, '.');
		if(periodptr == NULL)
			lastperiod = 0;
		else
			lastperiod = periodptr - filename;
		namelen = strlen(filename + lastperiod) + lastperiod;
		if(lastperiod == 0)
			namelen += 2;
		else
			namelen = lastperiod + 2;
		outfile = malloc((namelen + 1) * sizeof(*outfile));
		if(outfile == NULL)
			perror("malloc failed");
		else
		{
			memcpy(outfile, filename, namelen - 2);
			strcpy(outfile + namelen - 2, ".o");
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
				succ = runprogram(compiler, args) > 0;
			free(outfile);
		}
		free(args);
	}
	return succ;
}

int fill_default_options(cpbuild_options_t *opt)
{
	int succ = 0;
	char cbuf[4096];
	if(opt->artifact == NULL)
	{
		if(realpath(".", cbuf) == NULL)
			succ = -1;
		else
		{
			unsigned off = 0, len;
			for(const char *it = cbuf; *it != '\0'; ++len, ++it)
			{
				if(*it == '/')
				{
					off = it + 1 - cbuf;
					len = 0;
				}
			}
			opt->artifact = malloc(len);
			if(opt->artifact == NULL)
				succ = -1;
			else
				memcpy(opt->artifact, cbuf + off, len);
		}
	}
	if(opt->compiler == NULL)
	{
		opt->compiler = malloc(3);
		if(opt->compiler == NULL)
			succ = -1;
		else
			memcpy(opt->compiler, cpb_default_option_list, 3);
	}
	return succ;
}
