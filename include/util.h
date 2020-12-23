#ifndef _UTIL_H
#define _UTIL_H

typedef signed char int8_t;
typedef unsigned char   uint8_t;
typedef short  int16_t;
typedef unsigned short  uint16_t;
typedef int  int32_t;
typedef unsigned   uint32_t;

#define MAGICNUM 0x20201222

#define MAXARGS 10
#define MAXWORD 30
#define MAXLINE 100

#define TYPE_FILE 0
#define TYPE_DIR 1

#define BLOCK_SIZE 1024   // 每块 BLOCK 的大小 1KB
#define INODE_SIZE 32     // 每块 INODE 大小 32Bytes
#define DIR_ITEM_SIZE 128 // 每块 DIR_ITEM 大小 128Bytes

#define NDISKBLOCK_PER_DATABLOCK 2
#define N_INODE_BLOCK 32
#define N_DATA_BLOCK 4063

#define MAX_FILE_BLOCK_NUM 6
#define MAX_INODE_NUM 1024

/**
 * @brief 将两个字符串拼接，形成新的字符串
 */
char* join(char *s1, char *s2);




#endif