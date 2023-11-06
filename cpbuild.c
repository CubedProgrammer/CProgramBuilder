#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<unistd.h>
#include"cpbuild.h"
#include"utils.h"
void build_callback(const char *file, const void *arg)
{
    const struct cpbuild_options *opt = arg;
    size_t len = strlen(file);
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
    size_t lastperiod, namelen = 0;
    struct stat fdat, odat;
    unsigned short len = opt->compilerops.len;
    char *outfile, **args = malloc((len + 4) * sizeof(*args));
    char *compiler = opt->compiler;
    char recompile = 0, outputop[] = "-o";
    if(args == NULL)
        perror("malloc failed");
    else
    {
        args[0] = compiler;
        memcpy(args + 1, opt->compilerops.options, len * sizeof(char*));
        args[len + 1] = filename;
        args[len + 2] = outputop;
        for(char *it = filename; filename[namelen] != '\0'; ++namelen)
        {
            if(filename[namelen] == '.')
                lastperiod = namelen;
        }
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
            args[len + 3] = outfile;
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
