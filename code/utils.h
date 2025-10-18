#ifndef Included_utils_h
#define Included_utils_h
struct vector_char
{
	char*str;
	unsigned len,cap;
};
struct string_hashtable_entry
{
	const char*str;
	struct vector_char vec;
	struct string_hashtable_entry*next;
};
struct string_hashtable
{
	struct string_hashtable_entry**table;
	unsigned len,cap;
};
typedef int(*directory_iterator_callback_t)(const char*,void*,int);
typedef struct string_hashtable string_hashtable;
void iterate_directory(const char*dirname,directory_iterator_callback_t func,void*arg);
int runprogram(unsigned char maxi,char*const*args);
int program_output(struct vector_char*data,char*const*args);
char strcontains(const char*strlist,const char*str);
char*changeext_add_prefix(const char*og,const char*prefix,const char*ext);
int init_vector_char(struct vector_char*this);
int push_vector_char(struct vector_char*this,const char*first,const char*last);
void free_vector_char(struct vector_char*this);
size_t hash_string(const char*str);
void wait_children(void);
#endif
