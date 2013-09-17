#include<stdio.h>
#include <stdlib.h>
#include <string.h>
typedef int BOOL;
#define FALSE 0
#define TRUE 1
#define NUM_MAX_LENGTH 50

BOOL isCorrectInt (char *pstr) {
	int val = atoi (pstr);	// 소수점 위 파싱

	int len = 0;
	char str1[ NUM_MAX_LENGTH ];
	char *str2 = malloc(NUM_MAX_LENGTH);

	strcpy(str1, pstr);
	BOOL res = FALSE;
	int sosu;
	char *ptr = NULL;

	/* 소수점 밑 확인 */
	ptr = strtok_r(str1, ".", &str2);
	if (strlen( str2 ) == 0 ) {
		sosu = atoi (str1);
		if (sosu != -1)
			res = TRUE;
	}
	else if (strlen( str2 ) > 0) {
		sosu = atoi (str2);
		if (sosu == 0 )
			res = TRUE;
		else 
			res = FALSE;
	}
	
	// 소수점 갯수 가 한개를 넘어서면 오류
	if (res) {
		strcpy(str1, pstr);
		ptr = strtok_r(str1, "0123456789", &str2);
		if (strlen(str2)>1)
			res = FALSE;
	}

	// 음수 체크 
	if (val < 0) res = FALSE;
	else if (res == TRUE) {
		for ( ; val > 0 ; val /= 10 ) len++;

		// 소숫점 위 확인
		if (len == strlen (ptr))
			res = TRUE;
		else res = FALSE;
	}

	return res;
}

int main(void) 
{
	int a;
	if( isCorrectInt("2.0") )
		a = atoi("2.0");
	printf("%d\n", a); 
	return 0;
}
