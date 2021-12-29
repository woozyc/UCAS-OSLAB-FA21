#include <os/fs.h>
#include <os/list.h>
#include <sbi.h>
#include <os/stdio.h>
#include <os/time.h>
#include <pgtable.h>

fd_t openfile[MAX_FILE_NUM];
uint8_t empty_sector[512] = {0};
unsigned int current_dir = 0;

//NOTICE: dentries only support direct address

//count 1s in a binary number
int countbinary_1(unsigned int num){
	int cnt = 0;
	while(num){
		if(num % 2)
			cnt++;
		num /= 2;
	}
	return cnt;
}

//return: whether the file system exists
int fs_exist(){
	uint8_t sector_temp[512];
	sbi_sd_read(kva2pa((uintptr_t)sector_temp), 1, FS_SECTOR);
	superblock_t *superblock = (superblock_t *)sector_temp;
	return superblock->magic == MAGIC_NUM;
}

//read a inode table sector to mem
//return: inode pointer of the inode number
inode_t * ino2inode(uint8_t * sector_temp_2, unsigned int ino){
	uint8_t sector_temp[512];
    sbi_sd_read(kva2pa((uintptr_t)sector_temp), 1, FS_SECTOR);
	superblock_t *superblock = (superblock_t *)sector_temp;
	sbi_sd_read(kva2pa((uintptr_t)sector_temp_2), 1,
    			superblock->start + superblock->inodetable_offset + ino/(512/sizeof(inode_t)));
	return (inode_t *)(sector_temp_2 + (ino * sizeof(inode_t)) % 512);
}
//write inode to SD card
void write_inode_SD(uint8_t * sector_temp_2, unsigned int ino){
	uint8_t sector_temp[512];
    sbi_sd_read(kva2pa((uintptr_t)sector_temp), 1, FS_SECTOR);
	superblock_t *superblock = (superblock_t *)sector_temp;
	sbi_sd_write(kva2pa((uintptr_t)sector_temp_2), 1,
    			 superblock->start + superblock->inodetable_offset + ino/(512/sizeof(inode_t)));
}

//read the data sector(512B) that contains 'offset' to mem
//offset: offset(bytes) in a 4K-datablock
void get_datasector(uint8_t * sector_temp_3, unsigned int block_num, unsigned int offset){
	uint8_t sector_temp[512];
    sbi_sd_read(kva2pa((uintptr_t)sector_temp), 1, FS_SECTOR);
	superblock_t *superblock = (superblock_t *)sector_temp;
	sbi_sd_read(kva2pa((uintptr_t)sector_temp_3), 1,
    			superblock->start + superblock->datablock_offset + block_num*(BLOCK_SZ/512) + offset/512);
}
//write a data sector to SD card
void write_datasector_SD(uint8_t * sector_temp_3, unsigned int block_num, unsigned int offset){
	uint8_t sector_temp[512];
    sbi_sd_read(kva2pa((uintptr_t)sector_temp), 1, FS_SECTOR);
	superblock_t *superblock = (superblock_t *)sector_temp;
	sbi_sd_write(kva2pa((uintptr_t)sector_temp_3), 1,
    			 superblock->start + superblock->datablock_offset + block_num*(BLOCK_SZ/512) + offset/512);
}

