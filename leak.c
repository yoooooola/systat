/*
        먼저 pipe를 통해 ps -u 커맨드로 현재 로그인된 pts들의 PID들을 저장>한다.
        
        사용자가 ./leak -p로 접근했을때는 현재 활성화된 PID들의 smaps파일을
 저장한다.
        이때, 각 PID에 대한 smaps 파일의 저장은 pthread를 사용한다.
        smaps 파일들은 ps_file 폴더내에 PID_prev 형태로 저장된다.
        
        일정시간동안 사용자가 메모리를 사용하거나, leak이 발생하는 시간까지
 기다린후에
        ./leak -c로 접근하여 현재 활성화된 PID들을 다시 저장한뒤,
        그 PID들에 대한 smaps 파일을 저장한다.
        이때 smaps 파일들은 ps_file폴더내에 PID_curr 형태로 저장된다.
        
        그리고 만들어진 PID_prev와 PID_curr 파일을 pipe와 pthread를 사용하>여
        서로 파일의 차이점을 찾는다. 이때 diff 커맨드를 파이프를 통해 사용>한뒤
        결과를 ps_file 폴더내에 result_PID 형태로 저장한다.
        
        최종적으로 ./leak -c를 실행했을떄 ps -u에 활성화되있던 모든 PID들에
 대해서
        smaps파일의 변화유무를 보여준다.
        < : prev 파일정보
        > : curr 파일정보

        메모리 변화를 통해서 어떤 프로세스에서 leak이 발생했는지 확인한다.
*/

#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <pthread.h>
#include <errno.h>

#define oops(m,x) { perror(m); exit(x);}
#define here(x) { return x;}
#define MAX_LEN 100

int make_copy(int pid, char* when);
void* first_check(void *args);
void* check_leak(void *args);
int print_result(int pid);

struct pro_info{
        char* name;
        int pid;
};

static char dirname[50];

int main(int argc, char *argv[]){

        srand(time(NULL));
        int random = rand()%10000;
        sprintf(dirname,"ps_file");

        struct pro_info pro[100];
        int pid,fd,num_pro=0;
        int flags = 999;
        if ( argc != 2 ){
                fprintf(stderr,"usage : ./leak [option]. options are -p or -c");
        }

        fd = 0;
        if ( strcmp(argv[1],"-p") == 0)
                flags = 0;
        else if (strcmp(argv[1], "-c") == 0 )
                flags = 1;
        else{
                fprintf(stderr,"Please type option.");
                exit(1);
        }

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

// p_name 파일 읽기 / 구조체에 저장
        FILE *fp = fopen("p_name.txt","r");
        int p_buf;
        char *name = "";
        name = malloc(sizeof(char *)*100);
        char* trash[12];
        char temp;
        for ( int i = 0 ; i < 12 ; i++ ){
                trash[i] = malloc(sizeof(char *)*100);
        }

        fscanf(fp,"%s %s %s %s %s %s %s %s %s %s %s %c",trash[0],trash[1],trash[2],trash[3],trash[4],trash[5],trash[6],trash[7],trash[8],trash[9],trash[10],&temp);

        int prev = 0;
        while (EOF != fscanf(fp,"%s %d %s %s %s %s %s %s %s %s %s %c",trash[0],&p_buf,trash[1],trash[2],trash[3],trash[4],trash[5],trash[6],trash[7],trash[8],name,&temp)){
                if ( prev == p_buf ) continue;
                struct pro_info *temp = &pro[num_pro];
                temp->name = name;
                temp->pid = p_buf;
                //printf("%s %d\n",temp->name,temp->pid);
                num_pro++;
                prev = p_buf;
                //if ( temp != '\n') fscanf(fp,"%s",trash[0]);
         }
        fclose(fp);

//-------------------------- thread starts -----------------------------
/* // non thread version
        int idx = 0;
        
        if ( flags == 0){
        for ( int i = 0 ; i < num_pro ; i++ ){
                idx = 0;
                struct pro_info *temp = &pro[i];
                make_copy(temp->pid,"_prev");
        }       
        }
        else if (flags == 1){
        printf("Analyzing....\n");
        sleep(5);
        for ( int i = 0 ; i < num_pro ; i++ ){
                struct pro_info *temp = &pro[i];
                make_copy(temp->pid,"_curr");
        }
        }
*/
        // thread 생성
        void *tret = NULL;
        pthread_t threads[num_pro];

        if ( flags == 0 ){
        for ( int i = 0 ; i < num_pro ; i++ )
                pthread_create(&threads[i],NULL, first_check,(void *)&pro[i]);

         for ( int i = 0 ; i < num_pro ; i++ )
                pthread_join(threads[i],&tret);

                printf("%c[1;36m",27);
                printf("prev 파일 save 완료...\n");
                printf("%c[0m",27);
        }
        // ./leak -c에 대한 실행
        else if ( flags == 1){
                for ( int i = 0 ; i < num_pro ; i++ )
                        pthread_create(&threads[i],NULL, check_leak,(void *)&pro[i]);

                for ( int i = 0 ; i < num_pro ; i++ )
                        pthread_join(threads[i],&tret);

                printf("%c[1;36m",27);
                printf("curr 파일 save 완료...\n");
                printf("%c[0m",27);
// 결과 출력
                printf("%c[1;37m",27);
                printf("\nthese are diffrence of memory. the blank line is no difference");
                printf("\n# of process is %d\n",num_pro);
                printf("%c[0m",27);

                for ( int i = 0 ; i < num_pro ; i++ ){
                        struct pro_info *temp = &pro[i];
                        //printf("%s %d\n",temp->name,temp->pid);
                        printf("\n-----------------------------------------------------------------\n");
                        printf("%c[1;35m",27);
                        printf("\n                          pid : %d\n",temp->pid);
                        printf("%c[0m",27);
                        print_result(temp->pid);
                        if ( i == num_pro-1 )
                                printf("\n-----------------------------------------------------------------\n");

                }

        }
        free(name);
        for ( int i = 0 ; i < 11 ; i++ ){
                free(trash[i]);
        }
        return 0;
}

