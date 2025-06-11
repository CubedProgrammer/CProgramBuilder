#ifndef Included_cpbuild_h
#define Included_cpbuild_h
#define BOOLOPS_FORCE 1
#define BOOLOPS_DISPLAY_COMMAND 2
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
	char*artifact;
	char*compiler;
	char*compilerpp;
	char*objdir;
	unsigned pathshift;
	unsigned short boolops;
	unsigned char parallel;
	char helped;
	struct program_options compilerops,compilerppops;
	struct program_options linkerops;
	struct program_options ccmd,cppcmd;
	struct program_args linkerargs;
};
typedef struct cpbuild_options cpbuild_options_t;
int cpbuild(char**targets,unsigned len,cpbuild_options_t*opt);
int buildfile(char*filename,char*outfile,const cpbuild_options_t*opt);
int init_program_args(struct program_args*arr,unsigned short capa);
int append_program_arg(struct program_args*arr,char*arg);
int fill_default_options(cpbuild_options_t*opt);
int get_default_artifact(char**out,unsigned extra);
#endif
