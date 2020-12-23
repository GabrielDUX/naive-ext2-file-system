#include "disk.h"
#include "util.h"
#include "filesys.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char sp_block_buf[2*DEVICE_BLOCK_SIZE];
char disk_block_buf[DEVICE_BLOCK_SIZE];
inode_t inode_buf;
dir_item_t dir_item_buf;
dir_item_t block_buf[8];



/**
 * @brief 根据 inode_id 获取 disk block 的 id
 * @return 成功返回disk block 的 id，失败返回 -1
 */
int get_disk_id_inode(uint32_t inode_id){
    if(inode_id>1024){
        return -1;
    } else {
        return (2 + inode_id / 16);
    }
};

/**
 * @brief 根据 inode_id 获取 disk block 的 id
 * @return 成功返回disk block 的 id，失败返回 -1
 */
int get_disk_id_data(uint32_t block_point){
    if(block_point > 4063){
        return -1;
    } else {
        return (66 + block_point * 2);
    }
};

/**
 * @brief 根据inode id 读取inode
 * @return inode 指针,读取失败则返回NULL
 */
inode_t* read_inode(uint32_t inode_id){
    char *buf = disk_block_buf;
    uint32_t disk_block_id = 2 + inode_id / 16;
    if(disk_read_block(disk_block_id,buf)<0){
        return NULL;
    }
    uint32_t offset = (inode_id % 16);
    memcpy((char*)&inode_buf,(char*)&disk_block_buf[offset*sizeof(inode_t)],sizeof(inode_buf));
    return (inode_t*)&disk_block_buf[offset*sizeof(inode_t)];
    // return &inode_buf;
};

/**
 * @brief 读super_block
 * @return 成功返回super block buf指针，失败返回NULL
 */
sp_block_t* read_spblock(){
    char *buf = (char *)sp_block_buf;
    for(int i=0;i<2;i++){
        if(disk_read_block(i,buf)<0){
            return NULL;
        }
        buf += DEVICE_BLOCK_SIZE;
    }
    return (sp_block_t*)sp_block_buf;
}

/**
 * @brief 读dir_item,将block_id号对应的block读出到block_buf中
 * @return 成功返回block_buf指针（block中第一个dir_item的指针）
 *         失败返回NULL
 */
dir_item_t* read_dir_item(uint32_t block_id,uint16_t offset){
    char *buf = (char*)block_buf;
    for(int i=0;i<2;i++){
        if(disk_read_block(block_id*2+i,buf)<0){
            return NULL;
        }
        buf += DEVICE_BLOCK_SIZE;
    }
    return &block_buf[offset];
}


int write_spblock();
int write_inode();

/**
 * @brief 写inode，即将disk block buf内容写进disk
 *        同时更新 super block 的信息
 * @return 
 */
int write_inode(uint32_t inode_id){
    //TODO 
    // update disk


    if(disk_write_block(get_disk_id_inode(inode_id),disk_block_buf)<0){
        return -1;
    }

    sp_block_t* sp_block = read_spblock();
    sp_block->free_block_count -= 1;
    uint16_t index = inode_id / 32;
    uint16_t offset = inode_id % 32;
    uint32_t new_inode_map = sp_block->inode_map[index];
    new_inode_map |= (0x80000000 >> offset);
    sp_block->inode_map[index] = new_inode_map;        


    //update spblock
    if(write_spblock()<0){
        return -1 ;
    } 
    return 0;
};

/**
 * @brief 写dir_item:将dir_item_buf的信息拷贝到block_buf
 *        然后将block_buf写入磁盘。执行此函数之前需要read_dir_item
 *        同时更新 super block 的信息
 * @return 
 */
