#include <os/fs.h>
#include <os/list.h>
#include <sbi.h>
#include <os/stdio.h>
#include <os/time.h>
#include <os/string.h>
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
//judge whethe the given inode is a directory
int judge_directory(unsigned int ino){
	uint8_t sector_temp_2[512];
	inode_t * inode = ino2inode(sector_temp_2, ino);
	if(inode->type != INODE_DIR){
		prints("> [FS] Cannot cd to a file\n");
		return 0;
	}
	return 1;
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
int get_ino(unsigned int start_ino, char *name, unsigned int *ino, int *dentry_i){
    int len = kstrlen(name);
    //no name
    int i, nextname = 0;
    //starting from root
    if(name[0] == '/'){
    	start_ino = 0;
    	name++;
    	len--;
    }
    if(!len){
		if(ino)
			*ino = start_ino;
    	return 1;
    }
    //if(!judge_directory(start_ino))
    	//return 0;
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
	if(start_inode->type != INODE_DIR){
		prints("> [FS] Cannot cd to a file\n");
		return 0;
	}
	//search for first-level name in dentry
	dentry_t *dentry = (dentry_t *)sector_temp_3;
	for(i = 0; i < start_inode->size; i++, dentry++){
		if(i % (512/sizeof(dentry_t)) == 0){
			//read datablock from SD card
			dentry = (dentry_t *)sector_temp_3;
			sbi_sd_read(kva2pa((uintptr_t)sector_temp_3), 1,
						superblock->start + superblock->datablock_offset +
						(BLOCK_SZ/512) * start_inode->direct[i / (BLOCK_SZ/sizeof(dentry_t))] +
						((i*sizeof(dentry_t)) % BLOCK_SZ) / 512);
		}
		if(!kstrcmp(dentry->name, name)){
			//find first-level name in dentry
			if(ino)
				*ino = dentry->ino;
			if(dentry_i)
				*dentry_i = i;
			return get_ino(dentry->ino, name + nextname, ino, NULL);
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
void free_block(unsigned int block_num){
	uint8_t sector_temp[512];
	uint8_t sector_temp_2[512];
    sbi_sd_read(kva2pa((uintptr_t)sector_temp), 1, FS_SECTOR);
	superblock_t *superblock = (superblock_t *)sector_temp;
	
	sbi_sd_read(kva2pa((uintptr_t)sector_temp_2), 1, superblock->start + superblock->blockmap_offset + block_num / (512*8));
	sector_temp_2[(block_num % (512*8)) / 8] -= (0x1 << block_num % 8);
	sbi_sd_write(kva2pa((uintptr_t)sector_temp_2), 1, superblock->start + superblock->blockmap_offset + block_num / (512*8));
}
void free_inode(unsigned int inode_num){
	uint8_t sector_temp[512];
	uint8_t sector_temp_2[512];
    sbi_sd_read(kva2pa((uintptr_t)sector_temp), 1, FS_SECTOR);
	superblock_t *superblock = (superblock_t *)sector_temp;
	
	sbi_sd_read(kva2pa((uintptr_t)sector_temp_2), 1, superblock->start + superblock->inodemap_offset + inode_num / (512*8));
	sector_temp_2[(inode_num % (512*8)) / 8] -= (0x1 << inode_num % 8);
	sbi_sd_write(kva2pa((uintptr_t)sector_temp_2), 1, superblock->start + superblock->inodemap_offset + inode_num / (512*8));
}
//find the parent of a directory name(second to the last level directory)
//fill the parent inode number to *pa_inode
//and modify the *dir pointer to child name's start position
//return: 0-error, 1-success
int find_parent(char ** dir, int *pa_ino){
	//find parent directory
	int len = kstrlen(*dir);
	uint8_t name_temp[64];
	unsigned int dir_ino = current_dir;
	if(!len || dir[0][0] == '-'){
		prints("> [FS] Illegal directory name\n");
		return 0;
	}
	if(pa_ino)
		*pa_ino = current_dir;
	kmemcpy(name_temp, (uint8_t *)*dir, len+1);
	int name_i;
	for(name_i = len - 1; name_i >= 1; name_i--){
		if(name_temp[name_i] == '/'){
			name_temp[name_i] = 0;
			if(name_temp[name_i-1] == '/' || !get_ino(current_dir, (char *)name_temp, &dir_ino, NULL)){
				prints("> [FS] No such directory\n");
				return 0;
			}
			name_i++;
			break;
		}
	}
    if(!judge_directory(dir_ino)){
    	return 0;
    }
	if(pa_ino){
		*pa_ino = dir_ino;
	}
	*dir += name_i;
	return 1;
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
    kmemcpy((uint8_t *)dentry->name, (uint8_t *)".", 2);
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
	if(!get_ino(current_dir, dir, &dir_ino, NULL)){
		prints("> [FS] No such directory\n");
	}else{
    	if(!judge_directory(dir_ino)){
    		return ;
    	}
		current_dir = dir_ino;
	}
}

void do_mkdir(char *dir){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	//find parent directory
	int dir_ino;
	if(!find_parent(&dir, &dir_ino))
		return 0;
	//check directory name
	int len = kstrlen(dir);
	if(!len || kstrcontain(dir, '/') || kstrcontain(dir, ' ') || dir[0] == '-'){
		prints("> [FS] Illegal directory name\n");
		return ;
	}
	uint8_t name_temp[64];
	kmemcpy(name_temp, (uint8_t *)dir, len+1);
	if(get_ino(dir_ino, (char *)name_temp, NULL, NULL)){
		prints("> [FS] Such file or directory already exits\n");
		return ;
	}
	//make new directory
	//get parent inode
	uint8_t sector_temp_2[512];
	inode_t * pa_inode = ino2inode(sector_temp_2, dir_ino);
	//check permission
	if(pa_inode->mode == PMS_RDONLY){
		prints("> [FS] Parent directory is read-only, permission denied\n");
		return ;
	}
	//modify parent inode
	int new_index = pa_inode->size;
	pa_inode->size++;
	pa_inode->time = get_timer();
	if(pa_inode->size % (BLOCK_SZ/sizeof(dentry_t)) == 1){
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
	new_inode->links = 1;
	new_inode->time = get_timer();
	write_inode_SD(sector_temp_3, new_inode->ino);
	//modify parent dentry
	unsigned int data_number = pa_inode->direct[new_index / (BLOCK_SZ/sizeof(dentry_t))];
	get_datasector(sector_temp_3, data_number, (new_index % (BLOCK_SZ/sizeof(dentry_t))) * sizeof(dentry_t));
	dentry_t *pa_dentry = ((dentry_t *)sector_temp_3) + new_index % (512/sizeof(dentry_t));
	pa_dentry->ino = new_ino;
	kmemcpy((uint8_t *)(pa_dentry->name), (uint8_t *)dir, len+1);
	write_datasector_SD(sector_temp_3, data_number, (new_index % (BLOCK_SZ/sizeof(dentry_t))) * sizeof(dentry_t));
	//setup new dentry for new directory
	get_datasector(sector_temp_3, new_datanum, 0);
	dentry_t *new_dentry = (dentry_t *)sector_temp_3;
	new_dentry->ino = new_ino;
	kmemcpy((uint8_t *)(new_dentry->name), (uint8_t *)".", 2);
	new_dentry++;
	new_dentry->ino = dir_ino;
	kmemcpy((uint8_t *)(new_dentry->name), (uint8_t *)"..", 3);
	write_datasector_SD(sector_temp_3, new_datanum, 0);
}

void do_rmdir(char *dir){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	//find parent directory
	int pa_ino;
	if(!find_parent(&dir, &pa_ino))
		return 0;
	//check directory name
	int len = kstrlen(dir);
	if(!len || kstrcontain(dir, '/') || kstrcontain(dir, ' ') || dir[0] == '-' || dir[0] == '.'){
		prints("> [FS] Illegal directory name\n");
		return ;
	}
	unsigned int chd_ino;
	int dentry_i;
	uint8_t name_temp[64];
	kmemcpy(name_temp, (uint8_t *)dir, len+1);
	if(!get_ino(pa_ino, (char *)name_temp, &chd_ino, &dentry_i)){
		prints("> [FS] Such file or directory does not exist!\n");
		return ;
	}
	//remove directory
	//get parent inode
	uint8_t sector_temp_2[512];
	inode_t * pa_inode = ino2inode(sector_temp_2, pa_ino);
	//check permission
	if(pa_inode->mode == PMS_RDONLY){
		prints("> [FS] Parent directory is read-only, permission denied\n");
		return ;
	}
	//modify parent inode
	int old_index = pa_inode->size;
	pa_inode->size--;
	pa_inode->time = get_timer();
	if(old_index % (BLOCK_SZ/sizeof(dentry_t)) == 1){
		//free a data block for parent dentry
		free_block(pa_inode->direct[old_index / (BLOCK_SZ/sizeof(dentry_t))]);
	}
	write_inode_SD(sector_temp_2, pa_inode->ino);
	uint8_t sector_temp_3[512];
	uint8_t sector_temp_4[512];
	//modify parent dentry
	if(old_index != 2){
		//find the last dentry, copy to delete place
		get_datasector(sector_temp_3, pa_inode->direct[pa_inode->size / (BLOCK_SZ/sizeof(dentry_t))],
					   ((pa_inode->size) % (BLOCK_SZ/sizeof(dentry_t))) * sizeof(dentry_t));
		get_datasector(sector_temp_4, pa_inode->direct[dentry_i / (BLOCK_SZ/sizeof(dentry_t))],
					   ((dentry_i) % (BLOCK_SZ/sizeof(dentry_t))) * sizeof(dentry_t));
		kmemcpy(sector_temp_4 + (((dentry_i) % (BLOCK_SZ/sizeof(dentry_t))) * sizeof(dentry_t)),
				sector_temp_3 + (((pa_inode->size) % (BLOCK_SZ/sizeof(dentry_t))) * sizeof(dentry_t)), sizeof(dentry_t));
	}
	write_datasector_SD(sector_temp_3, pa_inode->direct[pa_inode->size / (BLOCK_SZ/sizeof(dentry_t))],
					    ((pa_inode->size) % (BLOCK_SZ/sizeof(dentry_t))) * sizeof(dentry_t));
	write_datasector_SD(sector_temp_4, pa_inode->direct[dentry_i / (BLOCK_SZ/sizeof(dentry_t))],
					    ((dentry_i) % (BLOCK_SZ/sizeof(dentry_t))) * sizeof(dentry_t));
	//free child dentry
	int i;
	inode_t * chd_inode = ino2inode(sector_temp_2, chd_ino);
	for(i = 1; i <= chd_inode->size; i++){
		if(i % (BLOCK_SZ/sizeof(dentry_t)) == 1){
			//free a data block for child dentry
			free_block(chd_inode->direct[i / (BLOCK_SZ/sizeof(dentry_t))]);
		}
	}
	//free child inode
	free_inode(chd_ino);
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
		if(dir){
			if(!get_ino(current_dir, dir, &dir_ino, NULL)){
				prints("> [FS] No such directory\n");
				return ;
			}
		}else{
			dir_ino = current_dir;
		}
	}else{
		if(!get_ino(current_dir, mode, &dir_ino, NULL)){
			prints("> [FS] No such directory\n");
			return ;
		}
	}
	//judge whether it is a directory
	uint8_t sector_temp_2[512];
	inode_t *inode = ino2inode(sector_temp_2, dir_ino);
	if(inode->type != INODE_DIR){
		prints("> [FS] Cannot cd to a file\n");
		return ;
	}
	//print every entry in the directory
	uint8_t sector_temp_3[512];
	uint8_t sector_temp_4[512];
	dentry_t *dentry;
	inode_t *child_inode;
	int i;
	prints("--total entries: %d\n", inode->size);
	prints("--[name],[type] type: 0-directory 1-file\n");
	for(i = 0; i < inode->size; i++){
		if(i % (512/sizeof(dentry_t)) == 0){
			//offset in a !!SECTOR!!(512B) reaches 0, read a new data sector
			//use offset in a !!DATABLOCK!!(4KB) to read
			get_datasector(sector_temp_3, inode->direct[i / (BLOCK_SZ/sizeof(dentry_t))], (i % (BLOCK_SZ/sizeof(dentry_t))) * sizeof(dentry_t));
		}
		dentry = ((dentry_t *)sector_temp_3) + i % (512/sizeof(dentry_t));
		child_inode = ino2inode(sector_temp_4, dentry->ino);
		if(extend_mode && child_inode->type == INODE_FILE){
			prints("  %s, %d\n", dentry->name, child_inode->type);
			prints("    inode--%d, links--%d, size--%dB\n", child_inode->ino, child_inode->links, child_inode->size);
		}else{
			prints("  %s, %d\n", dentry->name, child_inode->type);
		}
	}
}

void do_touch(char *file){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	//find parent directory
	int dir_ino;
	if(!find_parent(&file, &dir_ino))
		return 0;
	//check directory name
	int len = kstrlen(file);
	if(!len || kstrcontain(file, '/') || kstrcontain(file, ' ') || file[0] == '-' || file[0] == '.'){
		prints("> [FS] Illegal file name\n");
		return ;
	}
	uint8_t name_temp[64];
	kmemcpy(name_temp, (uint8_t *)file, len+1);
	if(get_ino(dir_ino, (char *)name_temp, NULL, NULL)){
		prints("> [FS] Such file or directory already exits\n");
		return ;
	}
	//make new file
	//get parent inode
	uint8_t sector_temp_2[512];
	inode_t * pa_inode = ino2inode(sector_temp_2, dir_ino);
	//check permission
	if(pa_inode->mode == PMS_RDONLY){
		prints("> [FS] Parent directory is read-only, permission denied\n");
		return ;
	}
	//modify parent inode
	int new_index = pa_inode->size;
	pa_inode->size++;
	pa_inode->time = get_timer();
	if(pa_inode->size % (BLOCK_SZ/sizeof(dentry_t)) == 1){
		//alloc a new data block for parent dentry
		pa_inode->direct[pa_inode->size / (BLOCK_SZ/sizeof(dentry_t))] = alloc_block();
	}
	write_inode_SD(sector_temp_2, pa_inode->ino);
	//alloc a new inode
	uint8_t sector_temp_3[512];
	uint16_t new_ino = alloc_inode();
	inode_t * new_inode = ino2inode(sector_temp_3, new_ino);
	new_inode->type = INODE_FILE;
	new_inode->mode = PMS_RDWR;
	new_inode->ino = new_ino;
	unsigned int new_datanum = alloc_block();
	new_inode->direct[0] = new_datanum;
	new_inode->size = 0;
	new_inode->links = 1;
	new_inode->time = get_timer();
	write_inode_SD(sector_temp_3, new_inode->ino);
	//modify parent dentry
	unsigned int data_number = pa_inode->direct[new_index / (BLOCK_SZ/sizeof(dentry_t))];
	get_datasector(sector_temp_3, data_number, (new_index % (BLOCK_SZ/sizeof(dentry_t))) * sizeof(dentry_t));
	dentry_t *pa_dentry = ((dentry_t *)sector_temp_3) + new_index % (512/sizeof(dentry_t));
	pa_dentry->ino = new_ino;
	kmemcpy((uint8_t *)(pa_dentry->name), (uint8_t *)file, len+1);
	write_datasector_SD(sector_temp_3, data_number, (new_index % (BLOCK_SZ/sizeof(dentry_t))) * sizeof(dentry_t));
	//setup new dentry for new file
	get_datasector(sector_temp_3, new_datanum, 0);
	kmemcpy((uint8_t *)(sector_temp_3), (uint8_t *)"", 1);
	write_datasector_SD(sector_temp_3, new_datanum, 0);
}

void do_cat(char *file){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	unsigned int file_ino;
	if(!get_ino(current_dir, file, &file_ino, NULL)){
		prints("> [FS] No such file\n");
		return ;
	}
	uint8_t sector_temp_2[512];
	inode_t *inode = ino2inode(sector_temp_2, file_ino);
	if(inode->type != INODE_FILE){
		prints("> [FS] Cannot cat a directory\n");
		return ;
	}
	int printed_sz;
	uint32_t *block_num;
	uint8_t sector_temp_3[512];
	uint8_t sector_temp_4[512];
	for(printed_sz = 0; printed_sz < inode->size; printed_sz += 512){
		if(printed_sz/BLOCK_SZ < 10){
			get_datasector(sector_temp_3, inode->direct[printed_sz/BLOCK_SZ], printed_sz % BLOCK_SZ);
		}else{
			get_datasector(sector_temp_4, inode->indirect_1[0], ((printed_sz - BLOCK_SZ*10) / BLOCK_SZ) * sizeof(uint32_t));
			block_num = (uint32_t *)(sector_temp_4 + (((((printed_sz - BLOCK_SZ*10) / BLOCK_SZ)) * sizeof(uint32_t)) % 512));
			get_datasector(sector_temp_3, *block_num, (printed_sz - BLOCK_SZ*10) % BLOCK_SZ);
		}
		prints("%s", (char *)sector_temp_3);
	}
	
	prints("\n");
}

void do_ln(char *dst, char *src){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	//find parent directory
	int pa_ino_dst, pa_ino_src;
	if(!find_parent(&dst, &pa_ino_dst) || !find_parent(&src, &pa_ino_src))
		return 0;
	//check file name
	int len_dst = kstrlen(dst);
	int len_src = kstrlen(src);
	if(!len_dst || kstrcontain(dst, '/') || kstrcontain(dst, ' ') || dst[0] == '-' || dst[0] == '.' ||
	   !len_src || kstrcontain(src, '/') || kstrcontain(src, ' ') || src[0] == '-' || src[0] == '.'){
		prints("> [FS] Illegal file name\n");
		return ;
	}
	unsigned int src_ino;
	uint8_t name_temp[64];
	kmemcpy(name_temp, (uint8_t *)src, len_src+1);
	if(!get_ino(pa_ino_src, (char *)name_temp, &src_ino, NULL)){
		prints("> [FS] Such file or directory does not exist!\n");
		return ;
	}
	//src file inode link +1
	uint8_t sector_temp_2[512];
	inode_t * inode_src = ino2inode(sector_temp_2, src_ino);
	if(inode_src->type != INODE_FILE){
		prints("> [FS] Cannot link a directory\n");
		return ;
	}
	inode_src->links++;
	write_inode_SD(sector_temp_2, inode_src->ino);
	//make new file
	//get dst parent inode
	inode_t * pa_inode_dst = ino2inode(sector_temp_2, pa_ino_dst);
	//check permission
	if(pa_inode_dst->mode == PMS_RDONLY){
		prints("> [FS] Parent directory is read-only, permission denied\n");
		return ;
	}
	//modify parent inode
	int new_index = pa_inode_dst->size;
	pa_inode_dst->size++;
	pa_inode_dst->time = get_timer();
	if(pa_inode_dst->size % (BLOCK_SZ/sizeof(dentry_t)) == 1){
		//alloc a new data block for parent dentry
		pa_inode_dst->direct[pa_inode_dst->size / (BLOCK_SZ/sizeof(dentry_t))] = alloc_block();
	}
	write_inode_SD(sector_temp_2, pa_inode_dst->ino);
	//modify parent dentry
	uint8_t sector_temp_3[512];
	unsigned int data_number = pa_inode_dst->direct[new_index / (BLOCK_SZ/sizeof(dentry_t))];
	get_datasector(sector_temp_3, data_number, (new_index % (BLOCK_SZ/sizeof(dentry_t))) * sizeof(dentry_t));
	dentry_t *pa_dentry = ((dentry_t *)sector_temp_3) + new_index % (512/sizeof(dentry_t));
	pa_dentry->ino = src_ino;
	kmemcpy((uint8_t *)(pa_dentry->name), (uint8_t *)dst, len_dst+1);
	write_datasector_SD(sector_temp_3, data_number, (new_index % (BLOCK_SZ/sizeof(dentry_t))) * sizeof(dentry_t));
}

void do_rm(char *file){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	//find parent directory
	int pa_ino;
	if(!find_parent(&file, &pa_ino))
		return 0;
	//check file name
	int len = kstrlen(file);
	if(!len || kstrcontain(file, '/') || kstrcontain(file, ' ') || file[0] == '-' || file[0] == '.'){
		prints("> [FS] Illegal file name\n");
		return ;
	}
	unsigned int chd_ino;
	int dentry_i;
	uint8_t name_temp[64];
	kmemcpy(name_temp, (uint8_t *)file, len+1);
	if(!get_ino(pa_ino, (char *)name_temp, &chd_ino, &dentry_i)){
		prints("> [FS] Such file or directory does not exist!\n");
		return ;
	}
	//remove file
	//get parent inode
	uint8_t sector_temp_2[512];
	inode_t * pa_inode = ino2inode(sector_temp_2, pa_ino);
	//check permission
	if(pa_inode->mode == PMS_RDONLY){
		prints("> [FS] Parent directory is read-only, permission denied\n");
		return ;
	}
	//modify parent inode
	int old_index = pa_inode->size;
	pa_inode->size--;
	pa_inode->time = get_timer();
	if(old_index % (BLOCK_SZ/sizeof(dentry_t)) == 1){
		//free a data block for parent dentry
		free_block(pa_inode->direct[old_index / (BLOCK_SZ/sizeof(dentry_t))]);
	}
	write_inode_SD(sector_temp_2, pa_inode->ino);
	uint8_t sector_temp_3[512];
	uint8_t sector_temp_4[512];
	//modify parent dentry
	if(old_index != 2){
		//find the last dentry, copy to delete place
		get_datasector(sector_temp_3, pa_inode->direct[pa_inode->size / (BLOCK_SZ/sizeof(dentry_t))],
					   ((pa_inode->size) % (BLOCK_SZ/sizeof(dentry_t))) * sizeof(dentry_t));
		get_datasector(sector_temp_4, pa_inode->direct[dentry_i / (BLOCK_SZ/sizeof(dentry_t))],
					   ((dentry_i) % (BLOCK_SZ/sizeof(dentry_t))) * sizeof(dentry_t));
		kmemcpy(sector_temp_4 + (((dentry_i) % (BLOCK_SZ/sizeof(dentry_t))) * sizeof(dentry_t)),
				sector_temp_3 + (((pa_inode->size) % (BLOCK_SZ/sizeof(dentry_t))) * sizeof(dentry_t)), sizeof(dentry_t));
	}
	write_datasector_SD(sector_temp_3, pa_inode->direct[pa_inode->size / (BLOCK_SZ/sizeof(dentry_t))],
					    ((pa_inode->size) % (BLOCK_SZ/sizeof(dentry_t))) * sizeof(dentry_t));
	write_datasector_SD(sector_temp_4, pa_inode->direct[dentry_i / (BLOCK_SZ/sizeof(dentry_t))],
					    ((dentry_i) % (BLOCK_SZ/sizeof(dentry_t))) * sizeof(dentry_t));
	int rm_sz;
	uint32_t *block_num;
	inode_t * chd_inode = ino2inode(sector_temp_2, chd_ino);
	if(chd_inode->links > 1){
		chd_inode->links--;
		write_inode_SD(sector_temp_2, chd_inode->ino);
		return ;
	}
	//free child datablock
	for(rm_sz = 0; rm_sz < chd_inode->size + 1; rm_sz += BLOCK_SZ){
		if(rm_sz/BLOCK_SZ < 10){
			free_block(chd_inode->direct[rm_sz/BLOCK_SZ]);
		}else{
			get_datasector(sector_temp_4, chd_inode->indirect_1[0], ((rm_sz - BLOCK_SZ*10) / BLOCK_SZ) * sizeof(uint32_t));
			block_num = (uint32_t *)(sector_temp_4 + (((((rm_sz - BLOCK_SZ*10) / BLOCK_SZ)) * sizeof(uint32_t)) % 512));
			free_block(*block_num);
		}
	}
	if(rm_sz/BLOCK_SZ >= 10)
		free_block(chd_inode->indirect_1);
	//free child inode
	free_inode(chd_ino);
}

int do_fopen(char *name, int access){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	unsigned int file_ino;
	if(!get_ino(current_dir, name, &file_ino, NULL)){
		prints("> [FS] No such file\n");
		return ;
	}
	uint8_t sector_temp_2[512];
	inode_t *inode = ino2inode(sector_temp_2, file_ino);
	if(inode->type != INODE_FILE){
		prints("> [FS] Cannot fopen a directory\n");
		return ;
	}
	int i, fd = -1;
	for(i = 0; i < MAX_FILE_NUM; i++){
		if(openfile[i].used == 0){
			fd = i;
			break;
		}
	}
	if(fd < 0 || openfile[fd].used){
		prints("> [FS] No free file descriptor\n");
		return -1;
	}else{
		openfile[fd].used = 1;
	}
	openfile[fd].mode = access;
	openfile[fd].ino = inode->ino;
	openfile[fd].r_cursor = 0;
	openfile[fd].w_cursor = 0;
	
	return fd;
}

int do_fread(int fd, char *buff, int size){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	if(fd < 0 || fd >= MAX_FILE_NUM || openfile[fd].used == 0){
		prints("> [FS] File descriptor invalid\n");
		return 0;
	}
	if(size == 0)
		return 1;
	unsigned int file_ino = openfile[fd].ino;
	uint8_t sector_temp_2[512];
	inode_t *inode = ino2inode(sector_temp_2, file_ino);
	//read
	int pad = size / 512;
	int flag = 1;
	//overflow
	if(openfile[fd].r_cursor + size + pad > inode->size)
		flag = 0;
	int read_sz, buff_index = 0, remain, r_offset, sector_remain;
	uint32_t *block_num;
	uint8_t sector_temp_3[512];
	uint8_t sector_temp_4[512];
	for(read_sz = 0; read_sz < (flag ? (size + pad) : (inode->size - openfile[fd].r_cursor)); read_sz += 512){
		//prints("%d ", size);
		remain = size + pad - read_sz;
		r_offset = openfile[fd].r_cursor + read_sz;
		sector_remain = 512 - r_offset % 512;
		if(r_offset/BLOCK_SZ < 10){
			get_datasector(sector_temp_3, inode->direct[r_offset/BLOCK_SZ], r_offset % BLOCK_SZ);
		}else if(r_offset/BLOCK_SZ < 1034){
			get_datasector(sector_temp_4, inode->indirect_1[0], ((r_offset - BLOCK_SZ*10) / BLOCK_SZ) * sizeof(uint32_t));
			block_num = (uint32_t *)(sector_temp_4 + (((((r_offset - BLOCK_SZ*10) / BLOCK_SZ)) * sizeof(uint32_t)) % 512));
			get_datasector(sector_temp_3, *block_num, (r_offset - BLOCK_SZ*10) % BLOCK_SZ);
		}else if(r_offset/BLOCK_SZ < 2058){
			get_datasector(sector_temp_4, inode->indirect_1[1], ((r_offset - BLOCK_SZ*1034) / BLOCK_SZ) * sizeof(uint32_t));
			block_num = (uint32_t *)(sector_temp_4 + (((((r_offset - BLOCK_SZ*1034) / BLOCK_SZ)) * sizeof(uint32_t)) % 512));
			get_datasector(sector_temp_3, *block_num, (r_offset - BLOCK_SZ*1034) % BLOCK_SZ);
		}else{
			get_datasector(sector_temp_4, inode->indirect_1[2], ((r_offset - BLOCK_SZ*2058) / BLOCK_SZ) * sizeof(uint32_t));
			block_num = (uint32_t *)(sector_temp_4 + (((((r_offset - BLOCK_SZ*2058) / BLOCK_SZ)) * sizeof(uint32_t)) % 512));
			get_datasector(sector_temp_3, *block_num, (r_offset - BLOCK_SZ*2058) % BLOCK_SZ);
		}
		kmemcpy((uint8_t *)(buff + buff_index), (uint8_t *)sector_temp_3 + r_offset % 512, (remain >= sector_remain) ? (sector_remain-1) : remain);
		if(remain >= sector_remain){
			buff_index += sector_remain-1;
		}else{
			buff_index += remain;
		}
	}
	openfile[fd].r_cursor += flag ? size + pad : inode->size - openfile[fd].r_cursor;
	return flag;
}

int do_fwrite(int fd, char *buff, int size){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	if(fd < 0 || fd >= MAX_FILE_NUM || openfile[fd].used == 0){
		prints("> [FS] File descriptor invalid\n");
		return 0;
	}
	if(size == 0)
		return 1;
	unsigned int file_ino = openfile[fd].ino;
	uint32_t *block_num;
	uint8_t sector_temp_2[512];
	uint8_t sector_temp_3[512];
	uint8_t sector_temp_4[512];
	inode_t *inode = ino2inode(sector_temp_2, file_ino);
	//jude overflow, alloc datablock
	int pad = size / 512;
	unsigned int old_size = inode->size;
	unsigned int new_size = openfile[fd].w_cursor + size + pad;
	int old_blocks, new_blocks, i, j;
	if(new_size > old_size){
		inode->size = new_size;
		old_blocks = old_size / BLOCK_SZ + 1;
		new_blocks = new_size / BLOCK_SZ + 1;
		if(old_blocks <= 10 && new_blocks > 10)
			inode->indirect_1[0] = alloc_block();
		if(old_blocks <= 1034 && new_blocks > 1034)
			inode->indirect_1[1] = alloc_block();
		if(old_blocks <= 2058 && new_blocks > 2058)
			inode->indirect_1[2] = alloc_block();
		
		for(i = old_blocks; i < new_blocks; i++){
			if(i < 10){
				inode->direct[i] = alloc_block();
				for(j = 0; j < BLOCK_SZ; j+= 512){
					get_datasector(sector_temp_3, inode->direct[i], j);
					kmemcpy(sector_temp_3, (uint8_t *)"\0", 1);
					write_datasector_SD(sector_temp_3, inode->direct[i], j);
				}
			}else if(i < 1034){
				get_datasector(sector_temp_4, inode->indirect_1[0], (i - 10) * sizeof(uint32_t));
				block_num = (uint32_t *)(sector_temp_4 + (((i - 10) * sizeof(uint32_t)) % 512));
				*block_num = alloc_block();
				for(j = 0; j < BLOCK_SZ; j+= 512){
					get_datasector(sector_temp_3, *block_num, j);
					kmemcpy(sector_temp_3, (uint8_t *)"\0", 1);
					write_datasector_SD(sector_temp_3, *block_num, j);
				}
				write_datasector_SD(sector_temp_4, inode->indirect_1[0], (i - 10) * sizeof(uint32_t));
			}else if(i < 2058){
				get_datasector(sector_temp_4, inode->indirect_1[1], (i - 1034) * sizeof(uint32_t));
				block_num = (uint32_t *)(sector_temp_4 + (((i - 1034) * sizeof(uint32_t)) % 512));
				*block_num = alloc_block();
				for(j = 0; j < BLOCK_SZ; j+= 512){
					get_datasector(sector_temp_3, *block_num, j);
					kmemcpy(sector_temp_3, (uint8_t *)"\0", 1);
					write_datasector_SD(sector_temp_3, *block_num, j);
				}
				write_datasector_SD(sector_temp_4, inode->indirect_1[1], (i - 1034) * sizeof(uint32_t));
			}else{
				get_datasector(sector_temp_4, inode->indirect_1[2], (i - 2058) * sizeof(uint32_t));
				block_num = (uint32_t *)(sector_temp_4 + (((i - 2058) * sizeof(uint32_t)) % 512));
				*block_num = alloc_block();
				for(j = 0; j < BLOCK_SZ; j+= 512){
					get_datasector(sector_temp_3, *block_num, j);
					kmemcpy(sector_temp_3, (uint8_t *)"\0", 1);
					write_datasector_SD(sector_temp_3, *block_num, j);
				}
				write_datasector_SD(sector_temp_4, inode->indirect_1[2], (i - 2058) * sizeof(uint32_t));
			}
		}
	}
	write_inode_SD(sector_temp_2, inode->ino);
	//write
	int write_sz, buff_index = 0, remain, w_offset, sector_remain;
	for(write_sz = 0; write_sz < size + pad; write_sz += 512){
		remain = size + pad - write_sz;
		w_offset = openfile[fd].w_cursor + write_sz;
		sector_remain = 512 - w_offset % 512;
		if(w_offset/BLOCK_SZ < 10){
			get_datasector(sector_temp_3, inode->direct[w_offset/BLOCK_SZ], w_offset % BLOCK_SZ);
		}else if(w_offset/BLOCK_SZ < 1034){
			get_datasector(sector_temp_4, inode->indirect_1[0], ((w_offset - BLOCK_SZ*10) / BLOCK_SZ) * sizeof(uint32_t));
			block_num = (uint32_t *)(sector_temp_4 + (((((w_offset - BLOCK_SZ*10) / BLOCK_SZ)) * sizeof(uint32_t)) % 512));
			get_datasector(sector_temp_3, *block_num, (w_offset - BLOCK_SZ*10) % BLOCK_SZ);
		}else if(w_offset/BLOCK_SZ < 2058){
			get_datasector(sector_temp_4, inode->indirect_1[1], ((w_offset - BLOCK_SZ*1034) / BLOCK_SZ) * sizeof(uint32_t));
			block_num = (uint32_t *)(sector_temp_4 + (((((w_offset - BLOCK_SZ*1034) / BLOCK_SZ)) * sizeof(uint32_t)) % 512));
			get_datasector(sector_temp_3, *block_num, (w_offset - BLOCK_SZ*1034) % BLOCK_SZ);
		}else{
			get_datasector(sector_temp_4, inode->indirect_1[2], ((w_offset - BLOCK_SZ*2058) / BLOCK_SZ) * sizeof(uint32_t));
			block_num = (uint32_t *)(sector_temp_4 + (((((w_offset - BLOCK_SZ*2058) / BLOCK_SZ)) * sizeof(uint32_t)) % 512));
			get_datasector(sector_temp_3, *block_num, (w_offset - BLOCK_SZ*2058) % BLOCK_SZ);
		}
		kmemcpy(sector_temp_3 + w_offset % 512, (uint8_t *)(buff + buff_index), (remain >= sector_remain) ? (sector_remain-1) : remain);
		if(remain >= sector_remain){
			buff_index += sector_remain-1;
			kmemcpy(sector_temp_3 + sector_remain-1, (uint8_t *)"\0", 1);
		}else{
			buff_index += remain;
			kmemcpy(sector_temp_3 + w_offset % 512 + remain + 1, (uint8_t *)"\0", 1);
		}
		if(w_offset/BLOCK_SZ < 10){
			write_datasector_SD(sector_temp_3, inode->direct[w_offset/BLOCK_SZ], w_offset % BLOCK_SZ);
		}else{
			write_datasector_SD(sector_temp_3, *block_num, (w_offset - BLOCK_SZ*10) % BLOCK_SZ);
		}
	}
	openfile[fd].w_cursor += size + pad;
	return size;
}

int do_fclose(int fd, char *buff, int size){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	if(fd < 0 || fd >= MAX_FILE_NUM || openfile[fd].used == 0){
		prints("> [FS] File descriptor invalid\n");
		return 0;
	}
	openfile[fd].used = 0;
}

int do_lseek(int fd, int offset, int whence){
	if(!fs_exist()){
		prints("> [FS] Error: file system does not exist!\n");
		return ;
	}
	if(fd < 0 || fd >= MAX_FILE_NUM || openfile[fd].used == 0){
		prints("> [FS] File descriptor invalid\n");
		return 0;
	}
	int pad = offset / 512;
	unsigned int file_ino = openfile[fd].ino;
	uint8_t sector_temp_2[512];
	inode_t *inode = ino2inode(sector_temp_2, file_ino);
	switch(whence){
		case SEEK_SET:
			openfile[fd].r_cursor = offset + pad;
			openfile[fd].w_cursor = offset + pad;
			break;
		case SEEK_CUR:
			openfile[fd].r_cursor += offset + pad;
			openfile[fd].w_cursor += offset + pad;
			break;
		case SEEK_END:
			openfile[fd].r_cursor += inode->size + offset + pad;
			openfile[fd].w_cursor += inode->size + offset + pad;
			break;
		default:
			break;
	}
	return 1;
}

