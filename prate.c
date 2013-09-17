#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
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

extern int opterr, optind;
extern char *optarg;
extern int optopt;
BOOL isCorrectInt (char *pstr);
void prate_gen(int percent, int procCnt);
int main(int argc, char **argv, char **env) {
	int param_opt, first_op, num, errcode = 0;
	int percent, procCnt;
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
				if (correct_stmt) 
					procCnt = atoi(argv[ first_op ]);
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
	}

	if (!correct_stmt) {
		printf("%d\n", errcode);
	}
	else {
		prate_gen (percent, procCnt);
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

void prate_gen (int percent, int procCnt) {
	int i, j;
	BOOL child = FALSE;
	pid_t ppid = getpid();
	pid_t pid_c[ MAX_FORK_CNT ];
	for (i = 0; i < procCnt && !child ; )
	{
		for (j = 0; j < MAX_FORK_CNT && i < procCnt; j++, i++) {
			pid_c[ j ] = fork();
			//child
			if (pid_c[ j ]  == 0) {
				child = TRUE;
				break;
			}
		}
		if (getpid() == ppid){
			sleep(1);
			printf("parent\n");
		}
		
	}



	printf("%d\n, i:%d j:%d\n", getpid(), i, j);
}
