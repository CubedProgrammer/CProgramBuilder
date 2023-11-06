#ifndef Included_utils_h
#define Included_utils_h
void iterate_directory(const char *dirname, void(*func)(const char*, const void*), const void *arg);
int runprogram(const char *program, char *const*args);
#endif