//find the target directory or file inode number
//return: 0 for failure, 1 for success
int get_ino(unsigned int start_ino, char *name, unsigned int *ino){
    int len = kstrlen(name);
    //no name
    if(!len)
    	return 1;
    int i, nextname = 0;
    //starting from root
    if(name[0] == '/'){
    	start_ino = 0;
    	name++;
    	len--;
    }
    //find first-level dir/file (or 'name' has only one level)
    for(nextname = 0; nextname < len; nextname++){
    	if(name[nextname] == '/'){
    		name[nextname] = 0;
    		break;
    	}
    }
    nextname += (nextname != len);
	//read inode from SD card
	uint8_t sector_temp[512];
	uint8_t sector_temp_2[512];
	uint8_t sector_temp_3[512];
    sbi_sd_read(kva2pa((uintptr_t)sector_temp), 1, FS_SECTOR);
	superblock_t *superblock = (superblock_t *)sector_temp;
	//get current inode
	inode_t *start_inode = ino2inode(sector_temp_2, start_ino);
	//search for first-level name in dentry
	dentry_t *dentry = (dentry_t *)sector_temp_3;
	for(i = 0; i < start_inode->size; i++, dentry++){
		if(i % (BLOCK_SZ/sizeof(dentry_t)) == 0){
			//read datablock from SD card
			dentry = (dentry_t *)sector_temp_3;
			sbi_sd_read(kva2pa((uintptr_t)sector_temp_3), 1,
						superblock->start + superblock->datablock_offset +
						(BLOCK_SZ/512) * start_inode->direct[i / (BLOCK_SZ/sizeof(dentry_t))]);
		}
		if(!kstrcmp(dentry->name, name)){
			//find first-level name in dentry
			if(ino)
				*ino = dentry->ino;
			return get_ino(dentry->ino, name + nextname, ino);
		}
	}
	//search complete, no such file or dir
	return 0;	
}

//return: the first 0's index in a binary number
int uint8_zero_index(uint8_t num){
	if(num == 255)
		return -1;
	int i;
	for(i = 0; i < 8; i++){
		if(!(num % 2))
			return i;
		num /= 2;
	}
}

//return: new data block number
unsigned int alloc_block(){
	uint8_t sector_temp[512];
	uint8_t sector_temp_2[512];
    sbi_sd_read(kva2pa((uintptr_t)sector_temp), 1, FS_SECTOR);
	superblock_t *superblock = (superblock_t *)sector_temp;
	int i, j, k;
	//find an empty block in block map
    for(i = 0; i < superblock->blockmap_sz; i++){
		sbi_sd_read(kva2pa((uintptr_t)sector_temp_2), 1, superblock->start + superblock->blockmap_offset + i);
		for(j = 0; j < 512; j++){
			if(sector_temp_2[j] != (uint8_t)255u){
				k = uint8_zero_index(sector_temp_2[j]);
				sector_temp_2[j] += (0x1 << k);
				sbi_sd_write(kva2pa((uintptr_t)sector_temp_2), 1, superblock->start + superblock->blockmap_offset + i);
				return (i*512*8) + (j*8) + k;
			}
		}
	}
	prints("> [FS] Disk has no free space!\n");
	return 0xffffffffu;
}
//return: new inode number
unsigned int alloc_inode(){
	uint8_t sector_temp[512];
	uint8_t sector_temp_2[512];
    sbi_sd_read(kva2pa((uintptr_t)sector_temp), 1, FS_SECTOR);
	superblock_t *superblock = (superblock_t *)sector_temp;
	int i, j, k;
	//find an empty inode in inode map
    for(i = 0; i < superblock->inodemap_sz; i++){
		sbi_sd_read(kva2pa((uintptr_t)sector_temp_2), 1, superblock->start + superblock->inodemap_offset + i);
		for(j = 0; j < 512; j++){
			if(sector_temp_2[j] != (uint8_t)255u){
				k = uint8_zero_index(sector_temp_2[j]);
				sector_temp_2[j] += (0x1 << k);
				sbi_sd_write(kva2pa((uintptr_t)sector_temp_2), 1, superblock->start + superblock->inodemap_offset + i);
				return (i*512*8) + (j*8) + k;
			}
		}
	}
	prints("> [FS] File system inode full!\n");
	return 0xffffffffu;
}

