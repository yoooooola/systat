/*
    /proc/meminfo 파일을 이용해 컴퓨터의 메모리 사용량을 관찰하고, user에게 필요한 정보들을 선별해 화면에 출력한다.

    option '-m'
    : 사용량을 mb 단위로 출력한다.
    (기본적으로 사용량은 kb 단위로 출력된다)
 
    option '-p [processname]'
    : 메모리 사용량을 알고 싶은 프로세스의 경우, 이름을 입력하면 이름을 기반으로 pid를 찾아 /proc/pid/statm 파일을 이용해 해당 프로세스의 메모리 사용 정보를 볼 수 있다.

    ** 두 가지 옵션에는 순서가 없다.
 
    ** -p 옵션을 개발할 때, fopen을 이용해 바로 /proc/pid/statm 파일을 열고자 했으나 segmentation 오류 발생이 있었다. 따라서 파이프를 사용해 creat() 으로 텍스트 파일에 statm 파일 내용을 cat 해 저장하고, 저장한 파일을 다시 불러와 읽는 방식을 사용했다.
 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define BUFSIZE 256

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
unsigned int getPID(); // get pid from process name
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
                    if(count+1 == ac || av[count+1][0] == '-') {
                        fprintf(stderr, "-p [processname]\n");
                        return 0;
                    }
                    else processName = av[count+1];
                }
                
            }
            count++;
        }
    }
    
    struct meminfo m;
    struct calinfo c;
    struct statminfo s;
    unsigned int pid;
    
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
        
        // get pid by process name
        pid = getPID();
        
        // get /proc/pid/statm
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
        printf("Text: %d\n", s.text);
        printf("Data(data + stack): %d\n\n", s.data);
    }
    
    else if(option == 'm') {
        printf("Size: %d\n", (s.size/BUFSIZE));
        printf("Size(not swap): %d\n", (s.resident/BUFSIZE));
        printf("Share: %d\n", (s.share/BUFSIZE));
        printf("Text: %d\n", (s.text/BUFSIZE));
        printf("Data(data + stack): %d\n\n", (s.data/BUFSIZE));
    }
    
    return;
}

void getStatm(struct statminfo *s, unsigned int pid) {
    // stores each word in meminfo file
    char buffer[BUFSIZE] = "";
    char fileName[BUFSIZE];
    
    sprintf(fileName, "/proc/%d/statm", pid);
    
    int p, fd;
    
    if((p = fork()) == -1)
        fprintf(stderr, "fork error\n");
    
    if(p == 0) {
        close(1);
        
        // create file saving /proc/pid/statm
        fd = creat("statm.txt", 0644);
        
        // save /proc/pid/statm into file
        if(execlp("cat","cat",fileName, NULL) == -1) {
            fprintf(stderr, "execlp error\n");
            return;
        }
    }
    
    if(p != 0) wait(NULL);
    
    int bin, count = 0;
    
    FILE *file = fopen("statm.txt", "r");
    
    // read the file and save into structure
    while (count < 7) {
        
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

unsigned int getPID() {
    
    char line[BUFSIZE];
    char str[BUFSIZE] = "pidof ";
    
    strcat(str, processName);
    
    // get pid by using pidof command
    FILE *f = popen(str, "r");
    
    fgets(line, BUFSIZE, f);
    
    unsigned int pid = strtoul(line, NULL, 10);
    
    pclose(f);
    
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
    printf("Nominal Memory Usage : ");
    
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
    // c->bufCache = (m.cached) + (m.buffers);
    c->swapUsed = (m.swapTotal) - (m.swapFree);
    
    // caculate nomial memory usage ratio
    c->nomMem = (double)(c->used) / (double)(m.memTotal);
    
    // for presenting by percentage
    c->nomMem = (c->nomMem) * 100;
    
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
