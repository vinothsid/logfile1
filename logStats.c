#include<stdio.h>
#include<assert.h>
#include<string.h>
#include<pthread.h>

#define MAX_COUNT 10

struct block {
	int sIndex;
	int eIndex;
} *fBlocks;

int **count;
char psList[MAX_COUNT][100];
int numThread;
int numProc;
char logFileName[50];
int *thrArray;
//int *offSet;
//pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;


int init( int psCount,int threadCount ) {
	int i=0;
	
	thrArray = (int *)malloc(sizeof(int) * threadCount);
	fBlocks=(struct block*)malloc( sizeof(struct block) * threadCount );

        count= (int **)calloc( psCount , sizeof(int *) );
	for(i=0;i<psCount;i++)
                count[i]=(int *) calloc(threadCount , sizeof(int) );
	
	for(i=0;i<threadCount;i++) { 
		thrArray[i]=i;
		(fBlocks+i)->sIndex=-1;
		(fBlocks+i)->eIndex=-1;
	}
		
	return 0;
}

void * getStat(void *arg) {
	int threadId = *(int *)arg;

	printf("Thread Id : %d\n",threadId);
	int start = (fBlocks+threadId)->sIndex;
	int end = (fBlocks+threadId)->eIndex;
	if( start == -1 || end == -1)
		return;
//Read each ps from log file
//for each ps ,do findMatch and increment the count of index returned by it.
	FILE *fp ;
	char procName[50];
	fp = fopen(logFileName,"r");
	printf("Block start : %d end : %d \n",start,end);
	int pIndex;
	fseek(fp,start,SEEK_SET);
	while( ftell(fp) != end ) {
		fscanf(fp,"%*s %*s %*s %[^\[]%*[^\n]\n",procName);
		pIndex = findMatch(procName);
		if(pIndex!=-1) {
//			pthread_mutex_lock(&mutex1);
			count[pIndex][threadId]++;
			printf("Count----> : %d\n",count[pIndex][threadId]);
//			pthread_mutex_unlock(&mutex1);
//
		}
	}

	return 0;
}

void printDetails() {
	int i = 0,j,tmp=0,numLogLines=0;
	printf("========================================================\n");
	printf("Statistics\n");
	printf("========================================================\n");
	printf("Process Name\tCount\n");
	
	for(i=0;i<numProc;i++) {
		for(j=0;j<numThread;j++) {
			tmp+=count[i][j];
		}
		printf("%-12s\t%4d\n",psList[i],tmp);
		tmp=0;
	}


}
int findMatch(char *s) {

	int found=0;

	int i;
	for(i=0;i<numProc;i++) {
		if(strcmp(s,psList[i])==0)
			return i;
	}

	return -1;
} 

int getProcList(char *fileName) {
	FILE *fp;
	fp = fopen(fileName,"r");
	assert(fp);
	int i=0;
	while(fscanf(fp,"%[^\n]\n",psList[i++])==1);

/*	numProc=i-1;
	for(i=0;i<numProc;i++) {
		printf("%s\n",psList[i]);
	}
*/
	fclose(fp);
	return i-1;
}

int findBlockIndices(char *fileName, int threadCount ) {
//Fill in sIndex and eIndex of fBlocks[0..threadCount-1]

	FILE *fp;
	int size,blockSize,i,start,end,lineNum=0;
	char newLine;
	fp = fopen(fileName,"r");
	assert(fp);
	
	fseek(fp,0,SEEK_END);
	size = ftell(fp);
	assert(size!=0);
	printf("Size of file is : %d\n",size);

	fseek(fp,0,SEEK_SET);

/*
	while(fscanf(fp,"%*[^\n]\n")==0)
		lineNum++;

	fseek(fp,0,SEEK_SET);
	printf("Total number of lines : %d \n",lineNum);	
//	blockSize = lineNum/threadCount;
	start=0;
	int j=0;
	int reminder=0;
	int tCount = threadCount;
	for(i=0;i<threadCount;i++) {

		reminder = lineNum%tCount;
		if( ( (float) reminder/tCount ) >= 0.5) {
			blockSize = lineNum/tCount + 1;	
		} else {
			blockSize = lineNum/tCount;
		}
		tCount--;
		lineNum = lineNum - blockSize;

		printf("Number of lines allocated for thread : %d is %d \n",i , blockSize);
		for(j=0;j<blockSize && fscanf(fp,"%*[^\n]\n")==0; j++);
		end=ftell(fp);
                (fBlocks+i)->sIndex = start;
                (fBlocks+i)->eIndex = end;
                start=end+1;
			
	}
*/

	start=0;
	end=0;
	int tc=threadCount;
	int remChars = size;
	for(i=0;i<threadCount && end < size;i++) {
		blockSize = remChars/tc;
		printf("i : %d . blockSize : %d remChars : %d remThread : %d \n",i,blockSize,remChars,tc);
		if( (start+blockSize-1 ) <= size ) {
			if (fseek(fp, blockSize ,SEEK_CUR)!= 0) {
				printf("Fseek failed during iteration %d with block size : %d\n",i,blockSize);
			}
		} 
		printf("Searching for newLine .... \n");
		do {
			newLine = fgetc(fp);	
		//	printf("Cur Char = %c \n",newLine);
		} while(newLine!='\n' && newLine!=EOF);
		end = ftell(fp);
		if(newLine=='\n')
			printf("NewLine found at : %d \n", end);
		else
			printf("EOF encountered at : %d \n",end);

		(fBlocks+i)->sIndex = start;
		(fBlocks+i)->eIndex = end;
		remChars=size-end;
		tc--;
		start=end+1;
	}


	for(i=0;i<threadCount;i++) 
		printf("Block num : %4d . sIndex : %4d eIndex : %4d\n",i,(fBlocks+i)->sIndex,(fBlocks+i)->eIndex);
	
	return 1;
}


int main() {

//	pthread_t t1,t2,t3,t4,t5;

	numThread = 10;
	strcpy(logFileName,"log");
	numProc = getProcList("pList");
	init(numProc,numThread);

	findBlockIndices(logFileName,numThread);
	printf("Number of processes in the given list is : %d \n" , numProc);

/*
	int threadId=3;
	getStat(&threadId);
*/

	pthread_t *t;
	t=(pthread_t *)malloc(sizeof(pthread_t) * numThread);

	int i;
	for(i=0;i<numThread;i++)
		pthread_create(t + i ,NULL,getStat,(void *)&thrArray[i]);


	for(i=0;i<numThread;i++)
		pthread_join(t[i],NULL);

	printDetails();	


//	printf("Position of grep is %d \n",findMatch("sed"));
}
