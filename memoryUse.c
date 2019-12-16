// processName 받아서 pid 찾고, 해당 pid의 statm 정보 받아서 출력하는 부분 추가했는데 실행 X 수정해야함

// /proc/meminfo 파일을 통해 메모리 사용량 관찰
// 커맨드 입력 시 전체 메모리의 사용량 관찰 가능
// 커맨드 입력 시 메모리 사용량을 관찰하고 싶은 프로그램을 함께 입력할 수 있다 (option)
// 입력받은 프로그램의 PID를 찾아 /proc/PID/meminfo 에 접근 및 해당 프로그램의 메모리 사용량 관찰
// -m (option) : 메모리 사용량 정보를 mb로 표시한다 (기본옵션 : kb)
// -p [processName] (option) : 해당 프로세스의 메모리 사용 정보를 표시한다.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> // for DIR
#include <unistd.h> // for readlink()

#define BUFSIZE 1024

struct meminfo {
    int memTotal;
    int memFree;
    int memAvailable;
    int cached;
    int buffers;
    int swapTotal;
    int swapFree;
    int sharedMem;
}; // structure saves meminfo variables

struct calinfo {
    int used;
    //      int bufCache;
    int swapUsed;
    double nomMem; // nomial memory usage ratio
    double actMem; // actual memory usage ratio
}; // structure saves calculating result using meminfo

struct statminfo {
    int size;
    int resident;
    int share;
    int text;
    int data;
}; // structure saves statm variables

int checkM = 0; // check -m option
int checkP = 0; // check there is process input or not

char *processName; // save process name input

void getMemory(struct meminfo *); // get memory info
void calMemory(struct meminfo , struct calinfo *); // caculate essential info using mem info
void red(); // bold red colored text
void blue(); // bold blue colored text
void reset(); // reset all text conf
void printMemory(struct meminfo, struct calinfo, char); // print memory info to screen
unsigned int getPID(char *); // get pid from process name
void getStatm(struct statminfo *, unsigned int); // get /proc/pid/statm
void printStatm(struct statminfo, char); // print statm info


int main(int ac, char *av[]) {
    
    int count = 1;
    
    if(ac > 1) {
        while(count < ac) {
            
            if(av[count][0] == '-') { // check option
                
                if(av[count][1] == 'm') checkM = 1; // option present mb
                
                if(av[count][1] == 'p') {
                    checkP = 1; // option provide process name as input
                    processName = av[count+1];
                }
                
                count++;
            }
        }
    }
    
    struct meminfo m;
    struct calinfo c;
    struct statminfo s;
    unsigned int pid;
    
    //      for debug
    //      printf("pName: %s, checkP: %d, checkM: %d\n", processName, checkP, checkM);
    
    if(checkP == 0) {
        
        // get memory info
        getMemory(&m);
        
        // caculate using memory info
        calMemory(m, &c);
        
        // print memory info
        if(checkM == 1) printMemory(m, c, 'm');
        else printMemory(m, c, 'k'); // basic option is kilobyte
        
        return 0;
    }
    
    else { // get process name as input
        
        pid = getPID(processName);
        printf("%d", pid);
        getStatm(&s, pid);
        
        if(checkM == 1) printStatm(s, 'm');
        else printStatm(s, 'k');
    }
    
}

void printStatm(struct statminfo s, char option) {
    
    red();
    printf("\nMemory of ");
    blue();
    printf("%s", processName);
    red();
    printf(" ↓\n\n");
    
    reset();
    if(option == 'k') {
        printf("Size: %d\n", s.size);
        printf("Size(not swap): %d\n", s.resident);
        printf("Share: %d\n", s.share);
        printf("Data(data + stack): %d\n", s.data);
    }
    
    else if(option == 'm') {
        printf("Size: %d\n", (s.size/BUFSIZE));
        printf("Size(not swap): %d\n", (s.resident/BUFSIZE));
        printf("Share: %d\n", (s.share/BUFSIZE));
        printf("Data(data + stack): %d\n", (s.data/BUFSIZE));
    }
    
    return;
}

void getStatm(struct statminfo *s, unsigned int pid) {
    // stores each word in meminfo file
    char buffer[BUFSIZE] = "";
    char *fileName;
    char processId[255];
    
    int bin;
    
    strcat(fileName, "/proc/");
    sprintf(processId, "%d", pid);
    strcat(fileName, processId);
    strcat(fileName, "/statm");
    
    
    // open file /proc/pid/statm
    FILE* file = fopen(fileName, "r");
    int count = 0;
    
    // read the file and save into structure
    while (fscanf(file, " %1023s", buffer) == 1) {
        
        if (count == 0)
            fscanf(file, "%d", &(s->size));
        
        if (count == 1)
            fscanf(file, "%d", &(s->resident));
        
        if (count == 2)
            fscanf(file, "%d", &(s->share));
        
        if (count == 3)
            fscanf(file, "%d", &(s->text));
        
        if (count == 5)
            fscanf(file, "%d", &(s->data));
        
        if (count == 4 || count == 6) fscanf(file, "%d", &bin);
        count++;
        
    }
    
    fclose(file);
    
    return;
}

