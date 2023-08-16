//-----------------------------------------
// NAME: Xing Zhou
// STUDENT NUMBER: 7869781
// COURSE: COMP 3430, SECTION: A01
// INSTRUCTOR: Robert Guderian
// ASSIGNMENT: assignment 2, QUESTION: question 1
// REMARKS: non-interactive shell
//-----------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUM 50 //a large enough number to hold all commands arguments in the array
#define BUF_SCRIPT 105 //script length is 100, add 5 for margin

//function details are below
void pretreat(char[]);
int parse(char*[], char[], char*);
int checkInput(char*[], int);
int checkOutput(char*[], int);
void single(int[][2], char*[]);
void firstNoInAndPipe(int[][2], char*[]);
void inputNoPipe(int[][2], char*[], int);
void inputAndPipe(int[][2], char*[], int);
void singleSubstitute(int[][2], char*[], char*[]);
void firstSubstitute(int[][2], char*[], char*[]);
void middle(int [][2], char*[], int);
void lastNoOutFile(int[][2], char*[], int);
void lastAndOutFile(int[][2], char*[], int, int);
void lastAndOutNoPipe(int[][2], char*[], int, int);

//the buffers to deal with various string components
char commandBuf[BUF_SCRIPT];
char argumentBuf[BUF_SCRIPT];
char substituteBuf[BUF_SCRIPT];

//array to decomposed strings 
char *command[NUM];
char *argument[NUM];
char *substitute[NUM];

//the count assoicated with the arrays above
int commandCount;
int argumentCount;
int substituteCount;

//to control whether a process substitute in the process
int substituteControl=0;

int main(int argc, char *argv[]){ 
	
	FILE *script;
	//check for < and >
	int check1;
	int check2;
	int parentID=getpid();
	
	printf("begin to execute the script\n");
	if(argc!=2){
		printf("wrong argument\n");
		exit(EXIT_FAILURE);
	}
	if((script=fopen(argv[1],"r"))==NULL){
		printf("wrong script file\n");
		exit(EXIT_FAILURE);
	}
	
	//initialize all arrays to hold decomposed parts
	for(int i=0; i<NUM; i++){
		command[i]=NULL;
		argument[i]=NULL;
		substitute[i]=NULL;
	}
	
	while(fgets(commandBuf, BUF_SCRIPT, script)!=NULL){
		if(commandBuf[strlen(commandBuf)-1]=='\n'){
			commandBuf[strlen(commandBuf)-1]='\0';
		}
		
		//parse the line
		//the pipe variable fd cannot be determined before parsing
		//so fd cannot be decleared at the top
	    commandCount=parse(command, commandBuf, "|");
	    int fd[commandCount][2];
	    
	    //deal with each command in a line
	    for(int i=0; i<commandCount; i++){
	    	
	    	//parse each command
	    	//if there is substitute, we need pretreat the command
	    	strncpy(argumentBuf, command[i], BUF_SCRIPT);
	    	if(strstr(command[i], "(")!=NULL){
	    		pretreat(argumentBuf);
			}
	    	argumentCount=parse(argument, argumentBuf, " ");
	    	
	    	//clean the array
	    	for(int j=argumentCount; j<NUM; j++){
	    		argument[j]=NULL;
			}
			
			//each command need a child process to run, thus need a pipe between child and parent
			if(pipe(fd[i])<0){
    	        fprintf(stderr, "the %d pipe error\n", i);
	        }
	        
	        //check the redirection
			check1=checkInput(argument, argumentCount); 
			check2=checkOutput(argument, argumentCount);
			
			//no input redirection, no pipe, no substitute
			if(substituteControl==0&&check1==-1&&check2==-1&&i==0&&i==commandCount-1){
				single(fd, argument);
			}
			
			//no input redirection, no pipe, only substitute
			if(substituteControl==1&&check1==-1&&check2==-1&&i==0&&i==commandCount-1){
				singleSubstitute(fd, argument, substitute);
			}
			
			//no input redirection, no substitute, has pipe
            if(substituteControl==0&&check1==-1&&i==0&&i<commandCount-1){
				firstNoInAndPipe(fd, argument);
			}
			
			//no input redirection, has substitute, has pipe 
			if(substituteControl==1&&check1==-1&&i==0&&i<commandCount-1){
				firstSubstitute(fd, argument, substitute);
			}
			
			//input redirection, no pipe
			if(check1!=-1&&i==0&&i==commandCount-1){
				inputNoPipe(fd, argument, check1);
			}
			
			//input redirection and pipe
			if(check1!=-1&&i==0&&i<commandCount-1){
				inputAndPipe(fd, argument, check1);
			}
			
			//command in the middle
			if(i>0&&i<commandCount-1){
				middle(fd, argument, i);
			}
			
			//last command no output redirection
			if(check2==-1&&i==commandCount-1&&i>0){
				lastNoOutFile(fd, argument, i);
			}
			
			//last command has output redirection
			if(check2!=-1&&i==commandCount-1&&i>0){
				lastAndOutFile(fd, argument, i, check2);
			}
			
			//the last also first command with output redirection
            if(check2!=-1&&i==commandCount-1&&i==0){
				lastAndOutNoPipe(fd, argument, i, check2);
			}
		}
		
		//go back to the parent process
	    if(getpid()==parentID){
		    for(int i=0; i<commandCount; i++){
	    	    close(fd[i][1]);
		    }
		    sleep(1);
		    for(int i=0; i<commandCount; i++){
	    	    close(fd[i][0]);
		    }
		    for(int i=0; i<commandCount+substituteControl; i++){
        	    if(wait(NULL)<0){
        	        fprintf(stderr, "wait error\n");
		        }  
		    }
	    }
	    substituteControl=0; //finish one line, clean variable
    }
    printf("end of the script\n");
    return 0;
}

