# FP_SISOP19_C06

Kholishotul Amaliah 05111740000030
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
```

- 
