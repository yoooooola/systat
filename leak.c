// 프로그램 실행 시
// ps -u 커맨드로 현재 로그인된 pts들의 PID 가져오기
// 구한 PID로 /proc/PID/smaps 파일 복사본 만들기 ( PID_prev )
// 5초 뒤에 PID_cur 파일 복사본 만들기
// 7ffe367c2000-7ffe367c4000 -> 주소. 두개 주소앞에 0x 붙여서
// 아래 커맨드로 실행
// dump memory ./dump_outputfile.dump 0x2b3289290000 0x2b3289343000
// 비교한뒤 strings 나 hexdump -C 사용하여 dump 파일 표시하기
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <pthread.h>
#define oops(m,x) { perror(m); exit(x);}
#define MAX_LEN 100

void make_copy(int pid, char* when);
void* first_check(void *args);
void* check_leak(void *args);

struct pro_info{
	char* name;
	int pid;
};

int main(){

	struct pro_info pro[100];
	int pid,fd,num_pro=0;

	fd = 0;

	// ps -u 커맨드 실행 후 파일로 저장
	if ( (pid = fork()) == -1){
		oops("fork error",1);
	}

	if ( pid == 0){
		close(1);
		fd = creat("p_name.txt",0644);
		execlp("ps","ps","-u",NULL);
		oops("execlp error",2);
	}

	if ( pid != 0 ){
		wait(NULL);
	}

// p_name 파일 읽기
    FILE *fp = fopen("p_name.txt","r");
    int p_buf;
    char *name = "";
    name = malloc(sizeof(char *)*100);
    char* trash[12];
    char temp;
    for ( int i = 0 ; i < 12 ; i++ ){
	trash[i] = malloc(sizeof(char *)*100);
    }
	    
    fscanf(fp,"%s %s %s %s %s %s %s %s %s %s %s %c",trash[0],trash[1],trash[2],trash[3],
	trash[4],trash[5],trash[6],trash[7],trash[8],trash[9],trash[10],&temp);

    int prev = 0;
    while (EOF != fscanf(fp,"%s %d %s %s %s %s %s %s %s %s %s %c",trash[0],&p_buf,trash[1],
	trash[2],trash[3],trash[4],trash[5],trash[6],trash[7],trash[8],name,&temp)){
        if ( prev == p_buf ) continue;
	struct pro_info *temp = &pro[num_pro]; 
        temp->name = name;
        temp->pid = p_buf;
        printf("%s %d\n",temp->name,temp->pid);
	num_pro++;
	prev = p_buf;
	//if ( temp != '\n') fscanf(fp,"%s",trash[0]);
    }
    fclose(fp);

//-------------------------- thread starts -----------------------------
   
    // thread 생성
    pthread_t threads[num_pro];
    for ( int i = 0 ; i < num_pro ; i++ )
        pthread_create(&threads[i],NULL, first_check,(void *)&pro[i]);
    
    for ( int i = 0 ; i < num_pro ; i++ )
       	pthread_join(threads[i],NULL);

	sleep(5);

    for ( int i = 0 ; i < num_pro ; i++ )
        pthread_create(&threads[i],NULL, check_leak,(void *)&pro[i]);

    for ( int i = 0 ; i < num_pro ; i++ )
        pthread_join(threads[i],NULL);
    
	return 0;
}

void* first_check(void *args)
{	
	struct pro_info *arg = args;
	make_copy(arg->pid,"_prev");
	return NULL;
}

void* check_leak(void *args)
{
	// 5초 후 파일의 변화 탐지하기
	int pid;
	struct pro_info *arg = args;
        char path2[50];
        sprintf(path2,"./ps_file/%d%s",arg->pid,"_prev");

	char path1[50];
	sprintf(path1,"./ps_file/%d%s",arg->pid,"_curr");

	// 두번째 copy 만들기
        make_copy(arg->pid,"_curr");
	if (( pid = fork()) == -1){
                oops("fork error",1);
        }

        if ( pid == 0){
                close(1);
		//fd = creat("result",0644);
                // 차이가 뭔가요?
		execlp("diff","diff","-u",path1,path2,NULL);
                oops("execlp error",2);
        }

        if ( pid != 0 ){
                wait(NULL);
        }


        return NULL;
}

void make_copy(int pid,char* when){
	char* line;
        int idx = 0;
        //struct pro_info *arg = args;

        // /proc/pid_num/smaps
        char path[50];
	sprintf(path,"/proc/%d/smaps",pid);

        // ./ps_file/pid_num_prev
        char path2[50];
	sprintf(path2,"./ps_file/%d%s",pid,when);

        // 해당하는 pid의 proc로 접속하기
        // /proc/pid_num/smaps 오픈 경로
        FILE *fp1 = fopen(path,"r");
        // 원본 파일 복사해서 쓸 경로
        FILE *fp2 = fopen(path2,"w");

        // 임시 버퍼
        line = (char *)malloc(sizeof(char)*MAX_LEN);

        // 받은거 바로 복사
        while ( NULL != fgets(line,MAX_LEN,fp1)){
            printf("%s",line);
            idx++;
            fputs(line,fp2);
        }
       // close(fp1);
       // close(fp2);
}