int write_dir_item(uint32_t block_point,uint16_t offset){
    //TODO
    // 将 dir_item_buf拷贝进block_buf
    char *buf = (char*) block_buf;
    memcpy(&block_buf[offset],&dir_item_buf,sizeof(dir_item_buf));
    // printf("write_dir:\n,inode_id:%d,name:%s\n",((dir_item_t*)&block_buf[offset])->inode_id,((dir_item_t*)&block_buf[offset])->name);
    for(int i=0;i<2;i++){
        if(disk_write_block(block_point*2+i,buf)<0){
            return -1;
        }
        buf+=DEVICE_BLOCK_SIZE;
    }
    sp_block_t *sp_block = read_spblock();
    sp_block->free_block_count -= 1;
    uint16_t index = block_point / 32;
    uint16_t offset_map = block_point % 32;
    uint32_t new_block_map = sp_block->block_map[index];
    new_block_map |= (0x80000000 >> offset_map);
    sp_block->block_map[index] = new_block_map;

    if(write_spblock()<0){
        return -1;
    }

    return 0;
}

/**
 * @brief 写super_block，将sp_block_buf内容写进disk
 * @return 成功返回0,失败返回-1
 */
int write_spblock(){
    char *buf = (char*)sp_block_buf;
    for(int i=0;i<2;i++){
        if(disk_write_block(i,buf)<0){
            return -1;
        }
        buf += DEVICE_BLOCK_SIZE;
    }
    return 0;
};


/**
 * @brief 检查是否有重复名字
 * @return 如果有重复名字，返回重复的 （-inode_id）
 *         没有则返回最后一个遍历到的dir_item
 */
int check_dup_name(char *tmp,int inode_id,int type){
    // int i,j;
    // inode
}


/**
 * @brief 初始化文件系统
 *        如果没有disk，则创建，创建失败返回“open disk error！”
 *        如果有disk，则检查magic num，正确则说明已经初始化，否则进行初始化
 */
int init_filesystem(){

    if(open_disk()<0){
        printf("open disk error!\n");
        exit(0);
    }
    
    sp_block_t* sp_block = read_spblock();
    // printf("start:%.8x\n magic_num:%.8x\n",((sp_block_t*)sp_block_buf)->block_map[0],((sp_block_t*)sp_block_buf)->magic_num);

    if(sp_block->magic_num == MAGICNUM){ //disk 已经建立
        return 0;
    } else {  // disk not initialized
        sp_block->magic_num = MAGICNUM;
        sp_block->free_block_count = 4096 - 34; //1 for super block, 32 for inode, 1 for floder "root"
        sp_block->free_inode_count = 1024 - 1; 
        sp_block->dir_inode_count = 1;  // folder "root"
        sp_block->inode_map[0] = (0x80000000); // first inode "root"
        sp_block->block_map[0] = (0xffffffff); // 1 super block,31 inode;
        sp_block->block_map[1] = (0xc0000000); // 1 inode, 1 for folder "root"
        // printf("init:%.8x\n",((sp_block_t*)sp_block_buf)->block_map[0]);
        write_spblock();
        // read_spblock();
        // printf("init:%.8x\n",((sp_block_t*)sp_block_buf)->block_map[0]);
        
        inode_t* inode = read_inode(0);   // root inode
        inode->file_type = TYPE_DIR;
        inode->link = 0;
        inode->block_point[0]=33;   // 0 for super block, 1~32 for inode, 33 for root dir_item
        inode->size = 1;
        write_inode(0);

        dir_item_buf.inode_id = 0;  // init root dir_item
        strcpy(dir_item_buf.name,".");
        dir_item_buf.type = TYPE_DIR;
        dir_item_buf.valid = 1;
        // memset(disk_block_buf,0,DEVICE_BLOCK_SIZE);
        // dir_item_t* dir_item = (dir_item_t*)disk_block_buf;
        // memcpy(dir_item,&dir_item_buf,sizeof(dir_item_buf));
        // disk_write_block(66,(char*)disk_block_buf);
        read_dir_item(33,0);
        write_dir_item(33,0);

        dir_item_t *dir_item = read_dir_item(33,0);
        // disk_read_block(66,disk_block_buf);
        // dir_item_t *dir_item = (dir_item_t*)disk_block_buf;
        // printf("init:\n");
        // printf("dir_item:\ninode_id:%d,name:%s\n",dir_item->inode_id,dir_item->name);
        
    }
    return 0;
}

/**
 * @brief 从左到右，找到一个空闲的inode
 * @return success: inode_id, fail: -1
 */
