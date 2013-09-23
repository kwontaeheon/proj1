#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>
typedef int BOOL;
#define FALSE 0
#define TRUE 1
#define NUM_MAX_LENGTH 50
#define MSG(a...) fprintf(stderr, a)
#define MAX_FORK_CNT 5
#define ERR_INV_PERCENT 1
#define ERR_INV_PROCNUM 2
#define ERR_INV_FORMAT 3
#define ERR_FAILED_EXEC 4
int isBlocked = FALSE;
char filename[9] = "20092318\0";
extern int opterr, optind;
extern char *optarg;
extern int optopt;
// 세마포어 
union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
};
static int semid;//semaphore id
void hdl_parent(int signal);
void hdl_child (int signal);
BOOL isCorrectInt (char *pstr);
void prate_gen(int percent, int procCnt, BOOL desc);
BOOL pmanipulator (int percent);

int main(int argc, char **argv, char **env) {
	int param_opt, first_op, num, errcode = 0;
	int percent=-1, procCnt=-1;
	BOOL desc = FALSE;
	BOOL correct_stmt = TRUE;
	opterr = 0;

	if (argc < 4)
		correct_stmt = FALSE;
	while( -1 != (param_opt = getopt (argc, argv, "p:d")) 
			&& correct_stmt) 
	{
		switch (param_opt)
		{
			case 'p':
				first_op = optind-1;
				if (optind != 3) {
					errcode = ERR_INV_PERCENT;
					correct_stmt = FALSE;
				}
				else if (isCorrectInt (argv[ first_op ]))
				{
					num = atoi (argv[first_op]);
					if (!(num > 0 && num < 100)) {
						errcode = ERR_INV_PERCENT;
						correct_stmt = FALSE;
					}
				}
				else
				{
					errcode = ERR_INV_PERCENT;
					correct_stmt = FALSE;
				}
				if (strlen (argv[ first_op-1 ]) != 2 ) {
					errcode = ERR_INV_PERCENT;
					correct_stmt = FALSE;
				}
				if (correct_stmt) 
					percent = atoi (argv[ first_op ]);
				break;
			case 'd':
				if (optind != 4) {
					errcode = ERR_INV_PERCENT;
					correct_stmt = FALSE;
				}
				first_op = optind;
				if (strlen (argv[ first_op-1 ]) != 2 ) {
					errcode = ERR_INV_PERCENT;
					correct_stmt = FALSE;
				}
				desc = TRUE;
				break;
			default :
				errcode = ERR_INV_FORMAT;
				correct_stmt = FALSE;
		}
	}
	if (correct_stmt == TRUE) {
		if ( !isCorrectInt(argv[optind]) ) {
			errcode = ERR_INV_FORMAT;
			correct_stmt = FALSE;
		}
		else {
			procCnt = atoi(argv[ optind] );
		}
	}

	if (!correct_stmt) {
		MSG("%d\n", errcode);
	}
	else {
		MSG("correct stmt\n");
		MSG("percent:%d procCnt:%d\n", percent, procCnt);
		prate_gen (percent, procCnt, desc);
	}
	return 0;
}

BOOL isCorrectInt (char *pstr) {
	BOOL res = TRUE;
	int val = atoi (pstr);	// 소수점 위 파싱
	// 음수 체크 
	if (val < 0) res = FALSE;

	int len = 0;
	char str1[ NUM_MAX_LENGTH ];
	char *str2 = malloc(NUM_MAX_LENGTH);

	strcpy(str1, pstr);
	int sosu, i;
	char *ptr = NULL;
	/* 소수점 밑 확인 */
	ptr = strtok_r(str1, ".", &str2);
	if (strlen( str2 ) == 0 ) {
		sosu = atoi (str1);
		if (sosu != -1)
			res = TRUE;
	}
	else {
		// 소수점 밑에 특수문자 포함시 오류
		for (i=0 ; i < strlen(str2) ; i++) {
			switch (str2[i]) {
				case '0': 
					break;
				default :
					res = FALSE;
			}
		}
	}

	if (res == TRUE) {
		for ( ; val > 0 ; val /= 10 ) len++;
		// 소숫점 위 확인
		if (ptr == NULL){
			if (len != strlen(str1))
				res = FALSE;
		}
		else if (len != strlen (ptr))
			res = FALSE;
	}
	return res;
}

