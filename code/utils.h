#ifndef Included_utils_h
#define Included_utils_h
void iterate_directory(const char*dirname,void(*func)(const char*,void*,int),void*arg);
int runprogram(unsigned char maxi,char*const*args);
char strcontains(const char*strlist,const char*str);
char*changeext_add_prefix(const char*og,const char*prefix,const char*ext);
void wait_children(void);
#endif
