// 프로그램 실행 시
// who 커맨드 구현으로 현재 로그인된 pts들의 PID 가져오기
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
#define oops(m,x) { perror(m); exit(x);}

int main(void){
	char* line[100];
        int idx = 0;
	char* cmd[2];
	char* options[3];
	int thepipe[2],pid;
        for (int i=0; i<2; ++i){
                cmd[i] = (char *)malloc(sizeof(char)*20);
        }
	

	cmd[0] = "ps";
	options[0] = "-u";
	cmd[1] = "grep";
	options[1] = "-v";
	options[2] = "bash";

        if ( pipe(thepipe) == -1) // open pipe
                oops("Cannot get a pipe",1);

        if ((pid = fork()) == -1) //make 1 child process
                oops("Cannot fork",2);

        // 부모 프로세스일 경우
        if ( pid > 0){
                close(thepipe[1]);

                if ( dup2(thepipe[0],0) == -1 ) 
			oops("could not redirect stdin",3);
			
		close(thepipe[0]); 
               	int txtfd = creat("p_name",0644);
		execlp(cmd[1],cmd[1],options[1],options[2],NULL);
		oops(cmd[1],4);
		
		/*
		// 메모리 할당
		for ( int i = 0 ; i < 100 ; ++i ){
			line[i] = (char *)malloc(sizeof(char)*100);
		}
	
		while ( NULL != fgets(line[idx],100,stdin)){
			printf("%s",line[idx]);
			idx++;	
		}
		
		//Redirect
		int txtfd = open("p_name.txt",O_RDWR|O_CREAT,S_IRWXU);
		if ( txtfd == -1 )
			oops("couldn't create txt.",4);
		for ( int i = 0 ; i < idx ; i++ ){
			write(txtfd,line[i],sizeof(line[i]));
		}*/
        }

        // 자식 프로세스 일경우
        close(thepipe[0]); 

        if (dup2(thepipe[1],1) == -1) 
                oops("could not redirect stdout",4);

        close(thepipe[1]);
        execlp(cmd[0],cmd[0],options[0],NULL); 
	oops(cmd[0],5);

	return 0;
}