unsigned int getPID(char *pName) {
    
    DIR *d;
    struct dirent *dirEntry;
    char dirName[255];
    char targetName[255];
    char exeLink[255];
    int targetPid, pid = 0;
    
    d = opendir("/proc/");
    
    while((dirEntry = readdir(d)) != NULL) {
        
        if(strspn(dirEntry->d_name, "0123456789") == strlen(dirEntry->d_name)) {
            
            // get full path of dir
            strcpy(dirName, "/proc/");
            strcat(dirName, dirEntry->d_name);
            strcat(dirName, "/");
            
            exeLink[0] = 0;
            strcat(exeLink, dirName);
            strcat(exeLink, "exe");
            targetPid = readlink(exeLink, targetName, sizeof(targetName)-1);
            
            if(targetPid > 0) {
                
                targetName[targetPid] = 0;
                
                if(strstr(targetName, pName) != NULL) {
                    
                    pid = atoi(dirEntry->d_name);
                    closedir(d);
                    
                    return pid;
                }
            }
        }
    }
    closedir(d);
    
    return pid;
    
}

void printMemory(struct meminfo m, struct calinfo c, char option) {
    
    red();
    printf("\nMemory ↓\n\n");
    
    reset();
    
    if(option == 'k') { // present with kilobyte
        printf("Total: %d \t Used: %d\n", m.memTotal, c.used);
        printf("Free: %d \t Available: %d\n\n", m.memFree, m.memAvailable);
        printf("Shared Memory: %d\n", m.sharedMem);
        printf("Buffer: %d\n", m.buffers);
        printf("Cache: %d\n\n", m.cached);
    }
    
    else if(option == 'm') { // megabyte
        printf("Total: %d \t Used: %d\n", (m.memTotal/BUFSIZE), (c.used/BUFSIZE));
        printf("Free: %d \t Available: %d\n\n", (m.memFree/BUFSIZE), (m.memAvailable/BUFSIZE));
        printf("Shared Memory: %d\n", (m.sharedMem/BUFSIZE));
        printf("Buffer: %d\n", (m.buffers/BUFSIZE));
        printf("Cache: %d\n\n", (m.cached/BUFSIZE));
    }
    
    red();
    printf("Swap ↓\n\n");
    
    reset();
    
    if(option == 'k')
        printf("Total: %d\n Used: %d\n Free: %d\n\n", m.swapTotal, c.swapUsed, m.swapFree);
    else if(option == 'm')
        printf("Total: %d\n Used: %d\n Free: %d\n\n", (m.swapTotal/BUFSIZE), (c.swapUsed/BUFSIZE),(m.swapFree/BUFSIZE));
    
    blue();
    printf("Nomial Memory Usage : ");
    
    reset();
    printf("%.2f%%\n", c.nomMem);
    
    blue();
    printf("Actual Memory Usage : ");
    
    reset();
    printf(" %.2f%%\n\n", c.actMem);
    
    return;
}

void red() {
    printf("\033[1;31m");
}

void blue() {
    printf("\033[1;34m");
}

void reset() {
    printf("\033[0m");
}

void calMemory(struct meminfo m, struct calinfo *c) {
    
    // used = total - free
    c->used = (m.memTotal) - (m.memFree);
    //      c->bufCache = (m.cached) + (m.buffers);
    c->swapUsed = (m.swapTotal) - (m.swapFree);
    
    // caculate nomial memory usage ratio
    c->nomMem = (double)(c->used) / (double)(m.memTotal);
    c->nomMem = (c->nomMem) * 100; // for presenting by percentage
    
    // caculate actual memory usage ratio
    c->actMem = (double)((m.memTotal) - (m.memAvailable)) / (double)(m.memTotal);
    c->actMem = (c->actMem) * 100;
    
    return;
}

void getMemory(struct meminfo* m) {
    
    // stores each word in meminfo file
    char buffer[BUFSIZE] = "";
    
    // open file contains memory info
    FILE* file = fopen("/proc/meminfo", "r");
    
    // read the file and save into structure
    while (fscanf(file, " %1023s", buffer) == 1) {
        
        if (strcmp(buffer, "MemTotal:") == 0)
            fscanf(file, " %d", &(m->memTotal));
        
        if (strcmp(buffer, "MemFree:") == 0)
            fscanf(file, " %d", &(m->memFree));
        
        if (strcmp(buffer, "MemAvailable:") == 0)
            fscanf(file, " %d", &(m->memAvailable));
        
        if (strcmp(buffer, "SwapTotal:") == 0)
            fscanf(file, " %d", &(m->swapTotal));
        
        if (strcmp(buffer, "SwapFree:") == 0)
            fscanf(file, " %d", &(m->swapFree));
        
        if (strcmp(buffer, "Cached:") == 0)
            fscanf(file, " %d", &(m->cached));
        
        if (strcmp(buffer, "Buffers:") == 0)
            fscanf(file, " %d", &(m->buffers));
        
        if (strcmp(buffer, "Shmem:") == 0)
            fscanf(file, " %d", &(m->sharedMem));
    }
    
    fclose(file);
    
    return;
}
