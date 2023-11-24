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
        msg_t* removed = container_of(&msgFree_h, msg_t, m_list);
        list_del(&removed->m_list);
        return removed;
    }
}

void mkEmptyMessageQ(struct list_head *head) {
}

int emptyMessageQ(struct list_head *head) {
}

void insertMessage(struct list_head *head, msg_t *m) {
}

void pushMessage(struct list_head *head, msg_t *m) {
}

msg_t *popMessage(struct list_head *head, pcb_t *p_ptr) {
}

msg_t *headMessage(struct list_head *head) {
}
