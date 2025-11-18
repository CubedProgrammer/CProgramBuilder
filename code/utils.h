#ifndef Included_utils_h
#define Included_utils_h
struct vector_char
{
	char*str;
	unsigned len,cap;
};
struct string_hashtable_entry
{
	char*str;
	struct vector_char vec;
	struct string_hashtable_entry*next;
};
struct string_hashtable
{
	struct string_hashtable_entry**table;
	unsigned len,cap;
};
struct string_hashtable_iterator
{
	struct string_hashtable_entry**it,**endit;
	struct string_hashtable_entry*node;
};
typedef int(*directory_iterator_callback_t)(const char*,void*,int);
typedef struct string_hashtable string_hashtable;
typedef struct string_hashtable_iterator string_hashtable_iterator;
void iterate_directory(const char*dirname,directory_iterator_callback_t func,void*arg);
int runprogram(unsigned char maxi,char*const*args);
int program_output(struct vector_char*data,char*const*args);
char strcontains(const char*strlist,const char*str);
char*changeext_add_prefix(const char*og,const char*prefix,const char*ext);
int init_vector_char(struct vector_char*this);
int push_vector_char(struct vector_char*this,const char*first,const char*last);
void free_vector_char(struct vector_char*this);
int init_string_hashtable(string_hashtable*this);
int reallocate_string_hashtable(string_hashtable*this);
int insert_string_hashtable(string_hashtable*this,char*key,struct vector_char value);
struct string_hashtable_entry*find_string_hashtable(string_hashtable*this,const char*key);
string_hashtable_iterator begin_string_hashtable(const string_hashtable*this);
string_hashtable_iterator end_string_hashtable(const string_hashtable*this);
void free_string_hashtable(string_hashtable*this);
int equal_sht_iterator(string_hashtable_iterator a,string_hashtable_iterator b);
void next_sht_iterator(string_hashtable_iterator*it);
struct string_hashtable_entry*get_sht_iterator(const string_hashtable_iterator*it);
long unsigned hash_string(const char*str);
void wait_children(void);
#endif
