//-----------------------------------------
// NAME: Xing Zhou
// STUDENT NUMBER: 7869781
// COURSE: COMP 3430, SECTION: A01
// INSTRUCTOR: Robert Guderian
// ASSIGNMENT: assignment 4, QUESTION: question 1
// REMARKS: read exfat file
//-----------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>

#pragma pack(1)
#pragma pack(push)
typedef struct MAIN_BOOT_SECTOR {
	uint8_t jump_boot[3];
	char fs_name[8];
	uint8_t must_be_zero[53];
	uint64_t partition_offset;
	uint64_t volume_length;
	uint32_t fat_offset;
	uint32_t fat_length;
	uint32_t cluster_heap_offset;
	uint32_t cluster_count;
	uint32_t first_cluster_of_root_directory;
	uint32_t volume_serial_number;
	uint16_t fs_revision;
	uint16_t fs_flags;
	uint8_t bytes_per_sector_shift;
	uint8_t sectors_per_cluster_shift;
	uint8_t number_of_fats;
	uint8_t drive_select;
	uint8_t percent_in_use;
	uint8_t reserved[7];
	uint8_t bootcode[390];
	uint16_t boot_signature;
} main_boot_sector;
#pragma pack(pop)

//struct for direcroty entry
//control to calculate several variables, for example file name entry count
//attrubite to determine directory or regular file 
#pragma pack(1)
#pragma pack(push)
typedef struct DIRECTORY_ENTRY {
	uint8_t type;
	uint8_t control;
	uint8_t skip1[2];
	uint16_t attribute;
	uint8_t skip2[14];
	uint32_t first_cluster;
	uint64_t data_length;
} entry;
#pragma pack(pop)

//file name entry struct, also used for volumn label entry
#pragma pack(1)
#pragma pack(push)
typedef struct NAME_STRING {
	uint8_t type;
	uint8_t count;
	uint16_t string[15];
} name;
#pragma pack(pop)

//3 commands
void info(main_boot_sector*, int);
void list(main_boot_sector*, int, uint32_t, unsigned int);
void get(main_boot_sector*, int, char*);
//help functions
int checkName(main_boot_sector*);
char *unicode2ascii(uint16_t*, uint8_t);

int main(int argc, char *argv[]) {
	char *c1="info";
	char *c2="list";
	char *c3="get";
	char *image=NULL;
	char *command=NULL;
	char *path=NULL;
	int fd=-1;
	main_boot_sector *mbs=NULL;
	
	if(argc<3){
		fprintf(stderr, "not enough arguments\n");
		exit(0);
	}
	image=argv[1];
	fd=open(image, O_RDONLY);
	if(fd<0){
		fprintf(stderr, "image file error\n");
		exit(0);
	}
	
	//check exfat file validity
	mbs=(main_boot_sector*)malloc(sizeof(main_boot_sector));
	read(fd, mbs, sizeof(main_boot_sector));
	if(checkName(mbs)!=1){
		printf("invalid file system name %s\n", mbs->fs_name);
	    close(fd);
		exit(0);
	}
	if(mbs->boot_signature!=0xaa55) {
		printf("invalid boot sector signature %x\n", mbs->boot_signature);
	    close(fd);
		exit(0);
	}
	
	command=argv[2];
	if(strcmp(command, c1)==0){
		info(mbs, fd);
	}
	if(strcmp(command, c2)==0){
		list(mbs, fd, mbs->first_cluster_of_root_directory, 0);
	}
	if(strcmp(command, c3)==0){
		if(argc>=4){
			path=argv[3];
			get(mbs, fd, path);
		}
		else{
			fprintf(stderr, "not enough arguments for file path\n");
		    exit(0);
		}
	}
	//in case invalid command
	if(strcmp(command, c1)!=0&&strcmp(command, c2)!=0&&strcmp(command, c3)!=0){
		fprintf(stderr, "invalid command: %s\n", command);
		exit(0);
	}
	
	free(mbs);
	close(fd);
	return 0;
}