int get_free_inode(){
    sp_block_t *sp_block = read_spblock();
    if(sp_block->free_inode_count == 0){
        printf("No free inode!\n");
        return -1;
    }
    for(int i=0;i<32;i++){
        if(sp_block->inode_map[i]==0xffffffff)
            continue;
        uint32_t mask = 0x80000000;
        //掩码从左到右寻找inode_map为0的位置
        for(int j=0;j<32;j++){
            if(mask & sp_block->inode_map[i])  //没有找到空闲位置，继续向右寻找
                mask >>= 1;
            else {  // 找到空闲,返回空闲inode,并且更新超级块
                // sp_block->inode_map[i] |= mask;
                return (i*32+j);
            }
        }
    }
    return -1;
}

/**
 * @brief 从左到右，找到满足num要求的空闲block
 * @return success: block_id, fail: -1
 */
int get_free_block(int block_num){
    sp_block_t *sp_block = read_spblock();
    if(sp_block->free_block_count<block_num){ //如果剩余block不满足要申请的数目，直接返回-1
        printf("No enough blocks \n");
        return -1;
    }
    
    for(int i=0;i<128;i++){
        if(sp_block->block_map[i]==(0xffffffff))
            continue;
        uint32_t mask  = 0x80000000;
        // printf("get_free_block(),sp_block.block_map[%d]:0x%.8x\n",i,sp_block->block_map[i]);
        for(int j=0;j<32;j++){
            if((mask & sp_block->block_map[i])){ //没有找到空闲位置，继续向右寻找
                mask >>= 1;
            } else {
                // sp_block->block_map[i] |= mask;
                // sp_block->free_block_count --;
                if(block_num == 0){
                    return -1;
                }
                else {
                    return (i*32+j);
                }
            }
        }
    }
}

/**
 * @brief mkdir，找到路径，并且tmp存储要创建的文件名
 * @return success: 最后一个目录的inode_id, fail: -1
 */
int find_path_directory(char *path,char *tmp){
    int inode_id = 0;
    int i = 0;
    int j = 0;
    inode_t *inode;
    if(path[0]=='/'){
        inode_id = 0;
    }
    while(1){

        if(path[i]!='/'){
            tmp[j++] = path[i];
        }

        if(path[i]=='\0'){
            break;
        }
        // mkdir /home/tmp 
        if(path[i]=='/' && path[i+1]!='\0'){
            tmp[j] = '\0';
            j = 0;
            int success = 0;
            inode = read_inode(inode_id);
            for(int k=0;k<inode->size;k++){
                read_dir_item(inode->block_point[k],0);
                    for(int p=0;p<8;p++){
                        if(block_buf[p].type==TYPE_DIR \
                            && !strcmp(tmp,block_buf[p].name) \
                            && block_buf[p].valid)
                        {
                            k = 1024;
                            inode_id = block_buf[p].inode_id;
                            success = 1;
                            break;
                        }
                        if(!block_buf[p].valid){
                            success = 0;
                            continue;
                        }
                    }
            }
            if(!success){
                printf("No such directory \"%s\"\n",tmp);
                return -1;
            }
        }
        if(path[i]=='/' && path[i+1]=='\0'){
            tmp[j] = '\0';
            break;
        }
        i++;
    }
    return inode_id;
}

/**
 * @brief mkdir和touch，找到路径，并且tmp存储要创建的文件名
 * @return success: 0, fail: -1
 */
int find_path_file(char *path,char *tmp){

}

