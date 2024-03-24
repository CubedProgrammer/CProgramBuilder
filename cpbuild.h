#ifndef Included_cpbuild_h
#define Included_cpbuild_h
#define BOOLOPS_FORCE 1
struct program_options
{
	char**options;
	unsigned short len;
};
struct program_args
{
	char**options;
	unsigned short len,capa;
};
struct cpbuild_options
{
	unsigned short boolops;
	char*artifact;
	char*compiler,*compilerpp;
	char*objdir;
	struct program_options compilerops,compilerppops;
	struct program_options linkerops;
	struct program_args linkerargs;
};
typedef struct cpbuild_options cpbuild_options_t;
int cpbuild(char**targets,cpbuild_options_t*opt);
int buildfile(char*filename,char*outfile,const cpbuild_options_t*opt);
int init_program_args(struct program_args*arr,unsigned short capa);
int append_program_arg(struct program_args*arr,char*arg);
int fill_default_options(cpbuild_options_t*opt);
#endif
