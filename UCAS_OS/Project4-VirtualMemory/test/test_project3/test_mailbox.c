#include <time.h>
#include <test3.h>
#include <mthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <mailbox.h>
#include <sys/syscall.h>

#define MAX_MAILBOX_OPERATOR 3
// struct task_info strserver_task = {(uintptr_t)&strServer, USER_PROCESS};
// struct task_info strgenerator_task = {(uintptr_t)&strGenerator, USER_PROCESS};

static const char initReq[] = "clientInitReq";
static const int initReqLen = sizeof(initReq);

struct MsgHeader
{
    int length;
    uint32_t checksum;
    pid_t sender;
};

const uint32_t MOD_ADLER = 65521;

/* adler32 algorithm implementation from: https://en.wikipedia.org/wiki/Adler-32 */
uint32_t adler32(unsigned char *data, size_t len)
{
    uint32_t a = 1, b = 0;
    size_t index;

    // Process each byte of the data in order
    for (index = 0; index < len; ++index)
    {
        a = (a + data[index]) % MOD_ADLER;
        b = (b + a) % MOD_ADLER;
    }

    return (b << 16) | a;
}

int clientInitReq(const char* buf, int length)
{
    if (length != initReqLen) return 0;
    for (int i = 0; i < initReqLen; ++i) {
        if (buf[i] != initReq[i]) return 0;
    }
    return 1;
}

void strServer(void)
{
    char msgBuffer[MAX_MBOX_LENGTH];
    struct MsgHeader header;
    int64_t correctRecvBytes = 0;
    int64_t errorRecvBytes = 0;
    int64_t blockedCount = 0;
    int clientPos = 11;

    mailbox_t mq = mbox_open("str-message-queue");
    mailbox_t posmq = mbox_open("pos-message-queue");
    sys_move_cursor(1, 10);
    printf("[Server] server started");
    sys_sleep(1);

    for (;;)
    {
        blockedCount += mbox_recv(mq, &header, sizeof(struct MsgHeader));
        blockedCount += mbox_recv(mq, msgBuffer, header.length);

        uint32_t checksum = adler32(msgBuffer, header.length);
        if (checksum == header.checksum) {
            correctRecvBytes += header.length;
        } else {
            errorRecvBytes += header.length;
        }

        sys_move_cursor(1, 10);
        printf("[Server]: recved msg from %d (blocked: %ld, correctBytes: %ld, errorBytes: %ld)",
              header.sender, blockedCount, correctRecvBytes, errorRecvBytes);

        if (clientInitReq(msgBuffer, header.length)) {
            mbox_send(posmq, &clientPos, sizeof(int));
            ++clientPos;
        }

        sys_sleep(1);
    }
}

int clientSendMsg(mailbox_t *mq, const char* content, int length)
{
    int i;
    char msgBuffer[MAX_MBOX_LENGTH] = {0};
    struct MsgHeader* header = (struct MsgHeader*)msgBuffer;
    char* _content = msgBuffer + sizeof(struct MsgHeader);
    header->length = length;
    header->checksum = adler32(content, length);
    header->sender = sys_getpid();

    for (i = 0; i < length; ++i) {
        _content[i] = content[i];
    }
    return mbox_send(mq, msgBuffer, length + sizeof(struct MsgHeader));
}

