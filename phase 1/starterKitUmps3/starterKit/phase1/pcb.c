#include "./headers/pcb.h"

static pcb_t pcbTable[MAXPROC];
LIST_HEAD(pcbFree_h);
static int next_pid = 1;

void initPcbs() {
    for (int i = 0; i < MAXPROC; i++){
        list_add(&pcbTable[i].p_list, &pcbFree_h);
    }
}

void freePcb(pcb_t *p) {
    list_add(&p->p_list, &pcbFree_h);
}

pcb_t *allocPcb() {
    if (list_empty(&pcbFree_h) == 1) {
        return NULL;
    } else {
        pcb_t* removed = container_of(list_prev(&pcbFree_h), pcb_t, p_list); // pop last element
        list_del(&removed->p_list);
        
        // init all fiends
        removed->p_list = (struct list_head)LIST_HEAD_INIT(removed->p_list);
        removed->p_parent = NULL;
        removed->p_child = (struct list_head)LIST_HEAD_INIT(removed->p_child);
        removed->p_sib = (struct list_head)LIST_HEAD_INIT(removed->p_sib);

        // /usr/include/umps3/umps/types.h
        /*
        #define STATE_GPR_LEN 29

        typedef struct state {
	        unsigned int entry_hi;
	        unsigned int cause;
	        unsigned int status;
	        unsigned int pc_epc;
	        unsigned int gpr[STATE_GPR_LEN];
	        unsigned int hi;
	        unsigned int lo;
        } state_t;
        */

        removed->p_s.entry_hi = 0;
        removed->p_s.cause = 0;
        removed->p_s.status = 0;
        removed->p_s.pc_epc = 0;
        for (int i = 0; i < STATE_GPR_LEN; i++) {
            removed->p_s.gpr[i] = 0;
        }
        removed->p_s.hi = 0;
        removed->p_s.lo = 0;

        removed->p_time = 0;
        removed->msg_inbox = (struct list_head)LIST_HEAD_INIT(removed->msg_inbox);
        removed->p_supportStruct = NULL;
        removed->p_pid = 0;

        return removed;
    }
}

void mkEmptyProcQ(struct list_head *head) {
    // assegno la listra a mano, TODO: da provare con LIST_HEAD_INIT
    head->next = head;
    head->prev = head;
}

int emptyProcQ(struct list_head *head) {
    return list_empty(head);
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
