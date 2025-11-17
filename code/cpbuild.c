#include<ctype.h>
#include <stddef.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<unistd.h>
#include"cpbuild.h"
#include"utils.h"
const char cpb_default_option_list[]="cc\0c++";
const char cpb_accepted_extensions[]="c\0c++\0cc\0cpp\0cxx\0";
char cpb_cmd_option_list[]="-c\0-o\0-MM";
struct option_and_files
{
	cpbuild_options_t*opt;
	struct program_args files;
	struct program_args folders;
};
int buildfile(FILE*cacheHandle,char*filename,char*outfile,const cpbuild_options_t*opt);
int timespec_compare(const struct timespec*a,const struct timespec*b)
{
	long c[2]={a->tv_sec-b->tv_sec,a->tv_nsec-b->tv_nsec};
	int64_t result=c[c[0]==0];
	return(result>=0)+(result>0)-1;
}
int build_callback(const char*file,void*arg,int info)
{
	struct option_and_files*oaf=arg;
	struct cpbuild_options*opt=oaf->opt;
	int r=0;
	int isdir=info&1;
	char*periodptr=strrchr(file,'.');
	size_t len=strlen(file);
	info>>=1;
	if(isdir)
	{
		if(len>opt->pathshift&&info==3)
		{
			char*directory=changeext_add_prefix(file+opt->pathshift,opt->objdir,"");
			if(directory==NULL)
			{
				perror("directory to be created is NULL, malloc failed");
			}
			else
			{
				r=append_program_arg(&oaf->folders,directory)==0;
			}
		}
	}
	else if(periodptr!=NULL&&strcontains(cpb_accepted_extensions,periodptr+1))
	{
		char*filename=malloc(len+1),*objname=changeext_add_prefix(file+opt->pathshift,opt->objdir,"o");
		if(objname==NULL||filename==NULL)
		{
			perror("objname or filename is NULL, malloc failed");
			if(objname!=NULL)
				free(objname);
			if(filename!=NULL)
				free(filename);
		}
		else
		{
			memcpy(filename,file,len+1);
			append_program_arg(&oaf->files,filename);
			append_program_arg(&opt->linkerargs,objname);
			r=1;
		}
	}
	return r;
}
int make_command_line(struct program_options*restrict d,const struct program_options*restrict s,char*compiler)
{
	int failed=1;
	d->options=malloc((s->len+6)*sizeof(*d->options));
	if(d->options!=NULL)
	{
		d->options[0]=compiler;
		memcpy(d->options+1,s->options,s->len*sizeof(char*));
		d->options[s->len+1]=cpb_cmd_option_list;
		d->options[s->len+3]=cpb_cmd_option_list+3;
		d->options[s->len+5]=NULL;
		d->len=s->len+6;
		failed=0;
	}
	return failed;
}
int parse_cache(FILE*handle,string_hashtable*table)
{
	int fail=0;
	size_t chcnt=0;
	char stackbuf[8192];
	char*file=stackbuf;
	char*tempbuf=NULL;
	size_t filecap=0;
	int isdependency=0;
	struct vector_char depends;
	for(char ch=fgetc(handle);!fail&&ch!=EOF;ch=fgetc(handle))
	{
		if(isdependency)
		{
			if(ch=='\n')
			{
				if(chcnt==0)
				{
					isdependency=0;
				}
				chcnt=0;
				ch='\0';
			}
			push_vector_char(&depends,&ch,&ch+1);
			if(!isdependency)
			{
				fail=insert_string_hashtable(table,file,depends);
				file=stackbuf;
			}
		}
		else if(ch=='\n')
		{
			file[chcnt]='\0';
			if(file==stackbuf)
			{
				file=malloc(++chcnt);
				if(file!=NULL)
				{
					memcpy(file,stackbuf,chcnt);
				}
				else
				{
					fail=1;
				}
			}
			chcnt=0;
			fail=init_vector_char(&depends);
			isdependency=1;
		}
		else if(chcnt+1>=sizeof(stackbuf))
		{
			if(file==stackbuf)
			{
				filecap=sizeof(stackbuf)<<1;
				file=malloc(filecap);
				memcpy(file,stackbuf,chcnt);
			}
			if(chcnt+1>=filecap)
			{
				tempbuf=realloc(file,filecap<<1);
				if(tempbuf!=NULL)
				{
					file=tempbuf;
				}
				else
				{
					fail=1;
				}
			}
			file[chcnt++]=ch;
		}
		else
		{
			stackbuf[chcnt++]=ch;
		}
	}
	return fail;
}
int cpbuild(char**targets,unsigned len,struct cpbuild_options*opt)
{
	int succ=0;
	int dependency_fail=0;
	struct stat fdat;
	char*target;
	int cfail=make_command_line(&opt->ccmd,&opt->compilerops,opt->compiler);
	int cppfail=make_command_line(&opt->cppcmd,&opt->compilerppops,opt->compilerpp);
	if(!cfail&&!cppfail)
	{
		size_t targetlen=len;
		size_t currLinkerLen;
		struct option_and_files oaf;
		string_hashtable dependency;
		FILE*cache=NULL;
		oaf.opt=opt;
		init_program_args(&opt->linkerargs,opt->linkerops.len+3);
		opt->linkerargs.options[0]=opt->compiler;
		opt->linkerargs.options[1]=cpb_cmd_option_list+3;
		opt->linkerargs.options[2]=opt->artifact;
		memcpy(opt->linkerargs.options+3,opt->linkerops.options,sizeof(char*)*opt->linkerops.len);
		opt->linkerargs.len=opt->linkerops.len+3;
		if(opt->cache!=NULL)
		{
			dependency_fail=init_string_hashtable(&dependency);
			cache=fopen(opt->cache,"rb");
			if(!dependency_fail&&cache!=NULL)
			{
				dependency_fail=parse_cache(cache,&dependency);
				fclose(cache);
				cache=NULL;
			}
		}
		if(targetlen==1)
		{
			opt->pathshift=strlen(targets[0])+1;
		}
		for(char**it=targets;it!=targets+targetlen;++it)
		{
			if(stat(*it,&fdat))
			{
				fprintf(stderr,"stat %s",*it);
				perror(" failed");
			}
			else if(S_ISDIR(fdat.st_mode))
			{
				currLinkerLen=opt->linkerargs.len;
				if(init_program_args(&oaf.files,16)==0&&init_program_args(&oaf.folders,4)==0)
				{
					iterate_directory(*it,&build_callback,&oaf);
					for(size_t i=oaf.folders.len;i>0;--i)
					{
						mkdir(oaf.folders.options[i-1],0755);
						free(oaf.folders.options[i-1]);
					}
					free(oaf.folders.options);
					for(size_t i=0;i<oaf.files.len;++i)
					{
						buildfile(cache,oaf.files.options[i],opt->linkerargs.options[currLinkerLen+i],opt);
						free(oaf.files.options[i]);
					}
					free(oaf.files.options);
				}
				else
				{
					if(oaf.files.options!=NULL)
					{
						free(oaf.files.options);
					}
					if(oaf.folders.options!=NULL)
					{
						free(oaf.folders.options);
					}
					perror("cpbuild: initializing array for files failed");
				}
			}
			else
			{
				if(append_program_arg(&opt->linkerargs, changeext_add_prefix(*it, opt->objdir, "o")) == 0)
					buildfile(cache,*it, opt->linkerargs.options[opt->linkerargs.len - 1], opt);
				else
				{
					fprintf(stderr, "Adding %s", *it);
					perror(" to linker array failed");
				}
			}
		}
		if(append_program_arg(&opt->linkerargs, NULL)==0)
		{
			if((opt->boolops&BOOLOPS_DISPLAY_COMMAND)==BOOLOPS_DISPLAY_COMMAND)
			{
				fputs(opt->linkerargs.options[0],stdout);
				for(char**it=opt->linkerargs.options+1;it!=opt->linkerargs.options+opt->linkerargs.len-1;++it)
				{
					putchar(' ');
					fputs(*it,stdout);
				}
				putchar('\n');
			}
			wait_children();
			runprogram(opt->parallel,opt->linkerargs.options);
		}
		else
			succ=-1;
		for(unsigned short i=opt->linkerops.len+3;i<opt->linkerargs.len-1;++i)
			free(opt->linkerargs.options[i]);
		free(opt->linkerargs.options);
		if(!dependency_fail&&opt->cache!=NULL)
		{
			free_string_hashtable(&dependency);
		}
	}
	if(opt->cppcmd.options!=NULL)
		free(opt->cppcmd.options);
	if(opt->ccmd.options!=NULL)
		free(opt->ccmd.options);
	return succ;
}
int buildfile(FILE*cacheHandle,char*filename,char*outfile,const cpbuild_options_t*opt)
{
	int succ=1;
	struct stat fdat,odat;
	unsigned short len=opt->compilerops.len;
	char**args=opt->ccmd.options;
	char recompile=(opt->boolops&BOOLOPS_FORCE)==BOOLOPS_FORCE;
	char*fileext=strrchr(filename,'.');
	if(fileext!=NULL&&strcontains(cpb_accepted_extensions+2,fileext+1))
	{
		args=opt->cppcmd.options;
		len=opt->compilerppops.len;
		opt->linkerargs.options[0]=opt->compilerpp;
	}
	args[len+2]=filename;
	if(!recompile)
	{
		if(stat(outfile, &odat))
			recompile=1;
		else if(stat(filename, &fdat)==0)
			recompile=timespec_compare(&fdat.st_mtim,&odat.st_mtim)>0;
	}
	if(!recompile)
	{
		struct vector_char arr;
		if(init_vector_char(&arr))
		{
			perror("buildfile failed: init_vector_char failed");
			fprintf(stderr,"Compiling %s anyways just in case.\n",filename);
			recompile=1;
		}
		else
		{
			args[len+1]=cpb_cmd_option_list+6;
			args[len+3]=NULL;
			int r=program_output(&arr,args);
			if(r==0)
			{
				char space=1,next;
				char*start=arr.str;
				char*last=arr.str+arr.len;
				char*it=arr.str;
				for(;it!=last&&*it!=':';++it);
				for(it+=it!=last;!recompile&&it!=last;++it)
				{
					next=isspace(*it)||*it=='\\';
					if(space&&!next)
					{
						start=it;
					}
					else if(!space&&next)
					{
						*it='\0';
						if(cacheHandle)
						{
							fprintf(cacheHandle,"%s\n",start);
						}
						fprintf(stdout,"%s\n",start);
						if(stat(start,&fdat))
						{
							fprintf(stderr,"buildfile failed: stat %s",start);
							perror(" failed");
						}
						else
						{
							recompile=timespec_compare(&fdat.st_mtim,&odat.st_mtim)>0;
						}
					}
					space=next;
				}
			}
			else
			{
				fputs("Executing",stderr);
				for(char**it=args;it!=args+len+6;++it)
				{
					fprintf(stderr," %s",*it);
				}
				fprintf(stderr," failed, recompiling %s anyways just in case.\n",filename);
				recompile=1;
			}
			args[len+1]=cpb_cmd_option_list;
			args[len+3]=cpb_cmd_option_list+3;
			free_vector_char(&arr);
		}
	}
	if(recompile)
	{
		args[len+4]=outfile;
		if((opt->boolops&BOOLOPS_DISPLAY_COMMAND)==BOOLOPS_DISPLAY_COMMAND)
		{
			fputs(args[0],stdout);
			for(char**it=args+1;it!=args+len+5;++it)
			{
				putchar(' ');
				fputs(*it,stdout);
			}
			putchar('\n');
		}
		succ=runprogram(opt->parallel,args);
	}
	return succ;
}
int init_program_args(struct program_args*arr,unsigned short capa)
{
	int succ=0;
	arr->options=malloc(capa*sizeof(*arr->options));
	if(arr->options==NULL)
		succ=-1;
	else
	{
		arr->len=0;
		arr->capa=capa;
	}
	return succ;
}
int append_program_arg(struct program_args*arr,char*arg)
{
	int succ=0;
	if(arr->len==arr->capa)
	{
		char**new=malloc((arr->capa+(arr->capa>>1))*sizeof(char*));
		if(new!=NULL)
		{
			arr->capa+=arr->capa>>1;
			memcpy(new,arr->options,arr->len*sizeof(char*));
			free(arr->options);
			arr->options=new;
		}
		else
			succ=-1;
	}
	if(succ==0)
		arr->options[arr->len++]=arg;
	return succ;
}
int fill_default_options(cpbuild_options_t*opt)
{
	int succ=0;
	if(opt->artifact==NULL)
	{
		succ=get_default_artifact(&opt->artifact,0);
	}
	if(opt->compiler==NULL)
	{
		opt->compiler=malloc(3);
		if(opt->compiler==NULL)
			succ=-1;
		else
			memcpy(opt->compiler,cpb_default_option_list,3);
	}
	if(opt->compilerpp==NULL)
	{
		opt->compilerpp=malloc(4);
		if(opt->compilerpp==NULL)
			succ=-1;
		else
			memcpy(opt->compilerpp,cpb_default_option_list+3,4);
	}
	opt->parallel=opt->parallel<1?1:opt->parallel;
	opt->objdir=opt->objdir==NULL?".":opt->objdir;
	return succ;
}
int get_default_artifact(char**out,unsigned extra)
{
	int succ=0;
	char cbuf[4096];
	if(realpath(".",cbuf)==NULL)
		succ=-1;
	else
	{
		unsigned off=0,len=1;
		for(const char*it=cbuf;*it!='\0';++len,++it)
		{
			if(*it=='/')
			{
				off=it+1-cbuf;
				len=0;
			}
		}
		*out=malloc(len+extra);
		if(*out==NULL)
			succ=-1;
		else
			memcpy(*out,cbuf+off,len);
	}
	return succ;
}