//the info function
void info(main_boot_sector *mbs, int fd){
	//get the active fat information
	uint16_t temp=mbs->fs_flags & 0x0001;
	uint8_t active=(uint8_t)temp;
	
	//pre calculate all the offset
	unsigned long clusterOffset=mbs->cluster_heap_offset*(1 << mbs->bytes_per_sector_shift);
	unsigned long fatOffset=(mbs->fat_offset+mbs->fat_length*active)*(1 << mbs->bytes_per_sector_shift);	
	unsigned int clusterSize=(1 << mbs->sectors_per_cluster_shift)*(1 << mbs->bytes_per_sector_shift);
	unsigned int entryPerCluster=clusterSize/32;
	
	//buffer to store a whole cluster and then read entries	
	entry entries[entryPerCluster];
	//need extra struct for file name parsing
	name nameString;
	
	//boolean for search the label entry and bitmap entry
	int label=0;
	int map=0;
	char *labelName=NULL;
	
	//bitmap parameters and bitmap read buffer, more details below
	unsigned int mapFirst=0;
	unsigned int mapLength=0;
	unsigned int use=0;
	unsigned int limit=0;
	unsigned int remain=0;
	uint8_t bitmap[clusterSize];
	
	//read the root directory
	uint32_t nextCluster=mbs->first_cluster_of_root_directory;
	
	//keep reading when not end of root and lable or bitmap entry not found
	//in case no chain attribute is set 1, then 0 can be treated as end of chain mark 
	while((nextCluster!=0xffffffff&&nextCluster!=0x00000000)&&(label!=1||map!=1)){
		lseek(fd, clusterOffset+(nextCluster-2)*clusterSize, SEEK_SET);
		
		//deal with a cluster each time
	    read(fd, entries, clusterSize);
	    
	    //0x83 is type for volumn lable and 0x81 is type for bitmap
	    //when the right entry found, record the necessary information
		for(unsigned int i=0; i<entryPerCluster; i++){
		    if(entries[i].type==0x83){
			    label=1;
			    //use file name struct to parse name
			    memcpy(&nameString, &entries[i], 32);
			    labelName=unicode2ascii(nameString.string, nameString.count);
		    }
		    if(entries[i].type==0x81&&entries[i].control==active){
			    map=1;
			    mapFirst=entries[i].first_cluster;
			    mapLength=entries[i].data_length;
		    }
	    }
	    
	    //go to next cluster in the chain
	    lseek(fd, fatOffset+nextCluster*4, SEEK_SET);
	    read(fd, &nextCluster, 4);
	}
	
	//bitmap must exist
	assert(map==1);
	//read the bitmap
	nextCluster=mapFirst;
	remain=mapLength;
	while(nextCluster!=0xffffffff&&nextCluster!=0x00000000){
		lseek(fd, clusterOffset+(nextCluster-2)*clusterSize, SEEK_SET);
	    read(fd, bitmap, clusterSize);
	    
	    //if the cluster is not the last one, then all data in the cluster should be considered
	    if(remain>=clusterSize){
	    	limit=clusterSize;
		}
		//the last cluster, not all data should be considered, use remain variable to control
		else{
			limit=remain;
		}
		
		//only limit bytes of data should be considered when calculate set/unset bits
	    for(unsigned int i=0; i<limit; i++){
		    use+=__builtin_popcount(bitmap[i]);
	    }
	    //update remain
	    remain-=limit;
	    lseek(fd, fatOffset+nextCluster*4, SEEK_SET);
	    read(fd, &nextCluster, 4);
	}
	
	//print the information
	//volumn label can be none
	if(label==0){
		printf("No Volumn Lable\n");
	}
	if(label==1){
		printf("Volumn Lable is %s.\n", labelName);
	}
	printf("Serial Number is %u.\n", mbs->volume_serial_number);
	printf("Free Space is %.0f KB.\n", (mbs->cluster_count-use)*(double)clusterSize/1024);
	printf("Cluster Size: %u sector(s), %u Bytes, %.2f KB\n", 1 << mbs->sectors_per_cluster_shift, clusterSize, (double)clusterSize/1024);
	free(labelName);
}

