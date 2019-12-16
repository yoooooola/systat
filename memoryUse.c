// /proc/meminfo 파일을 통해 메모리 사용량 관찰
// 커맨드 입력 시 전체 메모리의 사용량 관찰 가능
// 커맨드 입력 시 메모리 사용량을 관찰하고 싶은 프로그램을 함께 입력할 수 있다 (option)
// 입력받은 프로그램의 PID를 찾아 /proc/PID/meminfo 에 접근 및 해당 프로그램의 메모리 사용량 관찰
// 만약 조금 더 자세한 정보를 요구할 경우, top 커맨드를 구현 혹은 파이프를 통해 활용할 예정

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void getMemory(int* currRealMem, int* peakRealMem,
    int* currVirtMem, int* peakVirtMem) {

    // stores each word in meminfo file
    char buffer[1024] = "";

    // linux file contains memory info
    FILE* file = fopen("/proc/meminfo", "r");

    // read the entire file
    while (fscanf(file, " %1023s", buffer) == 1) {

        if (strcmp(buffer, "MemTotal:") == 0) {
            fscanf(file, " %d", currRealMem);
        }
        if (strcmp(buffer, "MemFree:") == 0) {
            fscanf(file, " %d", peakRealMem);
        }
        if (strcmp(buffer, "MemAvailable:") == 0) {
            fscanf(file, " %d", currVirtMem);
        }
        if (strcmp(buffer, "Cached:") == 0) {
            fscanf(file, " %d", peakVirtMem);
        }
    }
    fclose(file);
}

int main() {
	int currRealMem, peakRealMem, currVirtMem, peakVirtMem;

	getMemory(&currRealMem, &peakRealMem, &currVirtMem, &peakVirtMem);

	printf("MemTotal : %d\n", currRealMem);
	printf("MemFree : %d\n", peakRealMem);
	printf("MemAvailable : %d\n", currVirtMem);
	printf("Cached : %d\n", peakVirtMem);

	return 0;
}
