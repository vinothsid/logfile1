#include<stdio.h>
#include<assert.h>
#include<string.h>
#define MAX_COUNT 10

struct block {
	int sIndex;
	int eIndex;
} *fBlocks;

int count[MAX_COUNT];
char psList[MAX_COUNT][100];
int numThread;
int numProc;
char logFileName[50];
//int *offSet;

int init( int psCount ) {
	int i=0;
	for(i=0;i<psCount;i++)
		count[i]=0;
	
	return 0;
}

void * getStat(void *arg) {
	int threadId = *(int *)arg;

	printf("Thread Id : %d\n",threadId);
//Read each ps from log file
//for each ps ,do findMatch and increment the count of index returned by it.
	FILE *fp ;
	char procName[50];
	fp = fopen(logFileName,"r");
	int start = (fBlocks+threadId)->sIndex;
	int end = (fBlocks+threadId)->eIndex;
	printf("Block start : %d end : %d \n",start,end);
	int pIndex;
	fseek(fp,start,SEEK_SET);
	while( ftell(fp) != end ) {
		fscanf(fp,"%*s %*s %*s %[^\[]%*[^\n]\n",procName);
		pIndex = findMatch(procName);
		if(pIndex!=-1) {
//Critical section
			count[pIndex]++;
//
		}
	}

	return 0;
}

void printDetails() {
	int i = 0;
	printf("========================================================\n");
	printf("Statistics\n");
	printf("========================================================\n");
	printf("Process Name\tCount\n");
	for(i=0;i<numProc;i++) {
		printf("%-12s\t%4d\n",psList[i],count[i]);
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
	blockSize = size/threadCount;
	fBlocks=(struct block*)malloc( sizeof(struct block) * threadCount );

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
/*
	start=0;
	for(i=0;i<threadCount;i++) {
		if( (start+blockSize ) <= size ) {
			if (fseek(fp, blockSize ,SEEK_CUR)!= 0) {
				printf("Fseek failed during iteration %d with block size : %d\n",i,blockSize);
			}
		} 
		printf("Searching for newLine .... \n");
		do {
			newLine = fgetc(fp);	
			printf("Cur Char = %c \n",newLine);
		} while(newLine!='\n' && newLine!=EOF);
		end = ftell(fp);
		printf("NewLine or EOF found at : %d \n", end);
		(fBlocks+i)->sIndex = start;
		(fBlocks+i)->eIndex = end;
		start=end+1;
	}

*/
	for(i=0;i<threadCount;i++) 
		printf("Block num : %4d . sIndex : %4d eIndex : %4d\n",i,(fBlocks+i)->sIndex,(fBlocks+i)->eIndex);
	
	return 1;
}


int main() {
	numThread = 5;
	strcpy(logFileName,"log");
	findBlockIndices(logFileName,numThread);
	numProc = getProcList("pList");
	printf("Number of processes in the given list is : %d \n" , numProc);
	init(numProc);


	int threadId = 0;
	getStat((void *)&threadId);

	printDetails();	
//	printf("Position of grep is %d \n",findMatch("sed"));
}