void do_mkfs(){
	uint8_t sector_temp[512];
	uint8_t sector_temp_2[512];
	int i;
	superblock_t *superblock = (superblock_t *)sector_temp;
	prints("> [FS] Start initialize filesystem!\n");
	
	//set superblock
	superblock->magic = MAGIC_NUM;
	superblock->start = FS_SECTOR;
	
	superblock->blockmap_offset = 1;
	superblock->blockmap_sz = DATA_SZ/BLOCK_SZ/8/512;
	superblock->inodemap_offset = superblock->blockmap_offset + superblock->blockmap_sz;
	superblock->inodemap_sz = 1;
	superblock->inodetable_offset = superblock->inodemap_offset + superblock->inodemap_sz;
	superblock->inodetable_sz = 8*sizeof(inode_t);
	superblock->datablock_offset = superblock->inodetable_offset + superblock->inodetable_sz;
	superblock->datablock_sz = DATA_SZ/512;
	
	superblock->size = 1 + superblock->blockmap_sz + superblock->inodemap_sz +
					   superblock->inodetable_sz + superblock->datablock_sz;
	
	//write to SD card
	prints("> [FS] Setting superblock...\n");
    sbi_sd_write(kva2pa((uintptr_t)superblock), 1, FS_SECTOR);
    kmemset(superblock, 0, sizeof(superblock_t));
    sbi_sd_read(kva2pa((uintptr_t)superblock), 1, FS_SECTOR);
	prints("       magic : 0x%x\n", superblock->magic);
	prints("       num sector : %d, start sector : %d\n", superblock->size, superblock->start);
	prints("       block map offset : %d\n", superblock->blockmap_offset);
	prints("       inode map offset : %d\n", superblock->inodemap_offset);
	prints("       inode table offset : %d\n", superblock->inodetable_offset);
	prints("       data offset : %d\n", superblock->datablock_offset);
	prints("       inode entry siez : %dB, dir entry size : %dB\n", sizeof(inode_t), sizeof(dentry_t));
    
    //clear maps
    kmemset(empty_sector, 0, 512);
	prints("> [FS] Setting block-map...\n");
    for(i = 0; i < superblock->blockmap_sz; i++)
	    sbi_sd_write(kva2pa((uintptr_t)empty_sector), 1, superblock->start + superblock->blockmap_offset + i);
	prints("> [FS] Setting inode-map...\n");
    for(i = 0; i < superblock->inodemap_sz; i++)
    	sbi_sd_write(kva2pa((uintptr_t)empty_sector), 1, superblock->start + superblock->inodemap_offset + i);
    
    //set root directory
    //set inode
	prints("> [FS] Setting root directory...\n");
    kmemset(sector_temp_2, 0, 512);
    inode_t *inode = (inode_t *)sector_temp_2;
    inode->type = INODE_DIR;
    inode->mode = PMS_RDWR;
    inode->ino = 0;
    inode->direct[0] = 0;
    inode->size = 1;
    inode->time = get_timer();
    sbi_sd_write(kva2pa((uintptr_t)inode), 1, superblock->start + superblock->inodetable_offset);
    //set inode map
    kmemset(sector_temp_2, 0, 512);
    sector_temp_2[0] |= 1;
    sbi_sd_write(kva2pa((uintptr_t)sector_temp_2), 1, superblock->start + superblock->inodemap_offset);
    //set dentry
    kmemset(sector_temp_2, 0, 512);
    dentry_t *dentry = (dentry_t *)sector_temp_2;
    dentry->ino = 0;
    kmemcpy(dentry->name, ".", 2);
    sbi_sd_write(kva2pa((uintptr_t)dentry), 1, superblock->start + superblock->datablock_offset);
    //set block map
    kmemset(sector_temp_2, 0, 512);
    sector_temp_2[0] |= 1;
    sbi_sd_write(kva2pa((uintptr_t)sector_temp_2), 1, superblock->start + superblock->blockmap_offset);
	prints("> Initialize filesystem finished!\n");
}

