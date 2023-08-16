//-----------------------------------------
// NAME: Xing Zhou
// STUDENT NUMBER: 7869781
// COURSE: COMP 3430, SECTION: A01
// INSTRUCTOR: Robert Guderian
// ASSIGNMENT: assignment 3, QUESTION: question 1
// REMARKS: scheduler simulator
//-----------------------------------------

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define STD 200 //length of the queue
#define QUA 50 //quantum number
#define SLICE 200 //time slice
#define USEC_PER_MILLI 1000 
#define NANOS_PER_USEC 1000
#define USEC_PER_SEC   1000000

typedef struct TASK{
	char name[20];
	int type;
	int length;
	int io;
	int priority;
	int max;
	int remain;
	int run;
	int first;
	struct timespec arrive;
	struct timespec firstRe;
	struct timespec last;
}task;

char* inputFile="tasks.txt";
int cpuNo=0;
int s=0;
int finish=0;
int taskNo=0;
int finishNo=0;
char buf[STD]; //char buf for parse line
char *tBuf[4];
int tCount=0;
char *sBuf[2];
int sCount=0;
task *q[3][STD]; //the queue
int count[3][2];
task *reschedule[STD];
int rHead=0;
int rEnd=0;
int promptControl=0;
task *output[STD];
pthread_mutex_t queueLock, scheLock, writeLock;
pthread_cond_t readWait, cpuWait, scheWait;

void *readThread(void*);
void *cpu(void*);
void *scheduler(void*);
void *prompt(void*);
int min(int, int);
void microsleep(unsigned int);
struct timespec diff(struct timespec start, struct timespec end);


int main(int argc, char *argv[]){
	printf("program starts\n");
    if(argc!=4){
    	fprintf(stderr, "wrong argument\n");
	}	
    inputFile=argv[3];
	cpuNo=atoi(argv[1]);
	s=atoi(argv[2]);
	
	//variable for output
	unsigned long long response[4]={0,0,0,0};
	unsigned long long turnaround[4]={0,0,0,0};
	int num[4]={0,0,0,0};
	struct timespec res;
	struct timespec turn;
	int taskType;
	
	//all initialization below
	int check;
	for(int i=0; i<3; i++){
		for(int j=0; j<2; j++){
			count[i][j]=0;
		}
	}
	
	//lock and cv
	check=pthread_mutex_init(&queueLock, NULL);
	assert(check==0);
	check=pthread_mutex_init(&scheLock, NULL);
    assert(check==0);
	check=pthread_mutex_init(&writeLock, NULL);
	assert(check==0);
	check=pthread_cond_init(&readWait, NULL);
	assert(check==0);
	check=pthread_cond_init(&cpuWait, NULL);
	assert(check==0);
	check=pthread_cond_init(&scheWait, NULL);
	assert(check==0);
	
	//thread
	pthread_t tRead, sche, tPrompt;
	pthread_t tCpu[cpuNo];
	check=pthread_create(&tRead, NULL, readThread, NULL);
	assert(check==0);
	for(int i=0; i<cpuNo; i++){
	    check=pthread_create(&tCpu[i], NULL, cpu, NULL);
	    assert(check==0);	
	}
	check=pthread_create(&sche, NULL, scheduler, NULL);
	assert(check==0);
	check=pthread_create(&tPrompt, NULL, prompt, NULL);
	assert(check==0);
	
	//last call to force all tread exist
	check=pthread_cond_signal(&readWait);
    assert(check==0);
    for(int i=0; i<cpuNo; i++){
    	check=pthread_cond_broadcast(&cpuWait);
        assert(check==0);
	}
    check=pthread_cond_signal(&scheWait);
    assert(check==0);
    
	check=pthread_join(tRead, NULL);
	assert(check==0);
	for(int i=0; i<cpuNo; i++){
	    check=pthread_join(tCpu[i], NULL);
	    assert(check==0);
	}
	
	//join
	check=pthread_join(sche, NULL);
	assert(check==0);
	check=pthread_join(tPrompt, NULL);
	assert(check==0);
	
	//garbage collection
	check=pthread_mutex_destroy(&queueLock);
	assert(check==0);
	check=pthread_mutex_destroy(&scheLock);
	assert(check==0);
	check=pthread_mutex_destroy(&writeLock);
	assert(check==0);
	check=pthread_cond_destroy(&readWait);
	assert(check==0);
	check=pthread_cond_destroy(&cpuWait);
	assert(check==0);
	check=pthread_cond_destroy(&scheWait);
    assert(check==0);
    
	//the final output	
	for(int i=0; i<finishNo; i++){
		taskType=output[i]->type;
		res=diff(output[i]->arrive, output[i]->firstRe);
		turn=diff(output[i]->arrive, output[i]->last);
		response[taskType]+=res.tv_sec*1000000000+res.tv_nsec;
		turnaround[taskType]+=turn.tv_sec*1000000000+turn.tv_nsec;
		num[taskType]++;
	}
    printf("average response time in microseconds with %d cpu:\n", cpuNo);
	printf("short task: %llu\n", response[0]/num[0]/NANOS_PER_USEC);
	printf("medium task: %llu\n", response[1]/num[1]/NANOS_PER_USEC );
	printf("long task: %llu\n", response[2]/num[2]/NANOS_PER_USEC);
	printf("io task: %llu\n", response[3]/num[3])/NANOS_PER_USEC;
	printf("average turnaround time in microseconds with %d cpu:\n", cpuNo);
	printf("short task: %llu\n", turnaround[0]/num[0]/NANOS_PER_USEC);
	printf("medium task: %llu\n", turnaround[1]/num[1]/NANOS_PER_USEC);
	printf("long task: %llu\n", turnaround[2]/num[2]/NANOS_PER_USEC);
	printf("io task: %llu\n", turnaround[3]/num[3])/NANOS_PER_USEC;
	printf("program ends\n");
	return 0;
}

