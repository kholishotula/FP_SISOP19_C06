#include <ao/ao.h>
#include <mpg123.h>
#include <stdio.h>
#include <string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include <stdbool.h>
#include <dirent.h>

#define BITS 8
pthread_t tid1,tid2;
char filenam[256];
int flag = 0; int playable = 0;
int idx;
char filelist[1000][256];

void makelist();

void mp3player(void *arg)
{   
    chdir("/home/nandha/Downloads/LAGU/");
    while(1){
    if (playable == 0){
        sleep(1);
    }
    else
    {
    mpg123_handle *mh;
    unsigned char *buffer;
    size_t buffer_size;
    size_t done;
    int err;

    int driver;
    ao_device *dev;

    ao_sample_format format;
    int channels, encoding;
    long rate;

    ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    buffer_size = mpg123_outblock(mh);
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

    mpg123_open(mh, filenam);
    mpg123_getformat(mh, &rate, &channels, &encoding);

    format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;
    dev = ao_open_live(driver, &format, NULL);
    while(1){
        while(flag==0){
             mpg123_read(mh, buffer, buffer_size, &done);
             ao_play(dev, buffer, done); 
        }
        if(playable == 0)
        break;

        sleep(1);
    }
}
}
}
int findidx(); 
void stop(){
        flag = 1;
        playable = 0;
}
void start(){
        flag = 0;
        playable = 1;
}
void* komando(void* arg){
    char cmd[100];
    while(1){
    scanf("%s", cmd);
    if(strcmp(cmd, "pause")==0)
        {
            flag = 1;
        }
    else if(strcmp(cmd,"resume")==0)
        {   
            flag = 0;
        }
    else if(strcmp(cmd,"play")==0)
        {   
            playable = 0; char temp;
            flag = 0;
            //getchar();
            scanf("%c",&temp);
            scanf("%[^\n]", filenam);
            playable = 1;
            
        }
    else if(strcmp(cmd,"list")==0){
        int i = 0;
        makelist();
        while(i<idx){
            printf("%d. ",i+1);
            printf("%s\n", filelist[i]);
            i++;
        }
    }
    else if(strcmp(cmd,"stop")==0){
        stop();
    }
    else if(strcmp(cmd,"next")==0){
        stop();
        int now = findidx();
        if(now==idx-1)
        now = -1;
        char temp[256];
        strcpy(temp, filelist[now+1]);
        strcpy(filenam, temp);
        printf("%s\n", filenam);
        start();

    }
    else if(strcmp(cmd,"prev")==0){
        stop();
        int now = findidx();
        if(now==0)
        now=idx;
        char temp[256];
        strcpy(temp, filelist[now-1]);
        strcpy(filenam, temp);
        printf("%s\n", filenam);
        start();
    }
    }

}

void makelist()
{
    idx = 0;
    DIR *d;
    struct dirent *dir; 
    d = opendir("/home/nandha/Downloads/LAGU");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {   
            char temp[256];
            if(strcmp(dir->d_name,".")==0 || strcmp(dir->d_name,"..")==0)
                continue;
            else{
                  strcpy(filelist[idx],dir->d_name);
                  idx++;
            }

        }
        closedir(d);
    }
}
int main()
{   
    pthread_create(&(tid2), NULL, komando, NULL);
    pthread_create(&(tid1), NULL, mp3player, NULL);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    return 0;
}
int findidx(){
    int i;
    for(i = 0 ; i<idx ; i++ )
    {
        if(strcmp(filenam, filelist[i])==0)
        return i;
    }
}
//gcc -O2 -pthread -o fp fp.c -lmpg123 -lao
