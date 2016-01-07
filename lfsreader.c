// LFSSSSSSSSSSS written by allen rand, brianna smith
// LFSSSSSSSSSSS written by allen rand, brianna smith

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>
#include <libgen.h>


#define INODEPIECES 256
#define MFS_DIRECTORY 0
#define MFS_REGULAR_FILE 1
#define BSIZE 4096

typedef struct __attribute__((__packed__)) __checkpoint__ {
        int size;
        int iMapPtr[INODEPIECES];
} checkpoint;

typedef struct __attribute__((__packed__)) __dirEnt__   {
        char name[60];
        int inum;
} dirEnt;

typedef struct __attribute__((__packed__)) __inode__    {
        int size;
        int type;
        int ptr[14];
} inode;

typedef struct __attribute__((__packed__)) __inodeMap__ {
        int inodePtr[16];
} inodeMap;




int main(int argc, char *argv[]){
        //checkpoint->imap->root->root contents...

        //counters
        int i,x,w,z;//counters
        void * ptr;
        void * head;
        char * command;
        char * pathname;
        checkpoint * chckpnt;
        inodeMap * imap;
        inode * node;
        dirEnt * dirent;
        struct stat st;
        int ls, cat;
        ls = 1;
        cat = 1;
        //take image files in as input from the command line.
        if(argc != 4){
                printf("Error1!\n");
                return -1;
        }

        // check the command given
        command = argv[1];

        if( strcmp(command, "ls") != 0 ){
                ls = -1;
        }
        if(strcmp(command, "cat") != 0){
                cat = -1;
        }
        if(ls == -1 && cat == -1){
                printf("Error!2\n");
        }
        // open the file
        int fd = open(argv[3], O_RDWR);

        // make sure the file opened correctly
        if (fd == -1){
                printf("file open error3\n");
                return -1;
        }


        // get path name
        pathname = malloc(sizeof(char)*strlen(argv[2]));
        pathname = argv[2];
        fstat(fd, &st);
        // load the file into memory
        head = mmap(NULL, st.st_size , PROT_READ, MAP_PRIVATE, fd, 0);

        if(head == MAP_FAILED){

                return -1;
        }

        //get checkpoint
        chckpnt = head;
        //go to imap
        imap = head + chckpnt->iMapPtr[0];
        //get root directory inode
        node = head + imap->inodePtr[0];
        //root directory contents
        dirent = head + node->ptr[0];


        //parse the path 
        char * path;
        path = strtok(pathname, "/");

        //begin at root and step down the path to desired root or file
        while (path != NULL){

                for(i = 0; i < 14; i++){

                        if (strcmp(dirent->name, path) == 0){
                                //found what we are looking for goto the inode or
                                // its directory contents
                                node = head + imap->inodePtr[dirent->inum];
                                dirent = head + node->ptr[0];
                                break;
                        }
                        dirent = dirent + 1;
                }
                path = strtok(NULL, "/");
        }
        // node == inode for the directory or file we want
        // dirent == the directory contents for ls
        if(ls == 1){
                //command was an ls
                //if the node is a file we cannot ls
                if(node->type == 1){
                        printf("Error!\n");
                        return -1;
                }
                // do ls command
                for(i = 0; i < 14; i++){
                        // see if its a file or directory for output format
                        char * file;
                        file = strchr(dirent->name, '.');
                        //if it is the . or .. directories print them with /
                        // otherwise if it contains a . it must me a file
                        if(strcmp(dirent->name, ".") == 0 || strcmp(dirent->name, "..") == 0){
                                printf("%s/\n", dirent->name);
                        }else{
                                if(dirent->inum != -1 && file == NULL){
                                        printf("%s/\n", dirent->name);
                                }
                                else if(dirent->inum != -1){
                                        printf("%s\n", dirent->name);
                                }
                        }
                        dirent = dirent + 1;
                }
        }
        else if(cat == 1){
                // cat command
                // if the node is a directory we cannot do cat
                if(node->type == 0){
                        printf("Error!\n");
                        return -1;
                }
                //print out the file
                int size = node->size;
                char * c;
                c = (void * )dirent;
                printf("%s",c);


        }
        //unmap memory and close the file
        if (munmap(head, st.st_size) == -1) {
                printf("Error!\n");
        }
        close(fd);


}// END MAIN