//read the task file
void *readThread(void *a){
    a=NULL;
    int check;
	task* t;
	struct timespec time;
	int wait;	
	char *word;
	FILE *in=fopen(inputFile, "r");
	if(in==NULL){
		fprintf(stderr, "invalid input file\n");
	}
	while(fgets(buf, STD, in)!=NULL){
		if(buf[strlen(buf)-1]=='\n'){
			buf[strlen(buf)-1]='\0';
		}
		if(buf[0]!='D'){
			word=strtok(buf, " ");
		    while(word!=NULL){
			    tBuf[tCount]=word;
			    tCount++;
			    word=strtok(NULL, " ");
		    }
		    tCount=0;
		    t=malloc(sizeof(task));
		    strncpy(t->name, tBuf[0], 20);
		    t->type=atoi(tBuf[1]);
		    t->length=atoi(tBuf[2]);
		    t->io=atoi(tBuf[3]);
		    t->priority=0;
		    t->max=QUA;
		    t->remain=SLICE;
		    t->run=0;
		    t->first=0;
		    clock_gettime(CLOCK_REALTIME, &time);
		    t->arrive=time;
		    check=pthread_mutex_lock(&queueLock);
		    assert(check==0);
		    
		    //when the queue is full or scheduler is moving stuff to top queue
		    //read thread suspend
		    while(count[0][1]-count[0][0]>=STD||promptControl==1){
		    	check=pthread_cond_wait(&readWait, &queueLock);
		    	assert(check==0);
			}             
		    q[0][count[0][1]%STD]=t;
			count[0][1]++;
		    taskNo++;
		    check=pthread_cond_signal(&cpuWait); 
		    assert(check==0);
		    check=pthread_mutex_unlock(&queueLock);
		    assert(check==0);
		}
		
		//the delay
		else{
			word=strtok(buf, "Y");
		    while(word!=NULL){
			    sBuf[sCount]=word;
			    sCount++;
			    word=strtok(NULL, "Y");
		    }
		    sCount=0;
		    wait=atoi(sBuf[1]);
			microsleep(wait*USEC_PER_MILLI);
		}
	}
	fclose(in);
	finish=1;
	check=pthread_cond_signal(&scheWait);
    assert(check==0);
    check=pthread_cond_broadcast(&cpuWait);
    assert(check==0);
	return NULL;
}

