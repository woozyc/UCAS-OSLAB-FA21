#ifndef INCLUDE_LIST_SYS_H_
#define INCLUDE_LIST_SYS_H_

#ifndef NULL
#define NULL 	(void*)0
#endif

typedef struct list_node
{
    struct list_node *next, *prev;
} list_node_t;

typedef list_node_t list_head;

static inline void init_list_head(list_head *head){
	head->next = head;
	head->prev = head;
}
static inline void list_add(list_node_t *node, list_node_t *prev){
	prev->next->prev = node;
	node->next = prev->next;
	node->prev = prev;
	prev->next = node;
}
static inline void list_del(list_node_t *node){
	if(node->prev != NULL && node->next != NULL){
		node->next->prev = node->prev;
		node->prev->next = node->next;
		node->next = NULL;
		node->prev = NULL;
	}
}

#endif