//if there is a substitute, pretreat the command
void pretreat(char buf[]){
	int start;
	int end;
	int bufLen;
	for(int i=0; i<BUF_SCRIPT; i++){
		substituteBuf[i]='\0';
	}
	
	//find the substitute, copy it to its buffer
	bufLen=strlen(buf);
	for(int i=0; i<bufLen; i++){
		if(buf[i]=='('){
			start=i;
		}
		if(buf[i]==')'){
			end=i;
		}
	}
	for(int i=0; i<end-start-1; i++){
		substituteBuf[i]=buf[i+start+1];
	}
	
	//parse the substitute content
	substituteCount=parse(substitute, substituteBuf, " ");
	for(int i=substituteCount; i<NUM; i++){
	    substitute[i]=NULL;
	}
	
	//remove the substitute from the command, otherwise " " will not correctly parse the command
	substituteControl=1;
	for (int i=start-1; i<=end; i++){
		buf[i]=' ';
	}
}

//string token to decompose commands etc
int parse(char * a[], char buf[], char *c){
	int count=0;
	char *args=strtok(buf, c);
	while(args!=NULL){
		a[count]=args;
		count++;
		args=strtok(NULL, c);
	}
	return count;
}

//check < redirection
int checkInput(char *part[], int count){
	int position=-1;
	for(int i=0; i<count; i++){
		if(strcmp(part[i],"<")==0){
			position=i;
		}
	}
	return position;
}

////check > redirection
int checkOutput(char *part[], int count){
	int position=-1;
	for(int i=0; i<count; i++){
		if(strcmp(part[i],">")==0){
			position=i;
		}
	}
	return position;
}

//only 1 command, easy, close pipe the exec
void single(int fd[][2], char* a[]){
    pid_t cid=fork();
    if(cid<0){
        fprintf(stderr, "command 0 fork error\n");
    }
    if(cid==0){
		close(fd[0][0]);
		close(fd[0][1]);
        execvp(a[0], a);
    }
}

//no input redirection, has pipe
void firstNoInAndPipe(int fd[][2], char* a[]){
	pid_t cid=fork();
    if(cid<0){
        fprintf(stderr, "command 0 fork error\n");
    }
    if(cid==0){
		close(fd[0][0]);
		if(fd[0][1]!=STDOUT_FILENO){
			if(dup2(fd[0][1], STDOUT_FILENO)!=STDOUT_FILENO){
			    fprintf(stderr, "command 0 stdout dup2 error\n");
		    }
		}
	    close(fd[0][1]);
        execvp(a[0], a);
    }
}

