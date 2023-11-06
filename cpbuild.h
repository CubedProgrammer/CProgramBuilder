#ifndef Included_cpbuild_h
#define Included_cpbuild_h
#define BOOLOPS_FORCE 1
struct program_options
{
    char **options;
    unsigned short len;
};
struct cpbuild_options
{
    unsigned short boolops;
    char *artifact;
    char *compiler;
    struct program_options compilerops;
    struct program_options linkerops;
};
typedef struct cpbuild_options cpbuild_options_t;
int cpbuild(char **targets, const cpbuild_options_t *opt);
int buildfile(char *filename, const cpbuild_options_t *opt);
#endif
