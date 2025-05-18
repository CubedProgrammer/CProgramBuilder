#ifndef Included_utils_h
#define Included_utils_h
struct vector_char
{
	char*str;
	unsigned len,cap;
};
void iterate_directory(const char*dirname,void(*func)(const char*,void*,int),void*arg);
int runprogram(unsigned char maxi,char*const*args);
int program_output(struct vector_char*data,char*const*args);
char strcontains(const char*strlist,const char*str);
char*changeext_add_prefix(const char*og,const char*prefix,const char*ext);
int init_vector_char(struct vector_char*this);
int push_vector_char(struct vector_char*this,const char*first,const char*last);
void free_vector_char(struct vector_char*this);
void wait_children(void);
#endif
