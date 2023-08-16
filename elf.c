//-----------------------------------------
// NAME: Xing Zhou
// STUDENT NUMBER: 7869781
// COURSE: COMP 3430, SECTION: A01
// INSTRUCTOR: Robert Guderian
// ASSIGNMENT: assignment 1, QUESTION: question 1
// REMARKS: read the elf file
//-----------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#pragma pack(push)
#pragma pack(1)
typedef struct IDENTIFIER{
    unsigned char signature[4];
    uint8_t bit;
    uint8_t endian;
    unsigned char skip1[1];
    uint8_t abi;
    unsigned char skip2[8];
    uint16_t type;
    uint16_t isa;
    unsigned char skip3[4];
} identifier;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct ELF_HEADER_64{
    uint64_t entry;
    uint64_t phoff;
    uint64_t shoff;
    unsigned char skip[6];
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
} elf64;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct ELF_HEADER_32{
    uint32_t entry;
    uint32_t phoff;
    uint32_t shoff;
    unsigned char skip[6];
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
} elf32;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct PROGRAM_HEADER_64{
    uint32_t type;
    unsigned char skip1[4];
    uint64_t offset;
    uint64_t address;
    unsigned char skip2[8];
    uint64_t filesz;
} p64;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct PROGRAM_HEADER_32{
    uint32_t type;
    uint32_t offset;
    uint32_t address;
    unsigned char skip[4];
    uint32_t filesz;  
} p32;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct SECTION_HEADER_64{
        uint32_t name;
    uint32_t type;
    unsigned char skip[8];
    uint64_t address;
    uint64_t offset;
    uint64_t size;
} s64;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct SECTION_HEADER_32{
	uint32_t name;
    uint32_t type;
    unsigned char skip[4];
    uint32_t address;
    uint32_t offset;
    uint32_t size;  
} s32;
#pragma pack(pop)

void PrintComInfo(identifier*);
void PrintELF64(elf64*);
void PrintELF32(elf32*);
void PrintPH64(p64*, unsigned char[], int, int);
void PrintPH32(p32*, unsigned char[], int, int);
void PrintSH64(s64*, unsigned char[], int, int, unsigned char[]);
void PrintSH32(s32*, unsigned char[], int, int, unsigned char[]);
int min(int, int);

int main(int argc, char *argv[]){
	argc+=0;
    char *file=argv[1];
    int fd=open(file, O_RDONLY);
    if(fd<0){
    	printf("Please check input file.\n");
    	exit(1);
	}
    identifier* id=(identifier*)malloc(sizeof(identifier));
    read(fd, id, sizeof(identifier));
    if(id->signature[1]!='E'||id->signature[2]!='L'||id->signature[3]!='F'){
		printf("File is not an elf file.\n");
		free(id);
		close(fd);
		exit(1);
	}
    if(id->bit==2){
        elf64 *elf=(elf64*)malloc(sizeof(elf64));
        read(fd, elf, sizeof(elf64));
        PrintComInfo(id);
        PrintELF64(elf);
        int phNo=elf->phnum;
        int phSize=elf->phentsize;
        p64 *ph=(p64*)malloc(sizeof(p64));
        for(int i=0; i<phNo; i++){
            lseek(fd, elf->phoff+i*phSize, SEEK_SET);
            read(fd, ph, sizeof(p64));
            int bytes=min(32, ph->filesz);
            unsigned char buffer[bytes];
            lseek(fd, ph->offset, SEEK_SET);
            read(fd, buffer, bytes);
            PrintPH64(ph, buffer, bytes, i);
        }
        int shNo=elf->shnum;
        int shSize=elf->shentsize;
        uint64_t nameTable=elf->shoff+shSize*(shNo-1)+24;
        uint64_t nameStart=0;
        uint64_t nameSize=0;
        lseek(fd, nameTable, SEEK_SET);
        read(fd, &nameStart, 8);
        read(fd, &nameSize, 8);
        int size=(int)nameSize;
        char nameBuf[size];
        lseek(fd, nameStart, SEEK_SET);
        read(fd, nameBuf, size);
        s64 *sh=(s64*)malloc(sizeof(s64));
        for(int i=0; i<shNo; i++){
            lseek(fd, elf->shoff+i*shSize, SEEK_SET);
            read(fd, sh, sizeof(s64));
            int bytes=min(32, sh->size);
            unsigned char buffer[bytes];
            lseek(fd, sh->offset, SEEK_SET);
            read(fd, buffer, bytes);
            PrintSH64(sh, buffer, bytes, i, nameBuf);
        }
        free(elf);
        free(ph);
        free(sh);
    }
    if(id->bit==1){
        elf32 *elf=(elf32*)malloc(sizeof(elf32));
        read(fd, elf, sizeof(elf32));
        PrintComInfo(id);
        PrintELF32(elf);
        int phNo=elf->phnum;
        int phSize=elf->phentsize;
        p32 *ph=(p32*)malloc(sizeof(p32));
        for(int i=0; i<phNo; i++){
            lseek(fd, elf->phoff+i*phSize, SEEK_SET);
            read(fd, ph, sizeof(p32));
            int bytes=min(32, ph->filesz);
            unsigned char buffer[bytes];
            lseek(fd, ph->offset, SEEK_SET);
            read(fd, buffer, bytes);
            PrintPH32(ph, buffer, bytes, i);
        }
        int shNo=elf->shnum;
        int shSize=elf->shentsize;
        uint32_t nameTable=elf->shoff+shSize*(shNo-1)+16;
        uint32_t nameStart=0;
        uint32_t nameSize=0;
        lseek(fd, nameTable, SEEK_SET);
        read(fd, &nameStart, 4);
        read(fd, &nameSize, 4);
        int size=(int)nameSize;
        char nameBuf[size];
        lseek(fd, nameStart, SEEK_SET);
        read(fd, nameBuf, size);
        s32 *sh=(s32*)malloc(sizeof(s32));
        for(int i=0; i<shNo; i++){
            lseek(fd, elf->shoff+i*shSize, SEEK_SET);
            read(fd, sh, sizeof(s32));
            int bytes=min(32, sh->size);
            unsigned char buffer[bytes];
            lseek(fd, sh->offset, SEEK_SET);
            read(fd, buffer, bytes);
            PrintSH32(sh, buffer, bytes, i, nameBuf);
        }
        free(elf);
        free(ph);
        free(sh);
    }
    free(id);
    close(fd);
    return 0;
}

