#ifndef _FS_H
#define _FS_H

#include <type.h>
#define MAX_FILE_NUM 16      //open file num
#define FS_SECTOR 1048576lu  //file system start sector
#define MAGIC_NUM 0x20211224 //magic
#define BLOCK_SZ 4096lu      //block size
#define DATA_SZ 0x40000000   //1G data

//permission control
#define PMS_RDONLY 0
#define PMS_WTONLY 1
#define PMS_RDWR 2

//inode type
#define INODE_DIR 0
#define INODE_FILE 1

#define O_RDONLY 1 /* read only open */
#define O_WRONLY 2 /* write only open */
#define O_RDWR 3 /* read/write open */

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef struct _superblock{
    uint32_t magic;
    uint32_t size;
    uint32_t start;

    uint32_t blockmap_offset;
    uint32_t blockmap_sz;

    uint32_t inodemap_offset;
    uint32_t inodemap_sz;
    
    uint32_t inodetable_offset;
    uint32_t inodetable_sz;
    
    uint32_t datablock_offset;
    uint32_t datablock_sz;
} superblock_t;

typedef struct _inode{
    uint8_t type;
    uint8_t mode;
    uint16_t ino;
    uint32_t direct[10];
    uint32_t indirect_1;
    uint32_t indirect_2;
    uint32_t links;
    uint32_t size;
    uint32_t time;
}inode_t;

typedef struct _dentry{
    uint16_t ino;
    char name[62];
}dentry_t;

typedef struct _file_des{
    uint8_t used;
    uint8_t mode;
    uint16_t ino;
    uint32_t r_cursor;
    uint32_t w_cursor;
}fd_t;

int fs_exist();
void do_mkfs();
void do_statfs();
void do_cd(char *dir);
void do_mkdir(char *dir);
void do_rmdir(char *dir);
void do_ls(char *mode, char *dir);
void do_touch(char *file);
void do_cat(char *file);
void do_ln(char *dst, char *src);
void do_rm(char *file);

int do_fopen(char *name, int access);
int do_fread(int fd, char *buff, int size);
int do_fwrite(int fd, char *buff, int size);
int do_fclose(int fd, char *buff, int size);
int do_lseek(int fd, int offset, int whence);
#endif