//the list function
void list(main_boot_sector *mbs, int fd, uint32_t first, unsigned int depth){
	//get the active fat information
	uint16_t temp=mbs->fs_flags & 0x0001;
	uint8_t active=(uint8_t)temp;
	
	//pre calculate all the offset
	unsigned long clusterOffset=mbs->cluster_heap_offset*(1 << mbs->bytes_per_sector_shift);
	unsigned long fatOffset=(mbs->fat_offset+mbs->fat_length*active)*(1 << mbs->bytes_per_sector_shift);
	unsigned int clusterSize=(1 << mbs->sectors_per_cluster_shift)*(1 << mbs->bytes_per_sector_shift);
	unsigned int entryPerCluster=clusterSize/32;
		
    //buffer to store a whole cluster and then read entries	
	entry entries[entryPerCluster];
	//need extra struct for file name parsing
	name nameString;
	
	//check whether a directory or a file
	uint16_t directory=0;
	//first cluster of a directory
	uint32_t dirFirst=0;
	
	//varibales to parse and concatenate file name, more details below
	int nameEntry=0;
	int nameControl=0;
	char *fileName=NULL;
	
	//start from the root, first is set to the first cluster of root when calling this function
	//f and 0 both can be treated end of chain as explained in the info function
	uint32_t nextCluster=first;
	while(nextCluster!=0xffffffff&&nextCluster!=0x00000000){
	    lseek(fd, clusterOffset+(nextCluster-2)*clusterSize, SEEK_SET);
	    
	    //deal with a cluster each time
	    read(fd, entries, clusterSize);
	    for(unsigned int i=0; i<entryPerCluster; i++){	
	    	//the file entry to get the directory attribute and number of name entry
		    if(entries[i].type==0x85){
		    	directory=entries[i].attribute & 0x0010;
		    	nameEntry=entries[i].control-1;
		        nameControl=nameEntry;
			}
			
			//the extension entry to get the directory first cluster information
		    if(entries[i].type==0xc0){
		        dirFirst=entries[i].first_cluster;
		    }
		    
		    //since these 3 kinds of entry are sequencial, once relevant information is updated by previous entry
		    //the following entry can rely on that, thus name entry can rely on previous entry's information to print name
		    if(entries[i].type==0xc1){
		    	
		    	//the first name entry, print the depth and file attribute before it
		    	if(nameControl==nameEntry){
		    		for(unsigned int i=0; i<depth; i++){
		    			printf("-");
					}
		    		if(directory==0x0010){
		    			printf("Directory: ");
					}
					if(directory==0x0000){
						printf("File: ");
					}
				}
				
				//normal name parsing process as explained before
				//it's convenient to always use 15 as string length here since unused space in name entry is null value
				//will not do change the desired outcome and can reduce complex if check
			    memcpy(&nameString, &entries[i], 32);
			    fileName=unicode2ascii(nameString.string, 15);
			    printf("%s", fileName);
			    free(fileName);
			    nameControl--;
			    
			    //the last name entry
			    if(nameControl==0){
			    	printf("\n");
			    	
			    	//if it is a directory, recursively to print stuff in the directory
			    	//initial depth is 0 when calling this function, root directory has depth 0
			    	if(directory==0x0010){
					    depth++;
					    list(mbs, fd, dirFirst, depth);
					    depth--;
				    }
				}
		    }
	    }	    
	    //go to next cluster in the chain
	    lseek(fd, fatOffset+nextCluster*4, SEEK_SET);
	    read(fd, &nextCluster, 4);
	}
}