//cpu thread
void *cpu(void *a){
	a=NULL;
	struct timespec time;
	int check;
	int ioChance;
	int ioTime;
	int prio;
	int head;
	int left;
	int max;
	int realRun;
	task *t;
	int complete=0;
	int qUnlock=0;
	
	//the check condition make sure input file complete and all stuff in queue complete
	while(finish==0||finishNo<taskNo){	
		check=pthread_mutex_lock(&queueLock);
	    assert(check==0);
	    
	    //when queue is empty or scheduler is moving stuff to the top queue, cpu suspend
	    while(((count[0][0]==count[0][1]&&count[1][0]==count[1][1]&&count[2][0]==count[2][1])||promptControl==1)&&(finish==0||finishNo<taskNo)){
		    check=pthread_cond_wait(&cpuWait, &queueLock);
		    assert(check==0);
	    }
	    
	    //queue priority
	    if(finish==0||finishNo<taskNo){
	    	if(count[0][0]<count[0][1]){
		        prio=0;
		        head=count[prio][0];
	        }
	        if(count[0][0]==count[0][1]&&count[1][0]<count[1][1]){
		        prio=1;
		        head=count[prio][0];
	        }
	        if(count[0][0]==count[0][1]&&count[1][0]==count[1][1]&&count[2][0]<count[2][1]){
		        prio=2;
		        head=count[prio][0];
	        }
	        
	        //some arithmetical to determine hwo much to run
	        //and determine it goes to the scheduler or output
	        left=q[prio][head%STD]->length-q[prio][head%STD]->run;
	        max=q[prio][head%STD]->max;
	        ioChance=rand()%101;
	        ioTime=rand()%(QUA+1);
	        if(ioChance>q[prio][head%STD]->io||ioTime>max){
		        if(left<=max){
			        realRun=left;
			        complete=1;
		        }
		        else{
			        realRun=max;
		        }
	        }  
	        if(ioChance<=q[prio][head%STD]->io&&ioTime<=max){
		        if(left<=ioTime){
			        realRun=left;
			        complete=1;
		        }
		        else{
			        realRun=ioTime;
		        }
	        }
	        t=q[prio][head%STD];
	        count[prio][0]++;
	        check=pthread_cond_signal(&scheWait);
	        assert(check==0);
	        check=pthread_cond_signal(&readWait);
	        assert(check==0);
	        check=pthread_mutex_unlock(&queueLock);
	        assert(check==0);
	        qUnlock=1;
	        
	        //"run the program"
	        t->remain-=realRun;
	        t->run+=realRun;
	        if(t->first==0){
	        	t->first=1;
	        	clock_gettime(CLOCK_REALTIME, &time);
	        	t->firstRe=time;
			}
	        microsleep(realRun);
	        
	        //go to output
	        if(complete==1){
		        check=pthread_mutex_lock(&writeLock);
	            assert(check==0);
	            clock_gettime(CLOCK_REALTIME, &time);
	        	t->last=time;
	            output[finishNo]=t;
	            finishNo++;      
	            check=pthread_cond_signal(&scheWait);
                assert(check==0);
		        check=pthread_mutex_unlock(&writeLock);
	            assert(check==0);
	        }
	        
	        //go to scheduler
	        if(complete==0){
		        check=pthread_mutex_lock(&scheLock);
	            assert(check==0);
		        while(rEnd-rHead>=STD){
			        check=pthread_cond_wait(&cpuWait, &scheLock);
		            assert(check==0);
		        }
		        reschedule[rEnd%STD]=t;
		        rEnd++;    		        
		        check=pthread_cond_signal(&scheWait);
	            assert(check==0);
		        check=pthread_mutex_unlock(&scheLock);
	            assert(check==0);
	        }
		}
		
		//make sure all thread exit gracefully
		if(finish==1&&finishNo==taskNo&&qUnlock==0){
			check=pthread_cond_signal(&scheWait);
            assert(check==0);
            check=pthread_mutex_unlock(&queueLock);
	        assert(check==0);
		}
		qUnlock=0;
	    complete=0;
	}
	//make sure all thread exit gracefully too
	check=pthread_cond_signal(&scheWait);
    assert(check==0);
    check=pthread_cond_broadcast(&cpuWait);
    assert(check==0);
	return NULL;
}

