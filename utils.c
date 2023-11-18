#include<dirent.h>
#include<fcntl.h>
#include<linux/limits.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
void iterate_directory(const char *dirname, void(*func)(const char*, void*), void *arg)
{
	DIR *dirhand = opendir(dirname);
	if(dirhand == NULL)
	{
		fprintf(stderr, "Opening %s", dirname);
		perror(" failed");
	}
	else
	{
		char currdir[PATH_MAX];
		DIR *handarr[128];
		struct dirent *en, *enarr[128];
		unsigned depth = 1;
		size_t namelen, dirlen = strlen(dirname);
		memcpy(currdir, dirname, dirlen + 1);
		handarr[0] = dirhand;
		while(depth > 0)
		{
			en = enarr[depth - 1] = readdir(handarr[depth - 1]);
			if(en == NULL)
			{
				for(; dirlen > 0 && currdir[dirlen] != '/'; --dirlen);
				currdir[dirlen] = '\0';
				closedir(handarr[--depth]);
			}
			else
			{
				namelen = strlen(en->d_name);
				if(strcmp(en->d_name, ".") && strcmp(en->d_name, ".."))
				{
					currdir[dirlen] = '/';
					memcpy(currdir + dirlen + 1, en->d_name, namelen + 1);
					dirlen += namelen + 1;
					if(en->d_type == DT_DIR)
					{
						handarr[depth] = opendir(currdir);
						if(handarr[depth] == NULL)
						{
							fprintf(stderr, "Opening %s", currdir);
							perror(" failed");
							for(; dirlen > 0 && currdir[dirlen] != '/'; --dirlen);
							currdir[dirlen] = '\0';
						}
						else
							++depth;
					}
					else
					{
						func(currdir, arg);
						for(; dirlen > 0 && currdir[dirlen] != '/'; --dirlen);
						currdir[dirlen] = '\0';
					}
				}
			}
		}
	}
}
int runprogram(char*const*args)
{
	int fds[2];
	int status, succ = -1;
	char ch = 13;
	const char*program=args[0];
	int pid = fork();
	if(pipe(fds) < 0)
		perror("pipe failed");
	else if(pid > 0)
	{
		close(fds[1]);
		waitpid(pid, &status, 0);
		if(read(fds[0], &ch, sizeof ch) <= 0)
		{
			close(fds[0]);
			succ = WEXITSTATUS(status);
		}
	}
	else if(pid < 0)
		perror("fork failed");
	else
	{
		close(fds[0]);
		fcntl(fds[1], F_SETFD, FD_CLOEXEC);
		if(execvp(program, args) == -1)
		{
			fprintf(stderr, "Running program %s", program);
			perror(" failed");
			write(fds[1], &ch, sizeof ch);
			close(fds[1]);
			exit(0);
		}
	}
	return succ;
}
char strcontains(const char *strlist, const char *str)
{
	char found = 0;
	for(size_t len = strlen(strlist); len > 0; strlist += len + 1, len = strlen(strlist))
	{
		if(strcmp(strlist, str) == 0)
			found = 1;
	}
	return found;
}
char *changeext(const char *og, const char *ext)
{
	const char *period = strrchr(og, '.');
	period = period == NULL ? og : period;
	size_t len = period - og, extlen = strlen(ext);
	return strcpy((char*)memcpy(malloc(len + extlen + 2), og, len + 1) + len + 1, ext) - len - 1;
}