void do_statfs(){
	uint8_t sector_temp[512];
	uint8_t sector_temp_2[512];
	int i, j;
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	prints("> [FS] Loading file system information...\n");
	//get superblock
    sbi_sd_read(kva2pa((uintptr_t)sector_temp), 1, FS_SECTOR);
	superblock_t *superblock = (superblock_t *)sector_temp;
	//calculate occupancy
	int used_inode = 0;
	for(i = 0; i < superblock->inodemap_sz; i++){
		sbi_sd_read(kva2pa((uintptr_t)sector_temp_2), 1, superblock->start + superblock->inodemap_offset + i);
		for(j = 0; j < 512; j++){
			used_inode += countbinary_1((unsigned int)sector_temp_2[j]);
		}
	}
	int used_block = 0;
	for(i = 0; i < superblock->blockmap_sz; i++){
		sbi_sd_read(kva2pa((uintptr_t)sector_temp_2), 1, superblock->start + superblock->blockmap_offset + i);
		for(j = 0; j < 512; j++){
			used_block += countbinary_1((unsigned int)sector_temp_2[j]);
		}
	}
	prints("       magic : 0x%x\n", superblock->magic);
	prints("       num sector : %d, start sector : %d\n", superblock->size, superblock->start);
	prints("       block map offset : %d\n", superblock->blockmap_offset);
	prints("       inode map offset : %d\n", superblock->inodemap_offset);
	prints("       inode table offset : %d\n", superblock->inodetable_offset);
	prints("       data offset : %d, used : %d\n", superblock->datablock_offset, used_block);
	prints("       inode occupancy : %d/%d\n", used_inode, 512*8);
	prints("       data block occupancy : %d/%d\n", used_block, DATA_SZ/BLOCK_SZ);
	prints("       inode entry siez : %dB, dir entry size : %dB\n", sizeof(inode_t), sizeof(dentry_t));
}

void do_cd(char *dir){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	//find dir inode number
	unsigned int dir_ino;
	if(!get_ino(current_dir, dir, &dir_ino)){
		prints("> [FS] No such directory\n");
	}else{
		current_dir = dir_ino;
	}
}

void do_mkdir(char *dir){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	//check directory name
	int len = kstrlen(dir);
	if(!len || kstrcontain(dir, '.') || kstrcontain(dir, '/') || kstrcontain(dir, ' ') || dir[0] == '-'){
		prints("> [FS] Illegal directory name\n");
		return ;
	}
	uint8_t name_temp[64];
	kmemcpy(name_temp, dir, len+1);
	if(get_ino(current_dir, (char *)name_temp, NULL)){
		prints("> [FS] Such file or directory already exits\n");
		return ;
	}
	//make new directory
	//get current inode
	uint8_t sector_temp_2[512];
	inode_t * pa_inode = ino2inode(sector_temp_2, current_dir);
	//check permission
	if(pa_inode->mode == PMS_RDONLY){
		prints("> [FS] Current directory is read-only, permission denied\n");
		return ;
	}
	//modify parent inode
	pa_inode->size++;
	pa_inode->time = get_timer();
	if(pa_inode->size % (BLOCK_SZ/sizeof(dentry_t)) == 0){
		//alloc a new data block for parent dentry
		pa_inode->direct[pa_inode->size / (BLOCK_SZ/sizeof(dentry_t))] = alloc_block();
	}
	write_inode_SD(sector_temp_2, pa_inode->ino);
	//alloc a new inode
	uint8_t sector_temp_3[512];
	uint16_t new_ino = alloc_inode();
	inode_t * new_inode = ino2inode(sector_temp_3, new_ino);
	new_inode->type = INODE_DIR;
	new_inode->mode = PMS_RDWR;
	new_inode->ino = new_ino;
	unsigned int new_datanum = alloc_block();
	new_inode->direct[0] = new_datanum;
	new_inode->size = 2;
	new_inode->time = get_timer();
	write_inode_SD(sector_temp_3, new_inode->ino);
	//modify parent dentry
	unsigned int data_number = pa_inode->direct[pa_inode->size / (BLOCK_SZ/sizeof(dentry_t))];
	get_datasector(sector_temp_3, data_number, (pa_inode->size % (BLOCK_SZ/sizeof(dentry_t))) * sizeof(dentry_t));
	dentry_t *pa_dentry = ((dentry_t *)sector_temp_3) + pa_inode->size % (512/sizeof(dentry_t));
	pa_dentry->ino = new_ino;
	kmemcpy(pa_dentry->name, dir, len+1);
	write_datasector_SD(sector_temp_3, data_number, (pa_inode->size % (BLOCK_SZ/sizeof(dentry_t))) * sizeof(dentry_t));
	//setup new dentry for new directory
	get_datasector(sector_temp_3, new_datanum, 0);
	dentry_t *new_dentry = (dentry_t *)sector_temp_3;
	new_dentry->ino = new_ino;
	kmemcpy(new_dentry->name, ".", 2);
	new_dentry++;
	new_dentry->ino = current_dir;
	kmemcpy(new_dentry->name, "..", 3);
	write_datasector_SD(sector_temp_3, new_datanum, 0);
}

