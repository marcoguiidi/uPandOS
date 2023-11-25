#include "./headers/pcb.h"

static pcb_t pcbTable[MAXPROC];
LIST_HEAD(pcbFree_h);
static int next_pid = 1;

void initPcbs() {
    for (int i = 0; i < MAXPROC; i++){
        list_add_tail(&pcbTable[i].p_list, &pcbFree_h);
    }
}

void freePcb(pcb_t *p) {
    list_add_tail(&p->p_list, &pcbFree_h);
}

pcb_t *allocPcb() {
    if (list_empty(&pcbFree_h) == 1) {
        return NULL;
    } else {
        pcb_t* removed = container_of(&pcbFree_h, pcb_t, p_list);
        
        removed->p_list = (struct list_head)LIST_HEAD_INIT(removed->p_list);
        removed->p_parent = NULL;
        removed->p_child = (struct list_head)LIST_HEAD_INIT(removed->p_child);
        removed->p_sib = (struct list_head)LIST_HEAD_INIT(removed->p_sib);
        //removed->p_s = (state_t){0}; //initialize all to 0
        removed->p_time = 0;
        removed->msg_inbox = (struct list_head)LIST_HEAD_INIT(removed->msg_inbox);
        removed->p_supportStruct = NULL;
        removed->p_pid = 0;

        list_del(&removed->p_list);
        return removed;
    }
}

void mkEmptyProcQ(struct list_head *head) {
}

int emptyProcQ(struct list_head *head) {
}

void insertProcQ(struct list_head *head, pcb_t *p) {
}

pcb_t *headProcQ(struct list_head *head) {
}

pcb_t *removeProcQ(struct list_head *head) {
}

pcb_t *outProcQ(struct list_head *head, pcb_t *p) {
}

int emptyChild(pcb_t *p) {
}

void insertChild(pcb_t *prnt, pcb_t *p) {
}

pcb_t *removeChild(pcb_t *p) {
}

pcb_t *outChild(pcb_t *p) {
}
