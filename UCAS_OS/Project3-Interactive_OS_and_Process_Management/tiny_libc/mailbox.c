#include <mailbox.h>
#include <string.h>
#include <sys/syscall.h>

mailbox_t mbox_open(char *name)
{
    // TO DO:
    return invoke_syscall(SYSCALL_MAILBOX_OPEN, (long)name, IGNORE, IGNORE, IGNORE);
}

void mbox_close(mailbox_t mailbox)
{
    // TO DO:
    invoke_syscall(SYSCALL_MAILBOX_CLOSE, (long)mailbox, IGNORE, IGNORE, IGNORE);
}

int mbox_send(mailbox_t mailbox, void *msg, int msg_length)
{
    // TO DO:
    return invoke_syscall(SYSCALL_MAILBOX_SEND, (long)mailbox, (long)msg, (long)msg_length, IGNORE);
}

int mbox_recv(mailbox_t mailbox, void *msg, int msg_length)
{
    // TO DO:
    return invoke_syscall(SYSCALL_MAILBOX_RECV, (long)mailbox, (long)msg, (long)msg_length, IGNORE);
}
