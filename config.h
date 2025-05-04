#ifndef Included_config_h
#define Included_config_h
#include"cpbuild.h"
void read_config(struct cpbuild_options*options,const char*fname);
char**parse_args(int argl,char**argv,struct cpbuild_options*options);
#endif
