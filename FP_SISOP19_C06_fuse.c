#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

typedef struct lis
{
    char path[200];
    char name[200];
    struct lis *prev;
    struct lis *next;
}list;

list *head;
list *tail;
list *current;
list *temp;
static const char *dirpath = "/home/maya";

char *search(char name[])
{
    char *to_return;
    to_return = malloc(sizeof(char)*1000);
    current = head;
    while(current!=tail)
    {
        if(current->name == name)
        {
            sprintf(to_return, "%s%s", current->path, current->name);
            return to_return;
        }
        if(strcmp(current->name, name)==0)
        {
            sprintf(to_return, "%s%s", current->path, current->name);
            return to_return;
        }
        current=current->next;
    }
    return "";
}

void insert(char path[], char name[])
{
    current = (list*)malloc(sizeof(list));
    sprintf(current->path, "%s", path);
    sprintf(current->name, "/%s", name);
    if(tail==NULL)
    {
		current->next=NULL;
        current->prev=NULL;
		head = current;
		tail = current;
    }
    else
    {
		temp = tail;
		current->next = NULL;
		temp->next = current;
		current->prev = temp;
		tail = current;
    }
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
    int res;
	char fpath[1000];
    char *thisPath;
	sprintf(fpath,"%s%s",dirpath, path);

    char search_path[1000];
    sprintf(search_path, "%s", path);
    thisPath = search(search_path);

    if(strcmp(path, "/")==0)
    {
        thisPath=fpath;
    }

	res = lstat(thisPath, stbuf);

	if (res == -1)
		return -errno;

	return 0;
}

int iter(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	if(strcmp(path, "/home/maya/mount")==0)
	 	return 0;
	char fpath[1000];
    sprintf(fpath, "%s", path);

	DIR *dp;
	struct dirent *de;
    char thePath[500];
	int res;

	(void) offset;
	(void) fi;

	dp = opendir(path);

	while ((de = readdir(dp)) != NULL)
    {
        if(de->d_type != DT_DIR)
        {
            if(strstr(de->d_name, ".mp3"))
            {
                insert(fpath, de->d_name);
				struct stat st;
				memset(&st, 0, sizeof(st));
				st.st_ino = de->d_ino;
				st.st_mode = de->d_type << 12;
				if(filler(buf, de->d_name, &st, 0))
					break;
            }
        }
        else
        {
            sprintf(thePath, "%s/%s", path, de->d_name);
            if(strcmp(de->d_name, ".")==0 || strcmp(de->d_name, "..")==0 || de->d_name[0] == '.')
				continue;
            else res = iter(thePath, buf, filler, offset, fi);;
        }
        
	}
	closedir(dp);
    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res = 0;

	DIR *dp;

	(void) offset;
	(void) fi;

	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

	res = iter(fpath, buf, filler, offset, fi);

	closedir(dp);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
    int res;
	char fpath[1000];
    char *thisPath;
	sprintf(fpath,"%s%s",dirpath, path);

    int fd = 0 ;

	(void) fi;

    char search_path[1000];
    sprintf(search_path, "%s", path);
    thisPath = search(search_path);

    if(strcmp(path, "/")==0)
    {
        thisPath=fpath;
    }

	fd = open(thisPath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static void *pre_init(struct fuse_conn_info *conn)
{
    head=tail=NULL;
    return NULL;
}

static struct fuse_operations xmp_oper = {
	.init		= pre_init,
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
};

int main(int argc, char *argv[])
{
    umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}