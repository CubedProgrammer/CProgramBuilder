#ifndef Included_config_h
#define Included_config_h
#include"cpbuild.h"
char*read_config(const char*fname);
char**parse_args(int argl,char**argv,struct cpbuild_options*options);
#endif
