#include<dirent.h>
#include<fcntl.h>
#include<linux/limits.h>
#include<stddef.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include"utils.h"
void iterate_directory(const char*dirname,directory_iterator_callback_t func,void*arg)
{
	DIR *dirhand=opendir(dirname);
	if(dirhand==NULL)
	{
		fprintf(stderr,"Opening %s",dirname);
		perror(" failed");
	}
	else
	{
		char currdir[PATH_MAX];
		DIR*handarr[128];
		struct dirent*en,*enarr[128];
		char boolval[128];
		unsigned depth=1;
		size_t namelen,dirlen=strlen(dirname);
		memcpy(currdir,dirname,dirlen+1);
		handarr[0]=dirhand;
		boolval[0]=0;
		while(depth>0)
		{
			en=enarr[depth-1]=readdir(handarr[depth-1]);
			if(en==NULL)
			{
				boolval[depth-1]|=func(currdir,arg,boolval[depth-1]<<2|3);
				if(depth>1)
				{
					boolval[depth-2]|=boolval[depth-1];
				}
				for(;dirlen>0&&currdir[dirlen]!='/';--dirlen);
				currdir[dirlen]='\0';
				closedir(handarr[--depth]);
			}
			else
			{
				namelen=strlen(en->d_name);
				if(strcmp(en->d_name,".")&&strcmp(en->d_name,".."))
				{
					currdir[dirlen]='/';
					memcpy(currdir+dirlen+1,en->d_name,namelen+1);
					dirlen+=namelen+1;
					if(en->d_type==DT_DIR)
					{
						handarr[depth]=opendir(currdir);
						if(handarr[depth]==NULL)
						{
							fprintf(stderr,"Opening %s",currdir);
							perror(" failed");
							for(;dirlen>0&&currdir[dirlen]!='/';--dirlen);
							currdir[dirlen]='\0';
						}
						else
						{
							boolval[depth-1]|=func(currdir,arg,1);
							boolval[depth++]=0;
						}
					}
					else
					{
						boolval[depth-1]|=func(currdir,arg,0);
						for(;dirlen>0&&currdir[dirlen]!='/';--dirlen);
						currdir[dirlen]='\0';
					}
				}
			}
		}
	}
}
int runprogram(unsigned char maxi,char*const*args)
{
	static unsigned char running=0;
	int fds[2];
	int status,succ=-1;
	char ch=13;
	const char*program=args[0];
	int pid=fork();
	if(pipe(fds)<0)
		perror("pipe failed");
	else if(pid>0)
	{
		close(fds[1]);
		if(read(fds[0],&ch,sizeof ch)<=0)
		{
			++running;
			succ=0;
		}
		close(fds[0]);
		if(running==maxi)
		{
			wait(&status);
			--running;
			for(;waitpid(-1,&status,WNOHANG)>0;--running);
		}
	}
	else if(pid<0)
		perror("fork failed");
	else
	{
		close(fds[0]);
		fcntl(fds[1],F_SETFD,FD_CLOEXEC);
		if(execvp(program, args)==-1)
		{
			fprintf(stderr,"Running program %s",program);
			perror(" failed");
			write(fds[1],&ch,sizeof ch);
			close(fds[1]);
			exit(0);
		}
	}
	return succ;
}
int program_output(struct vector_char*data,char*const*args)
{
	int fds[2];
	int failed=pipe(fds);
	if(!failed)
	{
		int pid=fork();
		if(pid>0)
		{
			int status;
			int finished=0;
			char buf[8192];
			size_t buflen=sizeof(buf);
			close(fds[1]);
			for(size_t cnt=read(fds[0],buf,buflen);!failed&&cnt>0;cnt=read(fds[0],buf,buflen))
			{
				failed=push_vector_char(data,buf,buf+cnt);
			}
			for(;finished!=-1&&finished!=pid;finished=wait(&status));
			if(finished!=pid)
			{
				failed=1;
				perror("program_output failed: wait failed");
			}
			close(fds[0]);
			failed=WEXITSTATUS(status);
		}
		else if(pid<0)
		{
			failed=1;
			perror("program_output failed: fork failed");
		}
		else
		{
			dup2(fds[1],STDOUT_FILENO);
			close(fds[0]);
			close(fds[1]);
			if(execvp(args[0],args))
			{
				perror("program_output failed: execvp failed");
				failed=1;
				exit(failed);
			}
		}
	}
	return failed;
}
char strcontains(const char*strlist,const char*str)
{
	char found=0;
	for(size_t len=strlen(strlist);len>0;strlist+=len+1,len=strlen(strlist))
	{
		if(strcmp(strlist,str)==0)
			found=1;
	}
	return found;
}
char*changeext_add_prefix(const char*og,const char*prefix,const char*ext)
{
	size_t len=strlen(og);
	const char*period;
	for(period=og+len;period!=og&&period[-1]!='.'&&period[-1]!='/';--period);
	period=period==og||period[-1]=='/'?og+len:period;
	len=period-og;
	size_t extlen=strlen(ext);
	size_t plen=strlen(prefix);
	int addslash=prefix[plen-1]!='/';
	char*updated=malloc(plen+len+extlen+addslash+1);
	memcpy(updated,prefix,plen);
	if(addslash)
		updated[plen++]='/';
	return strcpy((char*)memcpy(updated+plen,og,len)+len,ext)-plen-len;
}
int init_vector_char(struct vector_char*this)
{
	int failed=1;
	this->len=0;
	this->cap=0;
	this->str=malloc(16);
	if(this->str!=NULL)
	{
		this->cap=1;
		failed=0;
	}
	return failed;
}
int push_vector_char(struct vector_char*this,const char*first,const char*last)
{
	int failed=0;
	if(this->len+(last-first)>this->cap)
	{
		unsigned nc=this->cap+(this->cap>>1);
		nc=nc>this->len+(last-first)?nc:this->len+(last-first);
		char*new=realloc(this->str,nc);
		if(new==NULL)
		{
			failed=1;
		}
		else
		{
			this->str=new;
			this->cap=nc;
		}
	}
	if(!failed)
	{
		memcpy(this->str+this->len,first,last-first);
		this->len+=last-first;
	}
	return failed;
}
void free_vector_char(struct vector_char*this)
{
	free(this->str);
}
void wait_children(void)
{
	while(wait(NULL)>0);
}
