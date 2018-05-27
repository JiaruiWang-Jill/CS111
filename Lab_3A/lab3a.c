#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "ext2_fs.h" 

// Constant 
#define BASE_OFFSET 1024   // Superblock's begining 

#define EXIT_BADARG 1 
#define EXIT_CORR 2 

// Global Variables 
int img_fd = -1; // Image's file descriptor 
unsigned int block_size = 0;

struct ext2_super_block superblock; 
struct ext2_group_desc groupdesc;

FILE* output_file;

void free(){
//TODO: 
// Free and Exit
  close(img_fd);
}

void error_printer(const char *info, int exit_code){
    if (errno) {
		  fprintf(stderr, "ERROR; %s: %s\n", info, strerror(errno));
    } else {
		  fprintf(stderr, "ERROR; %s\n", info);
    }
    free();
    exit(exit_code);
}


void superblock_handler(){ // SUPERBLOCK 
  void * temp_sb_p = (void*)&superblock; 
  if(pread(img_fd, temp_sb_p, sizeof(struct ext2_super_block), BASE_OFFSET) < 0){
      error_printer("SUPERBLOCK. pread", EXIT_CORR);
  }
  block_size = BASE_OFFSET << superblock.s_log_block_size; 
  fprintf(output_file, "%s,%d,%d,%d,%d,%d,%d,%d\n", "SUPERBLOCK", 
    superblock.s_blocks_count, 
    superblock.s_inodes_count,  
		block_size, 
    superblock.s_inode_size, 
    superblock.s_blocks_per_group, 
    superblock.s_inodes_per_group,
		superblock.s_first_ino );
}

void group_handler(){ // GROUP TODO:
  void* tempgdp=(void*)&groupdesc;

  
}

void free_block_handler(){ // FREE BLOCK TODO:

}

void free_inode_handler(){ // FREE I-NODE TODO:

}

void inode_handler(){ // I-NODE TODO:

}

void directory_handler(){ // DIRECTORY TODO:

}

void indirect_block_handler(){ // INDIRECT BLOCK TODO:

}

int main(int argc, char** argv){
    const char *temp_img = argv[1];
    img_fd = open(temp_img, O_RDONLY);
    if(img_fd < 0){
      error_printer("fail to open file", EXIT_CORR);
    }
    free();
    exit(0);
}