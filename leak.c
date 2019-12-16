// 프로그램 실행 시
// ps -u 커맨드로 현재 로그인된 pts들의 PID 가져오기
// 구한 PID로 /proc/PID/smaps 파일 복사본 만들기 ( PID_prev )
// 5초 뒤에 PID_cur 파일 복사본 만들기
// 7ffe367c2000-7ffe367c4000 -> 주소. 두개 주소앞에 0x 붙여서
// 아래 커맨드로 실행
// dump memory ./dump_outputfile.dump 0x2b3289290000 0x2b3289343000
// 비교한뒤 strings 나 hexdump -C 사용하여 dump 파일 표시하기
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define oops(m,x) { perror(m); exit(x);}

struct pro_info{
	char* name;
	int pid;
};

int main(){

	struct pro_info[100];
	int pid, fd, num_pro=0;

	fd = 0;

	// ps -u 커맨드 실행 후 파일로 저장
	if ( pid = fork() == -1){
		oops("fork error",1);
	}

	if ( pid == 0){
		close(1);
		fd = creat("p_name",0644);
		execlp("ps","ps","-u",NULL);
		oops("execlp error");
	}

	if ( pid != 0 ){
		wait(NULL);
	}

	// p_name 파일 읽기
	FILE *fp = fopen("p_name","r");
    int p_buf,idx = 0;
    char *name,*trash;
    fscanf(fp,"%s %s",name,trash)

    while (EOF != fscanf(fp,"%s %d",name,&p_buf)){
        struct pro_info *temp = &arg_arr[num_pro];
        temp->name = name;
        temp->pid = p_buf;
        num_pro++;
    }
    fclose(fp);

//-------------------------- thread starts -----------------------------
    
    // thread 생성
    pthread_t threads[num_pro];
    for ( int i = 0 ; i < num_pro ; i++ )
        pthread_create(&threads[i],NULL, make_copy,(void *)&pro_info[i]);

    for ( int i = 0 ; i < nt ; i++ )
        pthread_join(threads[i],NULL);



    // 해당하는 pid의 proc로 접속하기
    // thread 사용하기
    // /proc/pid_num/smaps 파일 읽어서
    // 파일로 저장하기


	return 0;
}

void* make_copy(void *args)
{	
	int temp = 0;
    	struct pro_info *arg = args;
	char path[30] = "./ps_file/";
	strcpy(path,arg->name);
	FILE *fp = fopen("./ps_file/"+arg->pid
        return NULL;
}

void* check_leak(void *args)
{

        return NULL;
}
