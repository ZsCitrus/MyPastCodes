//-----------------------------------------
// NAME: Xing Zhou 
// STUDENT NUMBER: 7869781
// COURSE: COMP 3430, SECTION: A01
// INSTRUCTOR: Robert Guderian
// ASSIGNMENT: assignment 1, QUESTION: question 2
// REMARKS: the thread herder
//-----------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAX 128;
#define BUF 10

pthread_t threads[MAX];
int queue[MAX];
int count=0;
char buf[BUF];

void* Worker(int);
void handler1(int arg);
void handler2(int arg);

int main(){
	for(int i=0; i<MAX; i++){
		queue[i]=0;
	}
	printf("the supervisor thread has id %d to receive signals\n", getpid());
	fflush(stdout);	
    FILE *confi=fopen("configure.txt","r");  
	fgets(buf, BUF, confi);
	if(buf[strlen(buf)-1]=='\n'){
		buf[strlen(buf)-1]=0;
	}
	count=atoi(buf);
	fclose(confi);
	signal(SIGHUP, handler1);
	signal(SIGINT, handler2);
    for(int i=0; i<count; i++){
	    pthread_create(&threads[i], NULL, Worker, i);
    }
    while(1){
		sleep(1);
	}
    return 0;
}

void* Worker(int i){
	printf("worker thread %d created\n", i+1);
	while(queue[i]==0){
		sleep(1);
	}
	printf("worker thread %d stopped\n", i+1);
	pthread_exit(NULL);
}

void handler1(int arg){
	printf("signal SIGHUP(%d) received by supervisor, re-read configure file\n", arg);
	FILE *confi=fopen("configure.txt","r");  
	fgets(buf, BUF, confi);
	if(buf[strlen(buf)-1]=='\n'){
		buf[strlen(buf)-1]=0;
	}
	int newNum=atoi(buf);
	fclose(confi);
	int diff=newNum-count;
	if(diff>0){
		printf("%d more worker(s) to create\n", diff);
		for(int i=0; i<diff; i++){
			pthread_create(&threads[count+i], NULL, Worker, count+i);
		}
	}
	else if(diff==0){
		printf("number in configure file keeps same, nothing to change\n");
	}
	else{
		printf("%d worker(s) to remove\n", -diff);
		for(int i=count-1; i>count-1+diff; i--){
			queue[i]=1;
			pthread_join(threads[i], NULL);
			printf("worker thread %d terminated successfully\n", i+1);
		}
	}
	count=newNum;
}

void handler2(int arg){
	printf("signal SIGINT(%d) received by supervisor, to terminate all stuff\n", arg);
    for(int i=0; i<count; i++){
    	queue[i]=1;
    	pthread_join(threads[i], NULL);
		printf("worker thread %d terminated successfully\n", i+1);
	}
	printf("the supervisor thread totally completed\n");
	exit(EXIT_SUCCESS);
}