#ifndef Included_config_h
#define Included_config_h
#include"cpbuild.h"
struct allocated_file_data
{
	char*a;
	char**b;
};
int initialize_global_file_data(void);
int append_global_file_data(const struct allocated_file_data*item);
void free_global_file_data(void);
char*read_config(const char*fname);
struct program_args parse_args(const char*name,int argl,char**argv,struct cpbuild_options*options);
#endif