//the scheduler
void *scheduler(void *a){
	a=NULL;
	int check;
	int prio;
	int end;
	task *t;
	while(finish==0||finishNo<taskNo){
		check=pthread_mutex_lock(&scheLock);
	    assert(check==0);
	    
	    //when nothing to do, go sleep
	    while((rHead==rEnd)&&(finish==0||finishNo<taskNo)){
	    	check=pthread_cond_wait(&scheWait, &scheLock);
		    assert(check==0);
		}
		if(finish==0||finishNo<taskNo){
			
			//scheduler determine the priority
			if(reschedule[rHead%STD]->remain==0){
			    if(reschedule[rHead%STD]->priority<2){
				    reschedule[rHead%STD]->priority++;
			    }
			    reschedule[rHead%STD]->remain=SLICE;
			    reschedule[rHead%STD]->max=QUA;
		    }
		    else{
			    reschedule[rHead%STD]->max=min(reschedule[rHead%STD]->remain, QUA);
		    }
		    prio=reschedule[rHead%STD]->priority;
		    t=reschedule[rHead%STD];
		    rHead++;
		    check=pthread_cond_signal(&cpuWait);
	        assert(check==0);
		    check=pthread_mutex_unlock(&scheLock);
	        assert(check==0);
			
			//put task back to the queue	    
	        check=pthread_mutex_lock(&queueLock);
	        assert(check==0);  
	        while(count[prio][1]-count[prio][0]>=STD||promptControl==1){
	    	    check=pthread_cond_wait(&scheWait, &queueLock);
		        assert(check==0);
		    }
		    end=count[prio][1];
		    q[prio][end%STD]=t;
		    count[prio][1]++;
		    check=pthread_cond_signal(&cpuWait);
	        assert(check==0);
		    check=pthread_mutex_unlock(&queueLock);
	        assert(check==0);
		}
		
		//exist gracefully
		if(finish==1&&finishNo==taskNo){
			check=pthread_cond_broadcast(&cpuWait);
	        assert(check==0);
	        check=pthread_mutex_unlock(&scheLock);
	        assert(check==0);
		}
	}
	check=pthread_cond_broadcast(&cpuWait);
	assert(check==0);
	return NULL;
}

//moving all stuff back to top queue every s time
void *prompt(void *a){
    a=NULL;
	int check;
	while(finish==0||finishNo<taskNo){
		microsleep(s*USEC_PER_MILLI);
		promptControl=1;
		
		//in the moving, do not let other thread access the queue
		check=pthread_mutex_lock(&queueLock);
		assert(check==0);
		for(int i=1; i<3; i++){
			for(int j=count[i][0]; j<count[i][1]; j++){
				q[i][j%STD]->priority=0;
			    q[i][j%STD]->max=QUA;
			    q[i][j%STD]->remain=SLICE;
			    q[0][count[0][1]%STD]=q[i][j%STD];
			    count[0][1]++;
			}
			count[i][1]=count[i][0];
		}
		promptControl=0;
		check=pthread_cond_signal(&readWait);
        assert(check==0);
    	check=pthread_cond_broadcast(&cpuWait);
        assert(check==0);
        check=pthread_cond_signal(&scheWait);
        assert(check==0);
		check=pthread_mutex_unlock(&queueLock);
		assert(check==0);
	}
	return NULL;
}

//just a min function for arithmetical
int min(int a, int b){
	int min=a;
	if(a>=b){
		min=b;
	}
	return min;
}

//provided sleep function
void microsleep(unsigned int usecs){
    long seconds=usecs/USEC_PER_SEC;
    long nanos=(usecs%USEC_PER_SEC)*NANOS_PER_USEC;
    struct timespec t={.tv_sec=seconds, .tv_nsec=nanos};
    int ret;
    do{
        ret=nanosleep(&t, &t);
        // need to loop, `nanosleep` might return before sleeping
        // for the complete time (see `man nanosleep` for details)
    }while (ret==-1&&(t.tv_sec||t.tv_nsec));
}

//provided profiling function
struct timespec diff(struct timespec start, struct timespec end){
	struct timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec=end.tv_sec-start.tv_sec-1;
		temp.tv_nsec=1000000000+end.tv_nsec-start.tv_nsec;
	} 
	else {
		temp.tv_sec=end.tv_sec-start.tv_sec;
		temp.tv_nsec=end.tv_nsec-start.tv_nsec;
	}
	return temp;
}