//./leak -p에 대한 쓰레드 함수
void* first_check(void *args)
{
        struct pro_info *arg = args;
        static int retval = 999;
        int a = make_copy(arg->pid,"_prev");
        if ( a == 9 ) pthread_exit((void *) &retval);
        return NULL;
}

void* check_leak(void *args)
{
        // 5초 후 파일의 변화 탐지하기
        int pid,fd;
        static int retval = 999;
        struct pro_info *arg = args;
        char path2[50];
        sprintf(path2,"./%s/%d%s",dirname,arg->pid,"_prev");

        char path1[50];
        sprintf(path1,"./%s/%d%s",dirname,arg->pid,"_curr");

        // 두번째 copy 만들기
        int a = make_copy(arg->pid,"_curr");
        if ( a == 9 ) pthread_exit((void *) &retval);
        if (( pid = fork()) == -1){
                oops("fork error",1);
        }

        if ( pid == 0){
                close(1);
                char name[50];
                sprintf(name,"./%s/result_%d",dirname,arg->pid);
                fd = creat(name,0644);
                // 차이가 뭔가요?
                if ( execlp("diff","diff",path2,path1,NULL) == -1 ){
                        perror("이미 종료된 프로세스입니다.\n");
                        pthread_exit((void*) &retval);
                }
        }
        if ( pid != 0 ){
                wait(NULL);
        }


        return NULL;
}

// smaps 파일 복사 함수
int make_copy(int pid,char* when){
//char line[100];
        //int idx = 0;
        //struct pro_info *arg = args;
        
        // /proc/pid_num/smaps
        char path[50];
        sprintf(path,"/proc/%d/smaps",pid);
        
        // ./ps_file/pid_num_prev
        char path2[50];
        sprintf(path2,"./%s/%d%s",dirname,pid,when);
        

        int p,fd;
        if ( (p = fork()) == -1){
                oops("fork error",1);
        }       
        
        if ( p == 0 ){
                close(1);
                fd = creat(path2,0644);
                if ( execlp("cat","cat",path,NULL) == -1){
                        perror("이미 종료된 프로세스입니다.\n");
                        return 9;
                }       
        }       
        
        if ( p != 0 ){
                wait(NULL);
        }       
        return 0;
}       

// 결과 출력 함수
int print_result(int pid){

        char path[50];
        sprintf(path,"ps_file/result_%d",pid);
        printf("\n");
        int p,fd;
        if ( (p = fork()) == -1){
                oops("fork error",1);
        }

        if ( p == 0 ){
                execlp("cat","cat",path,NULL);
                oops("exec",1);
        }
        if ( p != 0){
                wait(NULL);
        }

        printf("\n");
        printf("\n");
        return 0;
}