#include "./headers/msg.h"

static msg_t msgTable[MAXMESSAGES];
LIST_HEAD(msgFree_h);

void initMsgs() {
    for (int i = 0; i < MAXMESSAGES; i++){
        list_add_tail(&msgTable[i].m_list, &msgFree_h);
    }
}

void freeMsg(msg_t *m) {
    list_add_tail(&m->m_list, &msgFree_h);
}

msg_t *allocMsg() {
    if (list_empty(&msgFree_h) == 1){
        return NULL;
    }

    else{
        msg_t* removed = container_of(list_prev(&msgFree_h), msg_t, m_list);
        list_del(&removed->m_list);

        removed->m_payload = 0;
        INIT_LIST_HEAD(&removed->m_list);
        removed->m_sender = NULL;
        
        return removed;
    }
}

void mkEmptyMessageQ(struct list_head *head) {
    struct list_head queue = LIST_HEAD_INIT(queue);
    head = &queue;
    return NULL;
}

int emptyMessageQ(struct list_head *head) {
    return list_empty(head);
}

void insertMessage(struct list_head *head, msg_t *m) {
    list_add_tail(&m->m_list, head);
}

void pushMessage(struct list_head *head, msg_t *m) {
}

msg_t *popMessage(struct list_head *head, pcb_t *p_ptr) {
}

msg_t *headMessage(struct list_head *head) {
}
