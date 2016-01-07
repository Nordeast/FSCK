// Allen Rand, Brianna Smith
// P3a
// cs 537
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "mymem.h"
#include <string.h>
#include <inttypes.h>
#include <pthread.h>




int init = 0;// make sure mem_init may only be run once
int slabsize;
int mem_size; //holds total size of the memory
struct FreeHeader * nextfithead = NULL; //pointer to the head of the free list
struct FreeHeader * slabhead = NULL; //pointer to the head of the slab free list
pthread_mutex_t L; // lock
void * const_head = NULL;
void Clear_Mem(void * ptr, int size);

void * Mem_Init(int sizeOfRegion, int slabSize){
        int thread;

        thread = pthread_mutex_init(&L, NULL);
        if(thread != 0){

                return NULL;
        }

        if(init == 1){


                return NULL;
        }
        else{
                init = 1;// cannot call init twice
                void * ptr = mmap(NULL, sizeOfRegion, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
                if(ptr == MAP_FAILED){

                        return NULL;
                }
                while(slabSize % 16 != 0){
                        slabSize++;
                }

                slabhead = ptr;//put slabhead at the beginning of the slab space
                //put the nextfit head at the beginning of the nextfit space
                nextfithead = ptr + (sizeOfRegion / 4);
                // calculate the nextfit space length
                nextfithead->length = (((sizeOfRegion / 4) * 3) - 16);
                // set the next fithead to point to NULL.
                nextfithead->next = nextfithead;


                struct FreeHeader * pointer;
                int i;
                int NO_slabs = ((sizeOfRegion / 4 )/ slabSize);

                for(i = 0; i < NO_slabs; i++){
                        pointer = ptr + (slabSize * i);
                        pointer->next = ptr + (slabSize * (i + 1));

                }
                pointer->next = NULL; // set last freeheader to point to NULL

                mem_size = sizeOfRegion;//save variables for use in other functions
                slabsize = slabSize;
                const_head = ptr;

                return ptr; //return mmap ptr
        }//end else
}//end Mem_Init()





//Malloc function. 
void * Mem_Alloc(int size){
        //Check if size = slabSize
        //if no, it goes to next fit -> start at spot last allocated, look for next piece that fits, return that
        //if yes, slab allocate. If slab is full it goes into next fit.

        if(init == 0){
                ///memory is not yet been initalized
                return NULL;
        }
        pthread_mutex_lock(&L);// grab the lock


        // 0 do slab alloc
        // 1 do nextfit alloc
        int slab_or_nextfit = 1;
        // copy of nextfithead to facilitate looping
        void * free_pointer;
        // the pointer to be returned pointing to the allocated space
        // if NULL then the space is full or there isn't enough
        // contiguous space

        // padd size to 16 hold here for nextfit
        int padded_size = size;

        struct FreeHeader * temp; // header to aid in allocation
        temp = nextfithead;

        /* check that size is not negative and the size is not too large*/
        if((size <= 0) && (size < mem_size)){

                pthread_mutex_unlock(&L);
                return NULL;
        }

        while(padded_size % 16 != 0){

                //padd size to 16 for nextfit region
                padded_size++;

        }

        // determine alloc region
        if(size == slabsize){
                if (slabhead == NULL){
                        //slab is full do nextfit alloc
                        slab_or_nextfit = 1;

                }
                else{
                        //slab is available do slab alloc
                        slab_or_nextfit = 0;

                }
        }// end if determine alloc region


        if ( slab_or_nextfit == 0){
                // do alloc in slab region

                void * ptr = slabhead;
                slabhead = slabhead->next;
                Clear_Mem(ptr, slabsize); // clear the memory to zeros
                pthread_mutex_unlock(&L);
                return ptr;
        }
        else{
                if(nextfithead == NULL){// memory is already full
                        pthread_mutex_unlock(&L);
                        return NULL;
                }
                //do alloc in nextfit region
                free_pointer = nextfithead; // hold current location for head for looping
                do{

                        // check free region size
                        if (nextfithead->length >= padded_size){

                                if(nextfithead->length == padded_size && nextfithead->next == nextfithead){
                                        //free region is exact size

                                        struct AllocatedHeader * allocated;
                                        allocated = (void *) nextfithead;

                                        /* update the sectioned off portions size */
                                        allocated->length = (padded_size + sizeof(nextfithead));

                                        nextfithead = NULL; // memory is now full
                                        // update the magic feild
                                        allocated->magic = (void *) MAGIC;
                                        /*return the address to the beginning of the data portion of the block*/
                                        pthread_mutex_unlock(&L);
                                        return allocated + 1;
                                }
                                else{
                                        /*if the block is too big then we need to allocate only a portion of it*/
                                        /* put header in the correct place for the given size of memory */
                                        int head_size = temp->length;
                                        struct AllocatedHeader * allocated;
                                        allocated = (void *) nextfithead;

                                        /* update the sectioned off portions size */
                                        allocated->length = (padded_size + sizeof(nextfithead));
                                        // reset the next head to the correct size and properly link it 
                                        nextfithead = (temp + (padded_size/16) + 1);
                                        nextfithead->length = head_size - (padded_size + sizeof(nextfithead)) - sizeof(allocated);
                                        nextfithead->next = temp->next + ((padded_size/16) + 1);
                                        // update the magic feild
                                        allocated->magic = (void *) MAGIC;

                                        /*return the address to the beginning of the data portion of the block*/
                                        pthread_mutex_unlock(&L);
                                        return allocated + 1;

                                }
                        }// end if check free region size

                        nextfithead = nextfithead->next; // set the nextfithead to 
                        // the next free header

                }while(nextfithead != free_pointer);//check the whole free list 
                // for the space
// LFSSSSSSSSSSS written by allen rand, brianna smith
// written by Allen Rand, Brianna Smith
//MYFSCK

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


///////////////////////////////////////////////////
//      FILE SYSTEM HEADER CONTENTS             //
/////////////////////////////////////////////////


// On-disk file system format.
// Both the kernel and user programs use this header file.

// Block 0 is unused.
// Block 1 is super block.
// Inodes start at block 2.

#define ROOTINO 1  // root i-number
#define BSIZE 512  // block size

// File system super block
struct superblock {
        uint size;         // Size of file system image (blocks)
        uint nblocks;      // Number of data blocks
        uint ninodes;      // Number of inodes.
};

#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(uint))
#define MAXFILE (NDIRECT + NINDIRECT)

// On-disk inode structure
struct dinode {
        short type;           // File type
        short major;          // Major device number (T_DEV only)
        short minor;          // Minor device number (T_DEV only)
        short nlink;          // Number of links to inode in file system
        uint size;            // Size of file (bytes)
        uint addrs[NDIRECT+1];   // Data block addresses
};

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i
#define IBLOCK(i)     ((i) / IPB + 2)

// Bitmap bits per block
#define BPB           (BSIZE*8)

// Block containing bit for block b
#define BBLOCK(b, ninodes) (b/BPB + (ninodes)/IPB + 3)

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent {
        ushort inum;
        char name[DIRSIZ];
};

///////////////////////////////////////////////////
//      END FILE SYSTEM HEADER CONTENTS         //
/////////////////////////////////////////////////

///////////////////////////////////////////////////
//              HELPER FUNCTIONS                //
/////////////////////////////////////////////////

void display(int num_inodes, void * head, int bitmap_block, int flag);
const char *byte_to_binary(int x);
void clear_inode(struct dinode inode);
void clear_directory_entry(struct dirent * directory);
void clear_inode_ptr(struct dinode * inode);
struct dinode * find_inode(int inum, void * head, int num_inodes);
int find_parent(int inum, void * head, int num_inodes);
int directory_size(int inum, void * head, int num_inodes);
int * allocated_blocks(int num_inodes, void * head, int bitmap_block);

///////////////////////////////////////////////////
//              END HELPER FUNCTIONS            //
/////////////////////////////////////////////////

int main(int argc, char **argv){
        // MAIN IMPLEMENTS THE FSCK
        // steps
        // 1. open the file system image
        // 2. load the image in to memory
        // 3. Check the superblock and make size fix
        // 4. Check inode types and the link counts
        // 5. directory sanity check and fixes
        // 6. find lost+found directory
        // 7. file sanity check and fixes
        // 8. bitmap sanity check and fix

        //counters
        int MAX_BLOCKS = 995;
        int MAX_INODES = 200;
        int num_inodes;
        int bitmap_block;
        int i,x,w,z;//counters
        float blocks;
        void * ptr;
        void * head;
        struct dinode * inode;
        struct superblock * sb;
        struct stat st;


        //take image files in as input from the command line.
        if(argc < 2){
                printf("Error!\n");
                return -1;
        }

        // open the file
        int fd = open(argv[1], O_RDWR);
        // make sure the file opened correctly
        if (fd == -1){
                printf("file open error\n");
                return -1;
        }


        fstat(fd, &st);

        head = mmap(NULL, st.st_size , PROT_READ | PROT_WRITE,
        MAP_PRIVATE, fd, 0);

        if(head == MAP_FAILED){

                return -1;
        }
        //printf("head: %p\n", head);
        //align ptr with beginning of .img
        ptr = head;

        // find superblock
        for (;;){
                sb = (struct superblock *) ptr;
                if(sb->size != 0){
                        break;
                }
                ptr = ptr + sizeof(struct superblock *);
        }

        //make super block checks
        if(sb->size != st.st_size){
                sb->size = st.st_size/BSIZE;
        }
        //max number of blocks
        if(sb->nblocks >  MAX_BLOCKS){
                printf("Error!\n");
                exit(1);
        }
        //max inodes
        if(sb->ninodes > MAX_INODES){
                printf("Error!\n");
                exit(1);
        }
        //number of inodes
        num_inodes = sb->ninodes;
        //calculate number of blocks taken by inodes
        blocks = num_inodes;
        blocks = ceil(blocks / 8);

        // bit map block is located at the end of the inodes blocks
        // empty, superblock, inode(s), bitmap, data
        // calculation is number of inode blocks plus empty and superblock
        bitmap_block = 28;
        printf("%d\n",bitmap_block);



        ///////////////////////////////////////////////////
        //      FILE TYPE AND LINK COUNT CHECK          //
        /////////////////////////////////////////////////       
        // parse inodes for the number of inodes present said by superblock
        // inodes can have type 0,1,2,3
        // make link checks and clear bad inode types
        for (i = 0; i < num_inodes; i++){
                inode = find_inode(i, head, num_inodes);
                if(inode->type == 0 || inode->type == 1 || inode->type == 2 || inode->type == 3 ){

                        //if the inode is of type zero all feilds should be cleared.
                        if(inode->type == 0){
                                clear_inode_ptr(inode);
                        }
                }
                else{
                        clear_inode_ptr(inode);
                }
                if(inode->type == 2){
                        inode->nlink = 1;
                }

        }
        ///////////////////////////////////////////////////
        //      END FILE TYPE AND LINK COUNT CHECK      //
        /////////////////////////////////////////////////       

        // display info before directory and file fixes
        //display(num_inodes, head, bitmap_block, 0);


        ///////////////////////////////////////////////////
        //      DIRECTORY SANITY CHECK AND FIX          //
        /////////////////////////////////////////////////       
        struct dinode * directory;
        struct dirent * dot;
        struct dirent * dot2;
        for(x = 0; x < num_inodes ; x++){
                // iterate through Inodes.
                directory = find_inode(x, head, num_inodes);

                //is a directory
                if(directory->type == 1){
                        dot = head + (directory->addrs[0] * BSIZE);
                        dot2 = head + (directory->addrs[0] * BSIZE);
                        dot2 = dot2 + 1;
                        if(strcmp(dot->name, ".") != 0 && strcmp(dot2->name, "..") != 0){
                                clear_directory_entry(dot);
                                dot->inum = x;
                                dot->name[0] = '.';

                                clear_directory_entry(dot2);
                                dot2->name[0] = '.';
                                dot2->name[1] = '.';
                                dot2->inum = find_parent(x, head, num_inodes);
                                directory->size = directory_size(x, head, num_inodes);
                        }

                }
        }
        ///////////////////////////////////////////////////
        //      END DIRECTORY SANITY CHECK AND FIX      //
        /////////////////////////////////////////////////       

        ///////////////////////////////////////////////////
        //      FIND LOST+FOUND DIRECTORY               //
        /////////////////////////////////////////////////       

        //find the lost and found directory
        struct dirent * lost_found;// hold the lost and found directory info

        int found = 0;
        struct dinode * root;
        root = find_inode(ROOTINO, head, num_inodes);
        //go to root directory to search for the inode containing the lost and found directory
        for(w = 0; w < 13; w++){

                if(root->addrs[w] != 0 && found == 0){
                        lost_found = head + (BSIZE * root->addrs[w]);//index to the directories block

                        for(i = 0; i < BSIZE/sizeof(struct dirent); i++){
                                // check for the lost and found directory
                                if(strcmp(lost_found->name, "lost+found") == 0){
                                        found = 1;
                                        break;// found the lost and found directory
                                }
                                else
                                lost_found = lost_found + 1;
                        }
                }
        }


        ///////////////////////////////////////////////////
        //      END FIND LOST+FOUND DIRECTORY           //
        /////////////////////////////////////////////////

        ///////////////////////////////////////////////////
        //      FILE SANITY CHECK AND FIX               //
        /////////////////////////////////////////////////       
        struct dirent * file_ptr;
        struct dinode * lost;
        struct dinode * current;
        int parent;
        lost = find_inode(lost_found->inum, head, num_inodes);
        for(x = 0; x < num_inodes ; x++){
                current = find_inode(x, head, num_inodes);
                // 0 file has no parent
                // 1 file has a parent
                // > 1 file has multiple parents
                parent = 0;
                if(current->type == 2){
                        parent = find_parent(x, head, num_inodes);
                                //file has no parent set it to lost and found
                        if(parent == -1){
                                // go to lost and found directory and add the lost file
                                file_ptr = head + (BSIZE * lost->addrs[0]);
                                for(i = 0; i < BSIZE/sizeof(struct dirent); i++){
                                        // spot is open to add the file reference
                                        if(file_ptr->inum == 0){
                                                file_ptr->inum = x;
                                                file_ptr->name[0] = 'f';
                                                file_ptr->name[1] = 'o';
                                                file_ptr->name[2] = 'o';
                                                file_ptr->name[3] = 'b';
                                                file_ptr->name[4] = 'a';
                                                file_ptr->name[5] = 'r';

                                                lost->size = lost->size + sizeof(struct dinode);
                                                break;
                                        }
                                        else{

                                                file_ptr = file_ptr + 1;
                                        }
                                }
                        }// end if      
                }// end if
        }// end for             

        //printf("display 2\n");
        //display(num_inodes, head, bitmap_block, 2);
        ///////////////////////////////////////////////////
        //      END FILE SANITY CHECK AND FIX           //
        /////////////////////////////////////////////////

        ///////////////////////////////////////////////////
        //      BITMAP SANITY CHECK AND FIX             //
        /////////////////////////////////////////////////

        //get all allocated block numbers and put the list in allocated
        int a;
        int * allocated = malloc(1024*sizeof(int));
        for(a = 0; a < 29; a++){
                allocated[a] = a;
        }
                int * indirect;
                for(x = 0; x < num_inodes; x++){
                        inode = find_inode(x, head, num_inodes);
                        if(inode->type != 0){
                                        for(i = 0; i < 13 ; i++){
                                                if(i == 12){
                                                        if(inode->addrs[i] != 0){
                                                                allocated[a] = inode->addrs[i];
                                                                a++;
                                                        }
                                                        //follow it and print our the indirect pointer contents
                                                        if(inode->addrs[i] != 0){
                                                                indirect = head + ( inode->addrs[i] * BSIZE);
                                                                for(w = 0; w < BSIZE/sizeof(struct dirent); w++){
                                                                        if(*indirect != 0){
                                                                                if(*indirect != 0){
                                                                                        allocated[a] = *indirect;
                                                                                        a++;
                                                                                }
                                                                        }//end if
                                                                        indirect = indirect + 1;
                                                                }//end for
                                                        }//end if
                                                } //end if

                                                else{
                                                        if(inode->addrs[i] != 0){
                                                                allocated[a] = inode->addrs[i];
                                                                a++;
                                                        }
                                                }
                                        }//end for

                        }//end if
                }// end for


        char * bitmap = malloc (BSIZE/sizeof(char));
        char byte;
        char map;
        for(x = 0; x < 1024; x++){
                byte = bitmap[allocated[x]/8];
                map = 1 << allocated[x]%8;
                bitmap[allocated[x]/8] = map | byte;

        }

        ptr = head + (BSIZE*28);
        ptr = bitmap;


        //display(num_inodes, head, bitmap_block, 2);



        ///////////////////////////////////////////////////
        //      BITMAP SANITY CHECK AND FIX             //
        /////////////////////////////////////////////////
        int b;
        b = write(fd, head, st.st_size);
        if(b == -1){
                printf("file could not write");
        }
        if (munmap(head, st.st_size) == -1) {
                printf("Error!\n");
                /* Decide here whether to close(fd) and exit() or not. Depends... */
        }
        close(fd);


        return 0;
}

//displays the inodes
void display(int num_inodes, void * head, int bitmap_block, int flag){
        //flag 0 - print all
        //flag 1 - print inodes
        //flag 2 - print bitmap
        //flag 3 - print directory contents
        int x,i,w;
        void * ptr;
        struct dinode * inode;
        if(flag == 1 || flag == 0){

                int * indirect;
                printf("\n\n\tInodes: \n\n");
                for(x = 0; x < num_inodes; x++){
                        inode = find_inode(x, head, num_inodes);
                        if(inode->type != 0){
                                        printf("\t--inode %d --\n\n", x);
                                        printf("inode.type: %d\ninode.major: %d\n", inode->type, inode->major);
                                        printf("inode.minor: %d\ninode.nlink: %d\n", inode->minor, inode->nlink);
                                        printf("inode.size: %d\n", inode->size);
                                        for(i = 0; i < 13 ; i++){
                                                if(i == 12){
                                                        printf("inode.addr[indirect %d]: %d\n",i, inode->addrs[i]);
                                                        //follow it and print our the indirect pointer contents
                                                        if(inode->addrs[i] != 0){
                                                                printf("\nindirect contents: \n\n");
                                                                indirect = head + ( inode->addrs[i] * BSIZE);
                                                                for(w = 0; w < BSIZE/sizeof(struct dirent); w++){
                                                                        if(*indirect != 0){
                                                                                printf("block %d indirect: %d\n", inode->addrs[i], *indirect);
                                                                        }//end if
                                                                        indirect = indirect + 1;
                                                                }//end for
                                                        }//end if
                                                } //end if
                                                else printf("inode.addr[%d]: %d\n",i, inode->addrs[i]);
                                        }//end for
                                        printf("\n");
                        }//end if
                }// end for
                printf("\n\n\tEnd Inodes: \n\n");
        }

        if(flag == 2 || flag == 0){
                //print bit map
                ptr = head + (BSIZE * 28);
                printf("\thead: %p\n", head);
                printf("\tbitmap location: %p\n\n", ptr);
                int * bptr = ptr;
                char b[128];
                for (i = 1; i < BSIZE/sizeof(int);i++){

                        printf("%s ", byte_to_binary(*bptr));

                        if(i % 10 == 0)
                        printf("\n");
                        bptr = bptr + 1;
                }
                printf("\n\n end bitmap\n\n");
        }//end if
        if(flag == 3 || flag == 0){
                printf("\n\n\tdirectory contents: \n\n");
                // print directory contents
                struct dirent * directory;
                for(x = 0; x < num_inodes ; x++){
                        inode = find_inode(x, head, num_inodes);
                        if(inode->type == 1){
                                for(w = 0; w < 13; w++){
                                        if(inode->addrs[w] != 0){
                                                directory = head + (BSIZE * inode->addrs[w]);//index to the directories block
                                                printf("\t--followed directory ptr from inode: %d to block: %d--\n\n", x, inode->addrs[w]);
                                                for(i = 0; i < BSIZE/sizeof(struct dirent); i++){

                                                        if(directory->inum > 0 && directory->inum < 1024){
                                                                //check to see if the inode is an allocated file or directory or type 3
                                                                printf("\tdirectory inum: %d\n", directory->inum);
                                                                printf("\tdirectory name: %s\n", directory->name);
                                                                printf("\n");
                                                        }
                                                        directory = directory + 1;
                                                }//end for
                                        }//end if
                                }//end for
                        }// end if      
                } // end for
                printf("\n\n\tend directory contents: \n\n");
        }// end if






}// END MAIN

// displays the bytes in binary
const char *byte_to_binary(int x)
{

    static char b[9];
    b[0] = '\0';

    int z;
    for (z = 128; z > 0; z >>= 1)
    {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }

    return b;
}
// clears a inode
void clear_inode_ptr(struct dinode * inode){

        inode->type == 0;
        inode->major = 0;
        inode->minor = 0;
        inode->nlink = 0;
        inode->size = 0;
        int i;
        for(i = 0; i < 13 ; i++){
                inode->addrs[i] = 0;
        }
}
// clears a inode
void clear_inode(struct dinode inode){

        inode.type == 0;
        inode.major = 0;
        inode.minor = 0;
        inode.nlink = 0;
        inode.size = 0;
        int i;
        for(i = 0; i < 13 ; i++){
                inode.addrs[i] = 0;
        }
}
// clears a directory entry
void clear_directory_entry(struct dirent * directory){
        char empty = '\0';
        directory->inum = 0;
        int i;
        for(i = 0; i < 13 ; i++){
                directory->name[i] = empty;
        }


}

//cleans the file system

// get the location of a inode by inum
struct dinode * find_inode(int inum, void * head, int num_inodes){


        struct dinode * i_ptr = head + (BSIZE * 2);
        int i;
        for (i = 0; i < num_inodes; i++){
                if(i == inum){
                        return i_ptr;
                }
                i_ptr = i_ptr + 1;
        }
        return NULL;
}

int find_parent(int inum, void * head, int num_inodes){

        // finds the parent of the inode number you give it
        // returns -1 if no parent can be found.

        int i,w,z;
        struct dinode * parent;
        struct dirent * entry;
        //find the parent directory
        // iterate through inodes
        for(i = 0; i < num_inodes ; i++){
                //check the inodes
                parent = find_inode(i, head, num_inodes);
                // its a directory check if it points to the one we want
                if(parent->type == 1){
                        //check whole directory
                        for(z = 0; z < 13; z++){
                                entry = head + (BSIZE * parent->addrs[z]);
                                //iterate through the directory contents
                                for(w = 0; w < BSIZE/sizeof(struct dirent); w++){
                                        //printf("entry inum %d\n", entry->inum);
                                        //printf("i %d\n", i);
                                        //printf("ium %d\n", inum);
                                        if(entry->inum == inum && i != inum){
                                                return i;
                                        }
                                        entry = entry + 1;

                                }

                        }
                }
        }
        return -1;
}
int directory_size(int inum, void * head, int num_inodes){

        // finds the size of the 
        // returns -1 if the inode 
        //is not allocated or is not a directory.

        int size,w,z;
        struct dinode * dir;
        struct dirent * entry;
        size = 0;
        //find the directory
        dir = find_inode(inum, head, num_inodes);
        // its a directory check if it points to the one we want
        if(dir->type == 1){
                //check whole directory
                for(z = 0; z < 13; z++){
                        entry = head + (BSIZE * dir->addrs[z]);
                        //iterate through the directory contents
                        for(w = 0; w < BSIZE/sizeof(struct dirent); w++){
                                if(entry->inum != 0 && entry->inum > 0 && entry->inum < 200){
                                        size += sizeof(struct dirent);
                                }
                                entry = entry + 1;
                        }
                }
        }
        else{
                return -1;
        }
        return size;
}
