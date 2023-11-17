#ifndef Included_utils_h
#define Included_utils_h
void iterate_directory(const char *dirname, void(*func)(const char*, void*), void *arg);
int runprogram(const char *program, char *const*args);
char strcontains(const char *strlist, const char *str);
char *changeext(const char *og, const char *ext);
#endif
