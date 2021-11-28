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

#ifndef OS_SYSCALL_NUMBER_H_
#define OS_SYSCALL_NUMBER_H_

#define IGNORE 0
#define NUM_SYSCALLS 64

/* define */
#define SYSCALL_SPAWN 0
#define SYSCALL_EXIT 1
#define SYSCALL_SLEEP 2
#define SYSCALL_KILL 3
#define SYSCALL_WAITPID 4
#define SYSCALL_PS 5
#define SYSCALL_GETPID 6
#define SYSCALL_YIELD 7

#define SYSCALL_MUTEX_INIT 8
#define SYSCALL_MUTEX_ACQUIRE 9
#define SYSCALL_MUTEX_RELEASE 10
#define SYSCALL_MUTEX_DESTORY 11
#define SYSCALL_MUTEX_TRYLOCK 12
#define SYSCALL_SMP_INIT 13
#define SYSCALL_SMP_DOWN 14
#define SYSCALL_SMP_UP 15
#define SYSCALL_SMP_DESTORY 16
#define SYSCALL_BARRIER_INIT 17
#define SYSCALL_BARRIER_WAIT 18
#define SYSCALL_BARRIER_DESTORY 19


#define SYSCALL_WRITE 20
#define SYSCALL_READ 21
#define SYSCALL_CURSOR 22
#define SYSCALL_REFLUSH 23
#define SYSCALL_SERIAL_READ 24
#define SYSCALL_SERIAL_WRITE 25
//#define SYSCALL_READ_SHELL_BUFF 26
#define SYSCALL_SCREEN_CLEAR 27

#define SYSCALL_PRIORITY 28
#define SYSCALL_FORK 29

#define SYSCALL_GET_TIMEBASE 30
#define SYSCALL_GET_TICK 31
#define SYSCALL_GET_CHAR 32
#define SYSCALL_GETWALLTIME 33

#define SYSCALL_MAILBOX_OPEN 34
#define SYSCALL_MAILBOX_CLOSE 35
#define SYSCALL_MAILBOX_SEND 36
#define SYSCALL_MAILBOX_RECV 37

#define SYSCALL_MAILBOX_ACT 38

#define SYSCALL_SETMASK 39

#define SYSCALL_EXEC 40
#define SYSCALL_SHMPGGET 41
#define SYSCALL_SHMPGEDT 42

#define SYSCALL_BINSHMPGET 43
#define SYSCALL_BINSHMPOP 44

#define SYSCALL_EXECSHOW 45

#define SYSCALL_THREAD_CREATE 46
#define SYSCALL_THREAD_JION 47

#endif