void prate_gen (int percent, int procCnt, BOOL desc) {
	int i, j;
	FILE *fp = NULL;
	BOOL child = FALSE, res = FALSE, failed = FALSE;
	int succCnt = 0, readCnt = 0;
	char buf[ MAX_FORK_CNT ] = {0, };
	pid_t pid_c[ MAX_FORK_CNT ];
	pid_t pid_returned;
	// open과 close를 위한 semaphore 구조체 정의
	struct sembuf mysem_open = {0, -1, SEM_UNDO};	//세마포어 얻기
	struct sembuf mysem_close = {0, 1, SEM_UNDO};	//세마포어 돌려주기
	int sem_num = 1;	//부모 프로세스는 생성자
	union semun sem_union;
	//세마포어 설정
	while ((semid = semget((key_t)1234, sem_num, 0666|IPC_CREAT) ) == -1 );
	
	struct sigaction act_new, act_old, act_child_new, act_child_old;
	// 부모프로세스 시그널핸들러
	act_new.sa_handler = &hdl_parent;
	act_new.sa_flags |= SA_NOCLDSTOP;
	sigemptyset (&act_new.sa_mask);
	//오류나는부분 시작(************
	sigaddset (&act_new.sa_mask, SIGINT);
	sigaddset (&act_new.sa_mask, SIGTERM);

	act_old.sa_handler = &hdl_parent;
	act_old.sa_flags |= SA_NOCLDSTOP;
	sigemptyset (&act_old.sa_mask);
	if (sigaction (SIGINT, &act_old, NULL) == -1) {
		failed = TRUE;
	}
	else if(sigaction (SIGTERM, &act_old, NULL) == -1) {
		failed = TRUE;
	}
	// 오류나는부분 끝*********

	// 자식프로세스 시그널핸들러 설정
	act_child_new.sa_handler = &hdl_child;
	sigemptyset (&act_child_new.sa_mask);
	sigaddset (&act_child_new.sa_mask, SIGINT);
	sigaddset (&act_child_new.sa_mask, SIGTERM);

	act_child_old.sa_handler = &hdl_child;
	sigemptyset (&act_child_old.sa_mask);



	for (i = 0; i < procCnt && !failed; )
	{
		for (j = 0; j < MAX_FORK_CNT && i < procCnt; j++, i++) {
			pid_c[ j ] = fork();
			//child
			if (pid_c[ j ]  == 0) {
				//오류나는부분 시작(************
				if (sigaction (SIGINT, &act_child_old, NULL) == -1) {
					failed = TRUE;
				}
				else if(sigaction (SIGTERM, &act_child_old, NULL) == -1) {
					failed = TRUE;
				}
				// 오류나는부분 끝*********
				child = TRUE;
				break;
			} else if (pid_c[ j ] == -1) {
				failed = TRUE;
			}

		}	
		if (child) {
			//자식프로세스 시그널 블록
			if ((sigprocmask (SIG_BLOCK, &act_child_new.sa_mask, NULL)) == -1) {
				exit(EXIT_FAILURE);
			}
			sem_num = 0;	//자식 프로세스는 소비자
			res = pmanipulator(percent);
			usleep (rand() % 200000);	//자식프로세스가 무작위 순으로 반환
			usleep (rand() % 500000);
			//세마포어 설정
			while ((semid = semget((key_t)1234, sem_num, IPC_CREAT )) == -1 );
			//세마포어 초기화
			sem_union.val = 1;
			if (-1 == semctl (semid, 0, SETVAL, sem_union)) {
				exit(EXIT_FAILURE);
			}
			//세마포어 검사
			if (semop(semid, &mysem_open, 1) == -1) {
				exit(EXIT_FAILURE);
			}
			
			/** 임계영역 **/
			//자식 프로세스들의 성공여부를 기록하는 파일을 연다.
			if ((fp = fopen(filename, "a+")) == NULL) {
				exit(EXIT_FAILURE);
			}
			fprintf (fp, "%d", res);
			fclose (fp);
			/** 임계영역 끝 **/

			//세마포어 자원을 되돌려준다.
			semop (semid, &mysem_close, 1);
			//블록 끝
			if ((sigprocmask (SIG_UNBLOCK, &act_child_new.sa_mask, NULL)) == -1) {
				exit(EXIT_FAILURE);
			}

			exit(EXIT_SUCCESS);
		}
		else {
			// 부모 프로세스
			//자식프로세스의 시작을 대기
			usleep(100000);

			//자식 프로세스로부터 기록된 반환을 대기하고 출력
			// MAX_FORK_CNT만큼의 자식을 생성하고, 그 자식들의 결과를 받아 출력
			// 자식 한개 단위로 신호를 블록
			//수행결과 기록파일 열기
			while ((fp = fopen(filename, "r"))== NULL);
			for (j = 0; j < MAX_FORK_CNT ; j++) {
				int stat;
				//시그널처리를 블록화
				if ((sigprocmask (SIG_BLOCK, &act_new.sa_mask, NULL)) == -1) {
					failed = TRUE;
					break;
				}
			
				// 자식 프로세스가 종료되기를 기다리고 pid를 받아온다.
				pid_returned = wait(&stat);
				if (pid_returned == -1) {
					break;
				}
				else if (WEXITSTATUS(stat) == EXIT_FAILURE) {
					//자식 프로세스가 비정상적으로 종료되었을 경우 오류
					failed = TRUE;
					break;
				}/*
				else if (WIFSIGNALED(stat)) {
					// 자식 프로세스가 신호에 의해 종료되었을 경우
					readCnt = fscanf (fp, "%c", &buf[j]);

					printf("신호%d\n", readCnt);
				}*/
				else {
					readCnt = fscanf (fp, "%c", &buf[j]);
				}
				//읽은 정보 기록
				if (readCnt == -1) {
					failed = TRUE;
					break;
				}
				else if (desc) {
					MSG("PID %d returned %s.\n"
							, pid_returned
							, buf[ j ]=='1'?"success":"failure"
					   );
				}
				if (buf[ j ] == '1')
					succCnt++;
				// MAX_FORK_CNT childs ended
				/* 오류나는부분 2************
				//시그널처리를 un블록화
				if ((sigprocmask (SIG_UNBLOCK, &act_new.sa_mask, NULL)) == -1) {
					failed = TRUE;
					break;
				} */
				MSG("종료\n");
				fflush(stdout);
				sleep(1);
			}
			fclose (fp);
		}
	}
	//세마포어 키 반납
	if (-1 == semctl (semid, 0, IPC_RMID, sem_union)) {
		failed = TRUE;
	}
	// 부모 프로세스
	if (failed) {
		MSG("명령어 수행 실패\n");
		exit(EXIT_FAILURE);
	}
	printf("succ:%d\n", succCnt);
	float succ =  (float)succCnt / (float)procCnt;
	succ *= 100;
	MSG("Created %d processes.\n", procCnt);
	MSG("Success: %d %%\n", (int)succ);
	MSG("Failure: %d %%\n", 100 - (int)succ);
	
	//수행결과를 기록하던 파일을 삭제
	unlink(filename);

}

void hdl_parent (int signal) {
	MSG("sig:%d pid:%d : %d\n", signal, getpid(), isBlocked);
	MSG("asdf\n");

}
void hdl_child (int signal) {
	MSG("child hdl: %d\n", isBlocked);

}