//no input redirection, no pipe, only substitute
void singleSubstitute(int fd[][2], char* a[], char* b[]){
	int fifo;
	pid_t cid1;
	pid_t cid2;
	
	//make the fifo
	mkfifo("fifo", S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	
	//process to run substitute part, output to the fifo	
	cid1=fork();
	if(cid1<0){
        fprintf(stderr, "command 0 fork error\n");
    }
    if(cid1==0){
    	close(fd[0][0]);
    	close(fd[0][1]);
    	fifo=open("fifo", O_WRONLY); 	
    	if(fifo!=STDOUT_FILENO){
			if(dup2(fifo, STDOUT_FILENO)!=STDOUT_FILENO){
			    fprintf(stderr, "command 0 substitute out dup2 error\n");
		    }
		}
		close(fifo);
		execvp(b[0], b);
	}
	
	//process to run the first command, read from the fifo
	cid2=fork();
	if(cid2<0){
        fprintf(stderr, "command 0 fork error\n");
    }
    if(cid2==0){
    	close(fd[0][0]);
    	close(fd[0][1]);
    	fifo=open("fifo", O_RDONLY);
    	if(fifo!=STDIN_FILENO){
			if(dup2(fifo, STDIN_FILENO)!=STDIN_FILENO){
			    fprintf(stderr, "command 0 substitute in dup2 error\n");
		    }
		}
		close(fifo);
		execvp(a[0], a);
	}
}

//no input redirection, has substitute, has pipe
void firstSubstitute(int fd[][2], char* a[], char* b[]){
	int fifo;
	pid_t cid1;
	pid_t cid2;
	
	//make the fifo
	mkfifo("fifo", S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	
	//process to run substitute part, output to the fifo
	cid1=fork();
	if(cid1<0){
        fprintf(stderr, "command 0 fork error\n");
    }
    if(cid1==0){
    	close(fd[0][0]);
    	close(fd[0][1]);
    	fifo=open("fifo", O_WRONLY);
    	if(fifo!=STDOUT_FILENO){
			if(dup2(fifo, STDOUT_FILENO)!=STDOUT_FILENO){
			    fprintf(stderr, "command 0 substitute out dup2 error\n");
		    }
		}
		close(fifo);
		execvp(b[0], b);
	}
	
	//process to run the first command, read from the fifo, and write to the pipe
	cid2=fork();
	if(cid2<0){
        fprintf(stderr, "command 0 fork error\n");
    }
    if(cid2==0){
    	fifo=open("fifo", O_RDONLY);
    	if(fifo!=STDIN_FILENO){
			if(dup2(fifo, STDIN_FILENO)!=STDIN_FILENO){
			    fprintf(stderr, "command 0 substitute in dup2 error\n");
		    }
		}
		close(fifo);
		close(fd[0][0]);
	    if(fd[0][1]!=STDOUT_FILENO){
			if(dup2(fd[0][1], STDOUT_FILENO)!=STDOUT_FILENO){
			    fprintf(stderr, "command 0 stdout dup2 error\n");
		    }
		}
	    close(fd[0][1]);
		execvp(a[0], a);
	}
}


//input redirection, no pipe
void inputNoPipe(int fd[][2], char* a[], int position){
	int in;
    pid_t cid=fork();
    if(cid<0){
        fprintf(stderr, "command 0 fork error\n");
    }
    if(cid==0){
	    in=open(a[position+1], O_RDONLY);
	    if(in!=STDIN_FILENO){
		    if(dup2(in, STDIN_FILENO)!=STDIN_FILENO){
			    fprintf(stderr, "input file dup2 error\n");
		    }
		}
		close(in);
	    close(fd[0][0]);
	    close(fd[0][1]);
		a[position]=NULL;
	    a[position+1]=NULL;	    
        execvp(a[0], a);
    }
}

//input redirection, has pipe
void inputAndPipe(int fd[][2], char* a[], int position){
	int in;
    pid_t cid=fork();
    if(cid<0){
        fprintf(stderr, "command 0 fork error\n");
    }
    if(cid==0){
	    in=open(a[position+1], O_RDONLY);
	    if(in!=STDIN_FILENO){
		    if(dup2(in, STDIN_FILENO)!=STDIN_FILENO){
			    fprintf(stderr, "input file dup2 error\n");
		    }
		}
		close(in);
	    close(fd[0][0]);
	    if(fd[0][1]!=STDOUT_FILENO){
			if(dup2(fd[0][1], STDOUT_FILENO)!=STDOUT_FILENO){
			    fprintf(stderr, "command 0 stdout dup2 error\n");
		    }
		}
	    close(fd[0][1]);
	    a[position]=NULL;
	    a[position+1]=NULL;	
        execvp(a[0], a);
    }
}

//command in the middle
//need to have input from parent pipe and output to pipe belongs to it
void middle(int fd[][2], char* a[], int i){
	pid_t cid=fork();
	if(cid<0){
        fprintf(stderr, "command %d fork error\n", i);
    }
    if(cid==0){
    	for(int j=0; j<i-1; j++){
    		close(fd[j][0]);
	        close(fd[j][1]);
		}
		close(fd[i-1][1]);
		if(fd[i-1][0]!=STDIN_FILENO){
		    if(dup2(fd[i-1][0], STDIN_FILENO)!=STDIN_FILENO){
			    fprintf(stderr, "command %d stdin dup2 error\n", i);
		    }
		}
		close(fd[i-1][0]);
		close(fd[i][0]);
	    if(fd[i][1]!=STDOUT_FILENO){
			if(dup2(fd[i][1], STDOUT_FILENO)!=STDOUT_FILENO){
			    fprintf(stderr, "command %d stdout dup2 error\n", i);
		    }
		}
	    close(fd[i][1]);
	    execvp(a[0], a);
	}
}

//last command no output redirection
void lastNoOutFile(int fd[][2], char* a[], int i){
	pid_t cid=fork();
	if(cid<0){
        fprintf(stderr, "command %d fork error\n", i);
    }
    if(cid==0){
    	for(int j=0; j<i-1; j++){
    		close(fd[j][0]);
	        close(fd[j][1]);
		}
		close(fd[i-1][1]);
		if(fd[i-1][0]!=STDIN_FILENO){
		    if(dup2(fd[i-1][0], STDIN_FILENO)!=STDIN_FILENO){
			    fprintf(stderr, "command %d stdin dup2 error\n", i);
		    }
		}
		close(fd[i-1][0]);
		close(fd[i][0]);
		close(fd[i][1]);
		execvp(a[0], a);
	}
}

//last command has output redirection
void lastAndOutFile(int fd[][2], char* a[], int i, int position){
	int out;
	pid_t cid=fork();
	if(cid<0){
        fprintf(stderr, "command %d fork error\n", i);
    }
    if(cid==0){
    	for(int j=0; j<i-1; j++){
    		close(fd[j][0]);
	        close(fd[j][1]);
		}
		close(fd[i-1][1]);
		if(fd[i-1][0]!=STDIN_FILENO){
		    if(dup2(fd[i-1][0], STDIN_FILENO)!=STDIN_FILENO){
			    fprintf(stderr, "command %d stdin dup2 error\n", i);
		    }
		}
		close(fd[i-1][0]);
		close(fd[i][0]);
		close(fd[i][1]);
		out=open(argument[position+1], O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	    if(out!=STDOUT_FILENO){
		    if(dup2(out, STDOUT_FILENO)!=STDOUT_FILENO){
			    fprintf(stderr, "command %d stdout dup2 error\n", i);
		    }
	    }
	    close(out);
	    a[position]=NULL;
	    a[position+1]=NULL;
        execvp(a[0], a); 
    }
}

//the last also first command with output redirection
void lastAndOutNoPipe(int fd[][2], char* a[], int i, int position){
	int out;
	pid_t cid=fork();
	if(cid<0){
        fprintf(stderr, "command %d fork error\n", i);
    }
    if(cid==0){
    	close(fd[0][0]);
		close(fd[0][1]);
		out=open(argument[position+1], O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	    if(out!=STDOUT_FILENO){
		    if(dup2(out, STDOUT_FILENO)!=STDOUT_FILENO){
			    fprintf(stderr, "command %d stdout dup2 error\n", i);
		    }
	    }
	    close(out);
	    a[position]=NULL;
	    a[position+1]=NULL;
        execvp(a[0], a);
	}
}