//the get function
void get(main_boot_sector *mbs, int fd, char* path){
	printf("get command begin\n");
	
	//get the active fat information
	uint16_t temp=mbs->fs_flags & 0x0001;
	uint8_t active=(uint8_t)temp;	
	
    //pre calculate all the offset
	unsigned long clusterOffset=mbs->cluster_heap_offset*(1 << mbs->bytes_per_sector_shift);
	unsigned long fatOffset=(mbs->fat_offset+mbs->fat_length*active)*(1 << mbs->bytes_per_sector_shift);
	uint64_t clusterSize=(1 << mbs->sectors_per_cluster_shift)*(1 << mbs->bytes_per_sector_shift);
	unsigned int entryPerCluster=clusterSize/32;
	
	//start from root to search the desired file
	uint32_t nextCluster=mbs->first_cluster_of_root_directory;
	
	//buffer to copy file
	uint8_t copyBuf[clusterSize];
	//buffer to store a whole cluster and then read entries	
	entry entries[entryPerCluster];
	//need extra struct for file name parsing
	name nameString;
	
	//file parameters, more details below
	uint32_t firstCluster=0;
	uint64_t dataLength=0;
	
	//parameters to copy file, more details below
	uint64_t remain=0;
	uint64_t limit=0;
	
	//parameters to parse file name and compare it with the absolute path, more details below
	int nameEntry=0;
	int nameControl=0;
	char *fileName=NULL;
	int nameLength=0;
	char name[256];
	
	//boolean to track whether file found or not
	int found=0;
	//while loop count
	unsigned int i=0;
	
	//output file descriptor
	int out=-1;
	
	//variables to store information in the path
	char *file=NULL;
	char *copy=NULL;	
	int length=strlen(path);	
	//the buffer to parse the absolute path
	char buf[length+1];
	
	//initial/clean the array variable
	for(int j=0; j<256; j++){
		name[j]='\0';
	}
	
	//load the path
	strncpy(buf, path, length+1);
	file=strtok(buf, "/");
	
	//while there are more directory/file in the path
	while(file!=NULL){
		//for thr purpose to copy the desired file, since variable file will be null in the end
		copy=file;
		
		//while not found the desired directory or file and not end of fat chain
		//again, 0 can be treated as end of chain mark as explained above
		while(found==0&&(nextCluster!=0xffffffff&&nextCluster!=0x00000000)){
	        lseek(fd, clusterOffset+(nextCluster-2)*clusterSize, SEEK_SET);
	        
	        //deal with a cluster each time
	        read(fd, entries, clusterSize);
	        while(found==0&&i<entryPerCluster){
	        	
	        	//the file entry to get the number of name entry
		        if(entries[i].type==0x85){
		    	    nameEntry=entries[i].control-1;
		            nameControl=nameEntry;
			    }
			    
			    //the extension entry to get the directory first cluster and data length information
		        if(entries[i].type==0xc0){
		            firstCluster=entries[i].first_cluster;
		            dataLength=entries[i].data_length;
		        }
		        
		        //since these 3 kinds of entry are sequencial, once relevant information is updated by previous entry
                //the following entry can rely on that, thus name entry can rely on previous entry's information to deal with name
		        if(entries[i].type==0xc1){
		        	
		        	//normal name parsing process as explained before
                    //it's convenient to always use 15 as string length here since unused space in name entry is null value
                    //will not do change the desired outcome and can reduce complex if check
			        memcpy(&nameString, &entries[i], 32);
			        fileName=unicode2ascii(nameString.string, 15);
			        
			        //we need a name buffer to concatenate all file name entry
			        strncpy(&name[nameLength], fileName, strlen(fileName));
			        nameLength+=strlen(fileName);
			        free(fileName);
			        nameControl--;
			        
			        //when we complete to process all name entry and concatenate them in name buffer
			        if(nameControl==0){
			        	//then we can compare the specific name with desired name in the path
						if(strcmp(name, file)==0){
							found=1;
						}
						//clean name buffer for next file entry set
						nameLength=0;
						for(int j=0; j<256; j++){
			        		name[j]='\0';
						}
				    }
		        }
		        i++;
	        }
	        //a cluster finish, go to next in the chain if there is one
	        i=0;
	        lseek(fd, fatOffset+nextCluster*4, SEEK_SET);
	        read(fd, &nextCluster, 4);
	    }
	    
	    //if desired file is not in a specific derictory
	    if(found==0){
	    	printf("the absolute path does not exist\n");
	    	exit(0);
		}
		
		//if a desired directory is found, its first cluster has been recorded
		//we use this information to go into this specific directory, thus we go down the absolute path
		nextCluster=firstCluster;
		//clean found boolean
		found=0;
		//get next desired directory or file until the bottom of the absolute path
		file=strtok(NULL, "/");
	}
	
	//finally we reach the bottom of the path and have the information of the file to be copied
	out=open(copy, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	remain=dataLength;
	
	//read the file until end of chain
	while(nextCluster!=0xffffffff&&nextCluster!=0x00000000){
		lseek(fd, clusterOffset+(nextCluster-2)*clusterSize, SEEK_SET);
		
		//deal with a cluster each time
	    read(fd, copyBuf, clusterSize);
	    
	    //if the cluster is not the last one, then all data in the cluster should be considered
	    if(remain>=clusterSize){
	    	limit=clusterSize;
		}
		//the last cluster, not all data should be considered, use remain variable to control
		else{
			limit=remain;
		}
		//copy the data in the cluster
		write(out, copyBuf, limit);
	    remain-=limit;
	    
	    //go to next cluster in the chain
	    lseek(fd, fatOffset+nextCluster*4, SEEK_SET);
	    read(fd, &nextCluster, 4);
	}
	close(out);
	printf("get command end, ls command to see the file\n");
}

//make sure the name of the input file is "exfat   "
int checkName(main_boot_sector *mbs){
	int check=1;
	char name[8]; //the only valid exfat name
	name[0]='E';
	name[1]='X';
	name[2]='F';
	name[3]='A';
	name[4]='T';
	name[5]=' ';
	name[6]=' ';
	name[7]=' ';
	for(int i=0; i<8; i++){
		if(mbs->fs_name[i]!=name[i]){
			check=0;
		}
	}
	return check;
}

//the provided unicode to ascii switch function
char *unicode2ascii(uint16_t *unicode_string, uint8_t length){
    assert(unicode_string!=NULL);
    assert(length>0);
    char *ascii_string = NULL;
    if(unicode_string!=NULL&&length>0){
        //+1 for a NULL terminator
        ascii_string=calloc(sizeof(char), length+1); 
        if(ascii_string){
            //strip the top 8 bits from every character in the 
            //unicode string
            for(uint8_t i=0; i<length; i++){
                ascii_string[i]=(char)unicode_string[i];
            }
            //stick a null terminator at the end of the string.
            ascii_string[length] ='\0';
        }
    }
    return ascii_string;
}
