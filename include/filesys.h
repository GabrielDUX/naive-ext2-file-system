#ifndef _FILESYS_H
#define _FILESYS_H

#include "util.h"

typedef struct super_block {
    int32_t magic_num;          // 幻数
    int32_t free_block_count;   // 空闲数据块数       
    int32_t free_inode_count;   // 空闲inode数
    int32_t dir_inode_count;    // 目录inode数
    uint32_t block_map[128];    // 数据块占用位图
    uint32_t inode_map[32];     // inode占用位图
} sp_block_t;

typedef struct inode {
    uint32_t size;              // 文件大小
    uint16_t file_type;         // 文件类型（文件/文件夹）
    uint16_t link;              // 连接数
    uint32_t block_point[6];    // 数据块指针
} inode_t;

typedef struct dir_item {               // 目录项一个更常见的叫法是 dirent(directory entry)
    uint32_t inode_id;          // 当前目录项表示的文件/目录的对应inode
    uint16_t valid;             // 当前目录项是否有效 
    uint8_t type;               // 当前目录项类型（文件/目录）
    char name[121];             // 目录项表示的文件/目录的文件名/目录名
} dir_item_t;


typedef struct p_stack {                            // 路径栈
    struct dir_item dir_stack[MAXARGS];     // 元素为dir_item
    int top;                                // 栈顶指针
} p_stack_t;

/**
 * @brief 初始化文件系统
 */
int init_filesystem();

int shutdown_filesys();

/**
 * @brief 执行 ls 展示读取文件夹内容
 */
int exec_ls(char *argv[],int argc);


/**
 * @brief 执行 mkdir 创建文件夹
 */
int exec_mkdir(char *argv[],int argc);

/**
 * @brief 执行 touch 创建文件
 */
int exec_touch(char *argv[],int argc);

/**
 * @brief 执行 cp 复制文件
 */
int exec_cp(char *argv[],int argc);




#endif