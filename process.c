//-----------------------------------------
// NAME: Xing Zhou 
// STUDENT NUMBER: 7869781
// COURSE: COMP 3430, SECTION: A01
// INSTRUCTOR: Robert Guderian
// ASSIGNMENT: assignment 1, QUESTION: question 2
// REMARKS: the process herder
//-----------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

int count=0;
int queue[128];
char buf[10];

void handler1(int arg){
    printf("signal SIGINT(%d) received, worker with pid %d stopped\n", arg, getpid());
    exit(EXIT_SUCCESS);
}

void handler2(int arg){
	printf("signal SIGHUP(%d) received by supervisor, re-read configure file\n", arg);
	FILE *confi=fopen("configure.txt","r");  
	fgets(buf, 10, confi);
	if(buf[strlen(buf)-1]=='\n'){
		buf[strlen(buf)-1]=0;
	}
	int newNum=atoi(buf);
	fclose(confi);
	int diff=newNum-count;
	if(diff>0){
		printf("%d more worker(s) to create\n", diff);
		for(int i=0; i<diff; i++){
			pid_t child=fork();
            if(child<0){
                printf("fork failed\n");
		        exit(1);
            }
            else if(child==0){
        	    signal(SIGINT, handler1); 
        	    printf("worker %d with pid %d created\n", count+i+1, getpid());
        	    while(1){
        		    sleep(1);
			    }
            }
            else{
        	    queue[count+i]=child;
		    }
		}
	}
	else if(diff==0){
		printf("number in configure file keeps same, nothing to change");
	}
	else{
		printf("%d worker(s) to remove\n", -diff);
		for(int i=count-1; i>count-1+diff; i--){
			kill(queue[i],SIGINT);
			int w=waitpid(queue[i], NULL, 0);
			printf("worker %d with pid %d terminated successfully\n", i+1, w);
		}
	}
	count=newNum;
}

void handler3(int arg){
	printf("signal SIGINT(%d) received by supervisor, to terminate all stuff\n", arg);
    for(int i=0; i<count; i++){
    	printf("send SIGINT(2) to worker %d\n", i+1);
    	kill(queue[i], SIGINT);
    	int w=waitpid(queue[i], NULL, 0);
		printf("worker %d with pid %d terminated successfully\n", i+1, w);
	}
	printf("the supervisor process with pid %d totally completed\n", getpid());
	exit(EXIT_SUCCESS);
}

int main(){
	printf("the supervisor process has pid %d\n", getpid());
	fflush(stdout);	
    FILE *confi=fopen("configure.txt","r");  
	fgets(buf, 10, confi);
	if(buf[strlen(buf)-1]=='\n'){
		buf[strlen(buf)-1]=0;
	}
	count=atoi(buf);
	fclose(confi);
	signal(SIGHUP, handler2);
	signal(SIGINT, handler3);
    for(int i=0; i<count; i++){
    	pid_t child=fork();
        if(child<0){
            printf("fork failed\n");
		    exit(1);
        }
        else if(child==0){
        	signal(SIGINT, handler1);
        	printf("worker %d with pid %d created\n", i+1, getpid());
        	while(1){
        		sleep(1);
			}
        }
        else{
        	queue[i]=child;
		}
	}
	while(1){
		sleep(1);
	}
    return 0;
}