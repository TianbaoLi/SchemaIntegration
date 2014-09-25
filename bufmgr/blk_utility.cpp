#include"buf_common.h"
#include"stdio.h"
#include "math.h"
#include "stdlib.h"
#include "time.h"

long hashA, hashB, largePrime;
long firstPrimeLargerThan(long entries){
	long n=entries*1.3;
	int b = 1;
	while(b){
//		printf(">>%ld\n",n);
		int r=(int)sqrt(n);
		if(n%2==0){
			n += 1;
			continue;
		}
		int t=0;
		for(int i =3; i<=r;i+=2){
			if(n%i == 0){
				t = 1;
				break;
			}
		}
		if(t == 0){
			break;
		}
		else{
			n += 1;
		}
	}
	return n;
}

// initialize hashA and hashB;
void initHash(long entries)
{
	srand( (unsigned)time( NULL ) );
	largePrime = firstPrimeLargerThan(entries);
	hashA = 1 + (int)(rand() * ( (largePrime - 1) /(double)RAND_MAX ));
	//hashB = 1 + rand() % (largePrime - 1);
  hashB = 1 + (int)(rand() * ( (largePrime - 1) /(double)RAND_MAX ));

  printf("Hash = ( %d * X + %d ) %% %d\n", hashA, hashB, largePrime);
}