int exec_ls(char *argv[],int argc){
    // printf(".\n..\n");
    int i=0;
    int j=0;
    char tmp[MAXLINE];
    inode_t *inode;
    int inode_id = 0;
    if(argc == 1 || (argc>1 && !strcmp(argv[1],"/"))){ //列举根目录下的文件、文件夹
        // TODO
        inode_id = 0;
    } 
    else {
        char *path = argv[1];
        // printf("ls:path: %s\n",path);
        // 根据传进来的路径，找到对应目录
        while(path[i]!='\0'){

            if(path[i]!='/'){  //获取目录名
                tmp[j++] = path[i];
            }

            if(path[i]=='/' || path[i+1]=='\0'){
                tmp[j] = '\0';
                // printf("ls:tmp:%s\n",tmp);
                j=0;
                int success = 0;
                inode_t* inode = read_inode(inode_id);
                // 
                // printf("read_inode:\nsize:%d,type:%d,",inode->size,inode->file_type);
                for(int k=0;k<inode->size;k++){
                    // 读取inode的数据块
                    read_dir_item(inode->block_point[k],0);
                    for(int p=0;p<8;p++){
                        // printf("read block point %d of inode %d:\n",k,inode_id);
                        // printf("%d: type:%d,name: %s,valid:%d\n",p,block_buf[p].type,block_buf[p].name,block_buf[p].valid);
                        if(block_buf[p].type==TYPE_DIR \
                            && !strcmp(tmp,block_buf[p].name) \
                            && block_buf[p].valid)
                        {
                            k = 1024;
                            inode_id = block_buf[p].inode_id;
                            success = 1;
                            break;
                        }
                        if(!block_buf[p].valid){
                            success = 0;
                            continue;
                        }
                    }
                }
                if(!success){
                    printf("Can not find directory %s \n",path);
                    return -1;
                }
            }
            i++;
        }
    }
    // printf("ls(),inode_id:%d\n",inode_id);
    inode = read_inode(inode_id);
    // print size
    // printf("inode_%d_size:%d\n",inode_id,inode->size);
    for(int k=0;k<inode->size;k++){
        read_dir_item(inode->block_point[k],0);
        for(int j=0;j<8;j++){
            if(block_buf[j].valid){
                // printf("%s\n",block_buf[j].name);
                if(*block_buf[j].name=='\0'){
                    continue;
                }
                if((block_buf[j].type == TYPE_DIR) ) /// && (k!=0 && j!=0 && j!=1)) \\不为 .和..打印文件夹标志
                {
                    printf("[%s]\n",block_buf[j].name);
                }
                else{
                    printf("%s\n",block_buf[j].name);
                }
            }
        }
    }
}

int exec_mkdir(char *argv[],int argc){
    if(argc<2){
        printf("Too few arguments!\n");
        return -1;
    }
    char tmp[MAXLINE];
    char *path = argv[1];
    uint32_t inode_id = find_path_directory(path,tmp);
    // printf("find_path:path:%s,tmp:%s,inode_id:%d\n",path,tmp,inode_id);
    inode_t* inode;

    if(inode_id <0 || tmp[0]=='\0'){
        return -1;
    }

    // int i = check_dup_name(tmp,inode_id,TYPE_DIR);
    inode = read_inode(inode_id);
    // for(int j=0;j<6;j++){
    //     // 只有根目录的block_point[0] = 0，其余block_point都不能为0 
    //     // 否则视为该block_point是无效的,还没有指定
    //     if(inode_id==0){
    //         for(int k=0;k<inode->size;k++){
    //             read_dir_item(inode->block_point[k]);
    //             get_free_block(1)
    //         }
    //     }
    // }
    // printf("cur_dir:inode_id:%d,\n")

    int block_id = get_free_block(1);
    // inode_id = get_free_inode();

    int success = 0;
    for(int k=1;k<6;k++){
        // TODO：这里只实现了每个文件夹下最多五个dir_item
        if(inode->block_point[k]==0){ //未分配
            // 直接分配
            inode->block_point[k] = block_id;
            inode->size++;
            success = 1;
            write_inode(inode_id);
            break;
        }
        // else{
        //     // 寻找一块空闲的dir_item空间，将其放入
        // }
    }
    if(!success){
        printf("directory full!\n");
        return -1;
    }

    inode_id = get_free_inode();


    dir_item_t* dir_item = read_dir_item(block_id,0);
    dir_item_buf.inode_id = inode_id;
    strcpy(dir_item_buf.name,tmp);
    // printf("mkdir(),name:%s \nblock_point:%d\ninode_id:%d\n",dir_item_buf.name,block_id,inode_id);
    dir_item_buf.type = TYPE_DIR;
    dir_item_buf.valid = 1;
    write_dir_item(block_id,1);
    // read_spblock();
    // printf("mkdir:block_map[1]:%.8x\n",((sp_block_t*)sp_block_buf)->block_map[1]);

    inode = read_inode(inode_id);
    inode->block_point[0] = 0;
    inode->file_type = TYPE_DIR;
    inode->link = 0;
    inode->size += 1;
    write_inode(inode_id);

}