void PrintComInfo(identifier* id){
    printf("%c%c%c Header:\n", id->signature[1], id->signature[2], id->signature[3]);
    printf("%d bit\n", id->bit*32);
    if(id->endian==1){
        printf("little endian\n");
    }
    if(id->endian==2){
        printf("big endian\n");
    }
    printf("compiled for 0x%02x(os)\n", id->abi);
    printf("has type 0x%02x\n", id->type);
    printf("compiled for 0x%02x(isa)\n", id->isa);
}

void PrintELF64(elf64* elf){
    printf("entry point address 0x%016lx\n",elf->entry);
    printf("program header table starts at 0x%016lx\n",elf->phoff);
    printf("there are %d program headers, each is %d bytes\n", elf->phnum, elf->phentsize);
    printf("there are %d section headers, each is %d bytes\n", elf->shnum, elf->shentsize);
    printf("the section header string table is %d\n", elf->shstrndx);
}

void PrintELF32(elf32* elf){
    printf("entry point address 0x%08x\n",elf->entry);
    printf("program header table starts at 0x%08x\n",elf->phoff);
    printf("there are %d program headers, each is %d bytes\n", elf->phnum, elf->phentsize);
    printf("there are %d section headers, each is %d bytes\n", elf->shnum, elf->shentsize);
    printf("the section header string table is %d\n", elf->shstrndx);
}

void PrintPH64(p64* ph, unsigned char buffer[], int bytes, int i){
    printf("\nProgram Header #%d:\n", i);
    printf("segment type 0x%08x\n", ph->type);
    printf("virtual address of segment 0x%016lx\n", ph->address);
    printf("size in file %lu bytes\n", ph->filesz);
    printf("first up to 32 bytes starting at 0x%016lx:\n", ph->offset);
    for(int i=0; i<bytes; i++){
        if((i+1)%16!=0&&i!=bytes-1){
            printf("%02x ", buffer[i]);
        }
        if((i+1)%16==0||i==bytes-1){
            printf("%02x\n", buffer[i]);
        }
    }
}

void PrintPH32(p32* ph, unsigned char buffer[], int bytes, int i){
    printf("\nProgram Header #%d:\n", i);
    printf("segment type 0x%08x\n", ph->type);
    printf("virtual address of segment 0x%08x\n", ph->address);
    printf("size in file %d bytes\n", ph->filesz);
    printf("first up to 32 bytes starting at 0x%08x:\n", ph->offset);
    for(int i=0; i<bytes; i++){
        if((i+1)%16!=0&&i!=bytes-1){
            printf("%02x ", buffer[i]);
        }
        if((i+1)%16==0||i==bytes-1){
            printf("%02x\n", buffer[i]);
        }
    }
}

void PrintSH64(s64* sh, unsigned char buffer[], int bytes, int i, unsigned char nameBuf[]){
    printf("\nSection Header #%d:\n", i);
	printf("section name ");
	int j=sh->name;
	while(nameBuf[j]!=0){
	    printf("%c", nameBuf[j]);
		j++;
	}
	printf("\n");
    printf("section type 0x%08x\n", sh->type);
    printf("virtual address of section 0x%016lx\n", sh->address);
    printf("size in file %lu bytes\n", sh->size);
    printf("first up to 32 bytes starting at 0x%016lx:\n", sh->offset);
    for(int i=0; i<bytes; i++){
        if((i+1)%16!=0&&i!=bytes-1){
            printf("%02x ", buffer[i]);
        }
        if((i+1)%16==0||i==bytes-1){
            printf("%02x\n", buffer[i]);
        }
    }
}

void PrintSH32(s32* sh, unsigned char buffer[], int bytes, int i, unsigned char nameBuf[]){
    printf("\nSection Header #%d:\n", i);
	printf("section name ");
	int j=sh->name;
	while(nameBuf[j]!=0){
	    printf("%c", nameBuf[j]);
		j++;
	}
	printf("\n");
    printf("section type 0x%08x\n", sh->type);
    printf("virtual address of section 0x%08x\n", sh->address);
    printf("size in file %d bytes\n", sh->size);
    printf("first up to 32 bytes starting at 0x%08x:\n", sh->offset);
    for(int i=0; i<bytes; i++){
        if((i+1)%16!=0&&i!=bytes-1){
            printf("%02x ", buffer[i]);
        }
        if((i+1)%16==0||i==bytes-1){
            printf("%02x\n", buffer[i]);
        }
    }
}

int min(int a, int b){
    int r=0;
    if(a<=b){
        r=a;
    }
    else{
        r=b;
    }
    return r;
}
