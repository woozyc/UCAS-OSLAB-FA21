/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                       System call related processing
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#ifndef INCLUDE_SYSCALL_H_
#define INCLUDE_SYSCALL_H_

//#include <type.h>
#include <sys/os_type.h>
#include <os/syscall_number.h>
#include <stdint.h>
 
//#define SCREEN_HEIGHT 80
 
pid_t sys_exec(const char *file_name, int argc, char* argv[], spawn_mode_t mode);
void sys_show_exec();
#include <os.h>

extern long invoke_syscall(long, long, long, long, long);

void sys_sleep(uint32_t);

void sys_write(char *);
char sys_read();
void sys_move_cursor(int, int);
void sys_reflush();

long sys_get_timebase();
long sys_get_tick();

void sys_priority(task_priority_t priority);
int sys_fork();
uint32_t sys_get_wall_time(uint32_t *_time_elapsed);

pid_t sys_spawn(task_info_t *info, void* arg, spawn_mode_t mode, int hart_mask);
void sys_exit(void);
int sys_kill(pid_t pid);
int sys_waitpid(pid_t pid);
void sys_process_show(void);
void sys_screen_clear(void);
pid_t sys_getpid();
int sys_get_char();

char sys_serial_read();
void sys_serial_write(char *s);
void sys_screen_clear();
char sys_getchar();

void sys_setmask(int mask, int pid);

pid_t sys_exec(const char* file_name, int argc, char* argv[], spawn_mode_t mode);
void* shmpageget(int key);
void shmpagedt(void *addr);

#define BINSEM_OP_LOCK 0 // mutex acquire
#define BINSEM_OP_UNLOCK 1 // mutex release

int binsemget(int key);
int binsemop(int binsem_id, int op);
long sys_net_recv(uintptr_t addr, size_t length, int num_packet, size_t* frLength);
void sys_net_send(uintptr_t addr, size_t length);
void sys_net_irq_mode(int mode);

void sys_exec_show();
long sys_net_recv(uintptr_t addr, size_t length, int num_packet, size_t* frLength);
void sys_net_send(uintptr_t addr, size_t length);
void do_net_irq_mode(int mode);

void sys_mkfs();
void sys_statfs();
void sys_cd(char *dir);
void sys_mkdir(char *dir);
void sys_rmdir(char *dir);
void sys_ls(char *mode, char *dir);
void sys_touch(char *file);
void sys_cat(char *file);
void sys_ln(char *dst, char *src);
void sys_rm(char *file);

int sys_fopen(char *name, int access);
int sys_fread(int fd, char *buff, int size);
int sys_fwrite(int fd, char *buff, int size);
void sys_fclose(int fd);
int sys_lseek(int fd, int offset, int whence);
#endif
