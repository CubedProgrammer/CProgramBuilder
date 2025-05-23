#include<stddef.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"config.h"
struct string_data
{
	char*carray;
	char**cparray;
	char**it;
	size_t cplen;
};
struct string_data_array
{
	struct string_data*data;
	size_t len;
	size_t capa;
};
struct allocated_file_data*global_file_data;
size_t global_file_len;
size_t global_file_cap;
const unsigned VERSION_MAJOR=0;
const unsigned VERSION_MINOR=3;
const unsigned VERSION_PATCH=2;
int initialize_global_file_data(void)
{
	int failed=1;
	global_file_data=malloc(2*sizeof(*global_file_data));
	if(global_file_data)
	{
		global_file_cap=2;
		failed=0;
	}
	return failed;
}
int append_global_file_data(const struct allocated_file_data*item)
{
	int failed=0;
	if(global_file_len==global_file_cap)
	{
		size_t capacity=global_file_cap+(global_file_cap>>1);
		struct allocated_file_data*new=realloc(global_file_data,sizeof(*global_file_data));
		if(new!=NULL)
		{
			global_file_data=new;
			global_file_cap=capacity;
		}
		else
			failed=1;
	}
	if(!failed)
		global_file_data[global_file_len++]=*item;
	return failed;
}
void free_global_file_data(void)
{
	for(struct allocated_file_data*it=global_file_data;it!=global_file_data+global_file_len;++it)
	{
		free(it->a);
		free(it->b);
	}
	free(global_file_data);
	global_file_data=NULL;
	global_file_len=0;
	global_file_cap=0;
}
int append_data_array(struct string_data_array*array,const struct string_data*data)
{
	int failed=0;
	if(array->capa==array->len)
	{
		size_t capa=array->capa+(array->capa>>1);
		struct string_data*new=malloc(capa*sizeof(struct string_data));
		failed=1;
		if(new!=NULL)
		{
			memcpy(new,array->data,array->len*sizeof(struct string_data));
			free(array->data);
			array->data=new;
			failed=0;
		}
	}
	if(!failed)
		array->data[array->len++]=*data;
	return failed;
}
char**make_string_array(char*array,size_t*len)
{
	const char*last=array;
	*len=0;
	int ended=0;
	for(const char*it=array;!ended;++it)
	{
		if(*it=='\0')
		{
			ended=last==it;
			last=it+1;
			++*len;
		}
	}
	--*len;
	char**stringarray=malloc(*len*sizeof(char*));
	if(stringarray!=NULL)
	{
		size_t salen=0;
		--last;
		stringarray[0]=array;
		for(char*it=array;it!=last;++it)
		{
			if(*it=='\0')
			{
				++salen;
				if(salen<*len)
					stringarray[salen]=it+1;
			}
		}
	}
	return stringarray;
}
void help_screen(const char*program)
{
	printf("USAGE: %s [OPTIONS...] FILES...\n\n",program);
	puts("If no files or directories are specified, compile the present working directory.");
	puts("If no command line arguments are specified, reads the options from a file named the same as the base name of the PWD, with .conf at the end.");
	puts("If the PWD is /home/alice/Documents/MyProgram, read the options from the file MyProgram.conf.");
	puts("Options MUST come before the files and directories to be compiled.");
	puts("Unless a file is specific to the option, then it must come immediately after said option.");
	puts("-A: path to artifact to be built, the final executable file or shared object file.");
	puts("-C{N}: Command line arguments to be passed to the C++ compiler, N arguments must come after this.");
	puts("-L{N}: Linker arguments, N arguments must come after this.");
	puts("-b FILE: Load extra options from a file, the file must list exact one argument per line.");
	puts("-c{N}: Command line arguments to be passed to the C compiler, N arguments must come after this.");
	puts("-f: Force compilation, do not ignore files that have not been updated.");
	puts("-o DIRECTORY: Put the object files into this directory instead of the PWD.");
	puts("-s: Show commands being executed on the screen.");
	puts("--cc PROGRAM: Specifies the C compiler.");
	puts("--c++ PROGRAM: Specifies the C++ compiler.");
	puts("--version: Shows the program version.");
	puts("--help: Shows this message.");
}
char**parse_help(const char*name,struct cpbuild_options*options,char**first,char**last)
{
	unsigned nxtarg=0,nxtcnt=0;
	char*arg;
	struct program_options*currops=NULL;
	struct string_data_array stack={malloc(2*sizeof(struct string_data)),0,2};
	struct string_data dat;
	struct allocated_file_data pointers;
	char moveiter=0,helped=0;
	char**baseit=first;
	char**it=baseit;
	size_t shift=0;
	char*tmp;
	while(baseit!=last&&!helped)
	{
		arg=*it;
		if(stack.len==0&&shift>0)
		{
			tmp=it[0];
			it[0]=it[-shift];
			it[-shift]=tmp;
		}
		if(nxtcnt==0)
			nxtarg=0;
		switch(nxtarg)
		{
			case 8:
				dat.carray=read_config(arg);
				if(dat.carray!=NULL)
				{
					dat.cparray=make_string_array(dat.carray, &dat.cplen);
					if(dat.cparray!=NULL)
					{
						dat.it=dat.cparray;
						pointers.a=dat.carray;
						pointers.b=dat.cparray;
						if(!append_global_file_data(&pointers))
						{
							if(!append_data_array(&stack,&dat))
							{
								moveiter=1;
							}
							else
							{
								fprintf(stderr,"Failed to allocate memory for stack at capacity %zu\n",stack.capa);
								--global_file_len;
								free(pointers.b);
								free(pointers.a);
							}
						}
						else
						{
							fprintf(stderr,"Failed to allocate memory for global array at capacity %zu\n", global_file_cap);
							free(pointers.b);
							free(pointers.a);
						}
					}
					else
					{
						free(dat.carray);
						dat.carray=NULL;
					}
				}
				if(dat.carray==NULL)
					fprintf(stderr,"Processing file %s failed, could not allocate memory.\n",arg);
				--nxtcnt;
				break;
			case 7:
				options->compilerpp=arg;
				--nxtcnt;
				break;
			case 6:
				options->compiler=arg;
				--nxtcnt;
				break;
			case 5:
				options->objdir=arg;
				--nxtcnt;
				break;
			case 4:
				currops=&options->linkerops;
				break;
			case 3:
				currops=&options->compilerppops;
				break;
			case 2:
				currops=&options->compilerops;
				break;
			case 1:
				options->artifact=arg;
				--nxtcnt;
				break;
			default:
				if(arg[0]=='-')
				{
					++arg;
					switch(*arg)
					{
						case'A':
							nxtarg=1;
							nxtcnt=1;
							break;
						case'C':
							nxtarg=3;
							break;
						case'L':
							nxtarg=4;
							break;
						case'b':
							nxtarg=8;
							nxtcnt=1;
							break;
						case'c':
							nxtarg=2;
							break;
						case'f':
							options->boolops|=BOOLOPS_FORCE;
							break;
						case'j':
							options->parallel=atoi(arg+1);
							break;
						case'o':
							nxtarg=5;
							nxtcnt=1;
							break;
						case's':
							options->boolops|=BOOLOPS_DISPLAY_COMMAND;
							break;
						case'-':
							if(strcmp(arg+1,"cc")==0)
							{
								nxtarg=6;
								nxtcnt=1;
							}
							else if(strcmp(arg+1,"c++")==0)
							{
								nxtarg=7;
								nxtcnt=1;
							}
							else if(strcmp(arg+1,"help")==0)
							{
								help_screen(name);
								helped=1;
							}
							else if(strcmp(arg+1,"version")==0)
							{
								printf("%s version %u.%u.%u\n",name,VERSION_MAJOR,VERSION_MINOR,VERSION_PATCH);
								helped=1;
							}
							break;
						default:
							fprintf(stderr,"Unrecognized option %c will be ignored.\n",*arg);
							break;
					}
					if(nxtarg==2||nxtarg==3||nxtarg==4)
					{
						if(arg[1]=='\0')
							nxtcnt=1;
						else
							nxtcnt=atoi(arg+1);
					}
				}
				else if(stack.len==0)
				{
					++shift;
				}
				break;
		}
		if(currops!=NULL)
		{
			currops->options=it;
			currops->len=nxtcnt;
			nxtcnt=0;
			if(stack.len==0)
			{
				baseit+=currops->len-1;
			}
			else
				stack.data[stack.len-1].it+=currops->len-1;
			currops=NULL;
		}
		if(!moveiter)
		{
			while(stack.len>0&&++stack.data[stack.len-1].it==stack.data[stack.len-1].cparray+stack.data[stack.len-1].cplen)
			{
				--stack.len;
			}
		}
		moveiter=0;
		if(stack.len==0)
		{
			++baseit;
			it=baseit;
		}
		else
			it=stack.data[stack.len-1].it;
	}
	free(stack.data);
	return helped?NULL:last-shift;
}
char*read_config(const char*fname)
{
	char*content=malloc(1);
	char*new;
	size_t len=0;
	size_t capa=1;
	size_t nc;
	FILE*fhandle=fopen(fname,"rb");
	int c=0,d=0;
	if(fhandle!=NULL)
	{
		while(content!=NULL&&c!=-1)
		{
			c=fgetc(fhandle);
			if(len==capa)
			{
				nc=capa+(capa>>1);
				nc+=nc==capa;
				new=realloc(content,nc);
				if(new==NULL)
					free(content);
				else
					capa=nc;
				content=new;
			}
			if(content!=NULL)
			{
				d=c;
				if(c=='\n'||c==-1)
					d=0;
				content[len++]=d;
			}
		}
		fclose(fhandle);
	}
	if(len==1)
	{
		free(content);
		content=NULL;
	}
	else if(content[len-2]!='\0'||capa!=len)
	{
		new=realloc(content,len+=content[len-2]!='\0');
		if(new==NULL)
		{
			content=NULL;
		}
		else
			content=new;
		if(content!=NULL)
			content[len-1]='\0';
	}
	return content;
}
char**parse_args(const char*name,int argl,char**argv,struct cpbuild_options*options)
{
	return parse_help(name,options,argv,argv+argl);
}
