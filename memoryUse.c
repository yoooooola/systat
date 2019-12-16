// /proc/meminfo 파일을 통해 메모리 사용량 관찰
// 커맨드 입력 시 전체 메모리의 사용량 관찰 가능
// 커맨드 입력 시 메모리 사용량을 관찰하고 싶은 프로그램을 함께 입력할 수 있다 (option)
// 입력받은 프로그램의 PID를 찾아 /proc/PID/meminfo 에 접근 및 해당 프로그램의 메모리 사용량 관찰
// 만약 조금 더 자세한 정보를 요구할 경우, top 커맨드를 구현 혹은 파이프를 통해 활용할 예정

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
//	int bufCache;
	int swapUsed;
	double nomMem; // nomial memory usage ratio
	double actMem; // actual memory usage ratio
}; // structure saves calculating result using meminfo

int checkM = 0;

void getMemory(struct meminfo *); // get memory info
void calMemory(struct meminfo , struct calinfo *); // caculate essential info using mem info
void red(); // bold red colored text
void blue(); // bold blue colored text
void reset(); // reset all text conf
void printMemory(struct meminfo, struct calinfo, char); // print memory info to screen

int main(int ac, char *av[]) {


	if(ac > 1) {
		while(--ac) {
			if(av[ac][0] == '-') { // check option
				if(av[ac][1] == 'm') checkM = 1; // option present mb
			}
		}
	}

	struct meminfo m;
	struct calinfo c;

	// get memory info
	getMemory(&m);

	// caculate using memory info
	calMemory(m, &c);

	// print memory info
	if(checkM == 1) printMemory(m, c, 'm');
	else printMemory(m, c, 'k'); // basic option is kilobyte

	return 0;
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
//	c->bufCache = (m.cached) + (m.buffers);
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