void generateRandomString(char* buf, int len)
{
    static const char alpha[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-={};|[],./<>?!@#$%^&*";
    static const int alphaSz = sizeof(alpha) / sizeof(char);
    int i = len - 2;
    buf[len - 1] = '\0';
    while (i >= 0) {
        buf[i] = alpha[rand() % alphaSz];
        --i;
    }
}

void strGenerator(void)
{
    mailbox_t mq = mbox_open("str-message-queue");
    mailbox_t posmq = mbox_open("pos-message-queue");

    int len = 0;
    int strBuffer[MAX_MBOX_LENGTH - sizeof(struct MsgHeader)];
    clientSendMsg(mq, initReq, initReqLen);
    int position = 11;
    mbox_recv(posmq, &position, sizeof(int));
    int blocked = 0;
    int64_t bytes = 0;
    int pid = sys_getpid();

    sys_move_cursor(1, position);
    printf("[Client %d] server started", pid);
    sys_sleep(1);
    for (;;)
    {
        len = (rand() % ((MAX_MBOX_LENGTH - sizeof(struct MsgHeader))/2)) + 1;
        generateRandomString(strBuffer, len);
        blocked += clientSendMsg(mq, strBuffer, len);
        bytes += len;

        sys_move_cursor(1, position);
        printf("[Client %d] send bytes: %ld, blocked: %d", pid, bytes, blocked);
        sys_sleep(1);
    }
}

//added, for task 4
/*int mailbox_send_recv(mailbox_t mb, char *sendbuffer, char *recvbuffer, int *sendlen, int *recvlen, int act_prefer){
	;
}*/
void mailbox_act(int printloc){
	mailbox_t m1 = mbox_open("mbox_act_1");
	mailbox_t m2 = mbox_open("mbox_act_2");
	mailbox_t m3 = mbox_open("mbox_act_3");
	int m_num;
	
    int send_len = 0, recv_len = 0;
    int act_result;
    char strBuffer[MAX_MBOX_LENGTH - sizeof(struct MsgHeader)];
    char msgBuffer[MAX_MBOX_LENGTH];
    int pid = sys_getpid();
    sys_move_cursor(1, printloc);
    printf("[Operator %d] Started", pid);
    sys_sleep(1);
	//select send/recv and mbox randomly
	int act_prefer;
	mailbox_t mbox;
	while(1){
		m_num = rand() % 3;
		switch(m_num){
			case 0:
				mbox = m1; break;
			case 1:
				mbox = m2; break;
			case 2:
				mbox = m3; break;
			default:
				mbox = 0; break;
		}
		send_len = (rand() % ((MAX_MBOX_LENGTH - sizeof(struct MsgHeader))/2)) + 1;
		act_prefer = rand() % 2;
		recv_len = send_len;
    	generateRandomString(strBuffer, send_len);
    	//act_result = mailbox_send_recv(mbox, strBuffer, msgBuffer, &send_len, &recv_len, act_prefer);
    	act_result = mbox_act(mbox, strBuffer, send_len, act_prefer);
		while(act_result != act_prefer){
    		sys_move_cursor(1, printloc);
			if(!act_result){
				printf("[Operator %d] Sent %s%d bytes to   mailbox %d ", pid, (send_len/10) ? "" : " ", send_len, m_num);
			}else{
				printf("[Operator %d] Recv %s%d bytes from mailbox %d ", pid, (send_len/10) ? "" : " ", recv_len, m_num);
			}
			printf(", Try %s failed . ", act_prefer ? "op_recv" : "op_send");
			send_len = (rand() % ((MAX_MBOX_LENGTH - sizeof(struct MsgHeader))/2)) + 1;
			act_prefer = rand() % 2;
			recv_len = send_len;
    		generateRandomString(strBuffer, send_len);
    		//act_result = mailbox_send_recv(mbox, strBuffer, msgBuffer, &send_len, &recv_len, act_prefer);
    		act_result = mbox_act(mbox, strBuffer, send_len, act_prefer);
			sys_sleep(2);
		}
    	sys_move_cursor(1, printloc);
		if(!act_result){
			printf("[Operator %d] Sent %s%d bytes to   mailbox %d ", pid, (send_len/10) ? "" : " ", send_len, m_num);
		}else{
			printf("[Operator %d] Recv %s%d bytes from mailbox %d ", pid, (send_len/10) ? "" : " ", recv_len, m_num);
		}
		printf(", Try %s succeed.  ", act_prefer ? "op_recv" : "op_send");
		sys_sleep(2);
    }
	sys_exit();
}

void mailbox_test(void){
	struct task_info mailbox_operator = {(uintptr_t)&mailbox_act, USER_PROCESS, P_4};
	int i;
	int pids[MAX_MAILBOX_OPERATOR];
	for(i = 0; i < MAX_MAILBOX_OPERATOR; i++){
		pids[i] = sys_spawn(&mailbox_operator, (void*)(long)(i+1), ENTER_ZOMBIE_ON_EXIT, 0);
	}
	for(i = 0; i < MAX_MAILBOX_OPERATOR; i++){
		sys_waitpid(pids[i]);
	}
	sys_exit();
}