void do_rmdir(char *dir){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	prints("rmdir to be done\n");
	;
}

void do_ls(char *mode, char *dir){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	int extend_mode = 0;
	int len = kstrlen(mode);
	//get target inode number
	unsigned int dir_ino;
	if(!len){
		dir_ino = current_dir;
	}else if(mode[0] == '-'){
		if(!kstrcmp(mode, "-l"))
			extend_mode = 1;
		else{
			prints("> [FS] Unknow argument %s\n", mode);
			return ;
		}
		if(!get_ino(current_dir, dir, &dir_ino)){
			prints("> [FS] No such directory\n");
			return ;
		}
	}else{
		if(!get_ino(current_dir, mode, &dir_ino)){
			prints("> [FS] No such directory\n");
			return ;
		}
	}
	//judge whether it is a directory
	uint8_t sector_temp_2[512];
	inode_t *inode = ino2inode(sector_temp_2, dir_ino);
	if(inode->type != INODE_DIR){
		prints("> [FS] No such directory\n");
		return ;
	}
	//print every entry in the directory
	uint8_t sector_temp_3[512];
	uint8_t sector_temp_4[512];
	dentry_t *dentry;
	inode_t *child_inode;
	int i;
	prints("format--[x]:[name], x--0: directory, 1: file\n  ");
	for(i = 0; i < inode->size; i++){
		if(i % (512/sizeof(dentry_t)) == 0){
			//offset in a !!SECTOR!!(512B) reaches 0, read a new data sector
			//use offset in a !!DATABLOCK!!(4KB) to read
			get_datasector(sector_temp_3, inode->direct[i / (BLOCK_SZ/sizeof(dentry_t))], (i % (BLOCK_SZ/sizeof(dentry_t))) * sizeof(dentry_t));
		}
		dentry = ((dentry_t *)sector_temp_3) + i % (512/sizeof(dentry_t));
		child_inode = ino2inode(sector_temp_4, dentry->ino);
		if(extend_mode && child_inode->type == INODE_FILE){
			prints("\n  %d: %s,\n", child_inode->type, dentry->name);
			prints("    inode--%d, links--0, size--%dB\n", child_inode->ino, child_inode->size);
		}else{
			prints("%d: %s ", child_inode->type, dentry->name);
		}
	}
}

void do_touch(char *file){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	prints("touch to be done\n");
	;
}

void do_cat(char *file){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	prints("cat to be done\n");
	;
}

void do_ln(char *dst, char *src){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	prints("ln to be done\n");
	;
}

void do_rm(char *file){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	prints("rm to be done\n");
	;
}

int do_fopen(char *name, int access){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	prints("fopen to be done\n");
	;
}

int do_fread(int fd, char *buff, int size){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	prints("fread to be done\n");
	;
}

int do_fwrite(int fd, char *buff, int size){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	prints("fwrite to be done\n");
	;
}

int do_fclose(int fd, char *buff, int size){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	prints("fclose to be done\n");
	;
}

int do_lseek(int fd, int offset, int whence){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	prints("lseek to be done\n");
	;
}

