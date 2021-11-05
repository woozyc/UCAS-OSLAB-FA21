#ifndef INCLUDE_MAIL_BOX_
#define INCLUDE_MAIL_BOX_

#ifndef MAX_MBOX_LENGTH
#define MAX_MBOX_LENGTH 64
#endif

// TO DO: please define mailbox_t;
// mailbox_t is just an id of kernel's mail box.
typedef int mailbox_t;

mailbox_t mbox_open(char *);
void mbox_close(mailbox_t);
int mbox_send(mailbox_t, void *, int);
int mbox_recv(mailbox_t, void *, int);
int mbox_act(mailbox_t mailbox, void *msg, int msg_length, int act_prefer);

#endif
