# FP_SISOP19_C06

Kholishotul Amaliah 05111740000030<br>
Nandha Himawan      05111740000180

## FUSE dan Thread

Buatlah sebuah music player dengan bahasa C yang memiliki fitur play nama_lagu, pause, next, prev, list lagu. Selain music player juga terdapat FUSE untuk mengumpulkan semua jenis file yang berekstensi .mp3 kedalam FUSE yang tersebar pada direktori /home/user. Ketika FUSE dijalankan, direktori hasil FUSE hanya berisi file .mp3 tanpa ada direktori lain di dalamnya. Asal file tersebut bisa tersebar dari berbagai folder dan subfolder. program mp3 mengarah ke FUSE untuk memutar musik.

## FUSE

- membuat struktur data untuk menyimpan node masing-masing file berekstensi mp3. disini kami memilih linked list. setiap node akan bersi informasi path dan name file tersebut, serta prev dan next untuk mengetahui listing nya.
```c
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
```

- pada fungsi readdir, kami memodifikasi supaya dapat listing directory. yaitu dengan cara memanggil fungsi lain (fungsi iter).
```c
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
```
fungsi iter sendiri akan berisi iterasi untuk semua file dan folder dalam folder. jika file tersebut berekstensi mp3, maka masukkan ke list dan mount file tersebut ke file system. jika itu adalah direktori (folder), maka rekursif untuk menelusuri folder tersebut
```c
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
	    int length = strlen(de->d_name);
            if(length>5)
	    {
		if(de->d_name[length-4]=='.' && de->d_name[length-3]=='m' && de->d_name[length-2]=='p' && de->d_name[length-1]=='3')
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
```

- pada fungsi getattr, kami memodifikasi supaya file-file dalam file system dapat terkoneksi dengan file pada direktori aslinya. karena hanya file .mp3 saja (yang telah masuk list) yang perlu dicari atributnya, maka perlu fungsi search untuk mencocokkan antara file di file system dan file di direktori asli.
```c
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
```
fungsi search akan menyocokkan berdasarkan nama filenya. jika tidak ada, maka return empty string.
```c
char *search(char name[])
{
    char *to_return;
    to_return = malloc(sizeof(char)*1000);
    current = head;
    while(current!=tail)
    {
        if(strcmp(current->name, name)==0)
        {
            sprintf(to_return, "%s%s", current->path, current->name);
            return to_return;
        }
        current=current->next;
    }
    return "";
}
```

- pada fungsi read, kami memodifikasi supaya file-file dalam file system yang telah terkoneksi dengan file pada direktori aslinya dapat dibuka. karena hanya file .mp3 saja (yang telah masuk list) yang perlu supaya dapat diputar musiknya, maka perlu fungsi search untuk mencocokkan antara file di file system dan file di direktori asli. fungsi search sama dengan fungsi search yang dipanggil pada getattr.
```c
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
```