int exec_touch(char *argv[],int argc){
    if(argc<2){
        printf("Too few arguments!\n");
        return -1;
    }
    char tmp[MAXLINE];
    char *path = argv[1];
    uint32_t inode_id = find_path_directory(path,tmp);
    // printf("find_path:path:%s,tmp:%s,inode_id:%d\n",path,tmp,inode_id);
    inode_t* inode;

    if(inode_id <0 || tmp[0]=='\0'){
        return -1;
    }

    // int i = check_dup_name(tmp,inode_id,TYPE_DIR);
    inode = read_inode(inode_id);
    // for(int j=0;j<6;j++){
    //     // 只有根目录的block_point[0] = 0，其余block_point都不能为0 
    //     // 否则视为该block_point是无效的,还没有指定
    //     if(inode_id==0){
    //         for(int k=0;k<inode->size;k++){
    //             read_dir_item(inode->block_point[k]);
    //             get_free_block(1)
    //         }
    //     }
    // }
    // printf("cur_dir:inode_id:%d,\n")

    int block_id = get_free_block(1);
    // inode_id = get_free_inode();

    int success = 0;
    for(int k=1;k<6;k++){
        // TODO：这里只实现了每个文件夹下最多五个dir_item
        if(inode->block_point[k]==0){ //未分配
            // 直接分配
            inode->block_point[k] = block_id;
            inode->size++;
            success = 1;
            write_inode(inode_id);
            break;
        }
        // else{
        //     // 寻找一块空闲的dir_item空间，将其放入
        // }
    }
    if(!success){
        printf("directory full!\n");
        return -1;
    }

    inode_id = get_free_inode();


    dir_item_t* dir_item = read_dir_item(block_id,0);
    dir_item_buf.inode_id = inode_id;
    strcpy(dir_item_buf.name,tmp);
    // printf("mkdir(),name:%s \nblock_point:%d\ninode_id:%d\n",dir_item_buf.name,block_id,inode_id);
    dir_item_buf.type = TYPE_FILE;
    dir_item_buf.valid = 1;
    write_dir_item(block_id,1);
    // read_spblock();
    // printf("mkdir:block_map[1]:%.8x\n",((sp_block_t*)sp_block_buf)->block_map[1]);

    inode = read_inode(inode_id);
    inode->block_point[0] = 0;
    inode->file_type = TYPE_FILE;
    inode->link = 0;
    inode->size += 1;
    write_inode(inode_id);
}

int exec_cp(char *argv[],int argc){
    if(argc!=3){
        printf("arguments wrong!\n");
        return -1;
    }

    char *src_path = argv[2];
    char *dst_path = argv[1];
    char tmp1[MAXLINE];
    char tmp2[MAXLINE];
    // 首先找到src_path
    int inode_id = find_path_directory(src_path,tmp1);

    inode_t *inode = read_inode(inode_id);
    // 读取src目录的inode，此时tmp1存储src文件的名称。
    // 接下来找到src文件，并且判断其类型。如果不是FILE，则返回错误
    // 如果是FILE，则inode指向src_file的inode，然后缓存其inode的信息。
    // 并且调用touch创建文件
    for(int i=1;i<6;i++){
        if(inode->block_point[i]==0){
            continue;
        }
        read_dir_item(inode->block_point[i],0);
        for(int k=0;k<8;k++){
            // block_buf[p].type==TYPE_DIR \
            //                 && !strcmp(tmp,block_buf[p].name) \
            //                 && block_buf[p].valid)
            if(block_buf[k].type==TYPE_FILE \
                && !strcmp(tmp1,block_buf[k].name) \
                && block_buf[k].valid){
                    char* new_argv[MAXARGS];
                    strcpy(new_argv[0],"touch");
                    strcpy(new_argv[1],dst_path);
                    if(exec_touch(new_argv,2)<0){
                        return -1;
                    }
                    return 0;
                }
        }
    }


}

int shutdown_filesys(){
    printf("Shutting down file system...\n");
    if(close_disk()<0){
        printf("shutdown error!\n");
        return -1;
    }
    printf("Goodbye!\n");
    sleep(1);
    exit(0);
}