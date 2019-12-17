#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define MEGABYTE 1024*1024

int main(){
        struct timeval tv;
        char *current_data;

        while(1){
                gettimeofday(&tv,NULL);
                current_data = (char *)malloc(MEGABYTE);
                sprintf(current_data,"%d",tv.tv_usec);
                printf("current_data = %s\n",current_data);
                sleep(1);
        }
        exit(0);
}
