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
    *head = (struct list_head)LIST_HEAD_INIT(*head);
}

int emptyProcQ(struct list_head *head) {
    return list_empty(head);
}

void insertProcQ(struct list_head *head, pcb_t *p) {
    list_add_tail(&p->p_list, head); // is a queue, element in goes last
}

pcb_t *headProcQ(struct list_head *head) {
    if (emptyProcQ(head)) return NULL; 

    return container_of(head->next, pcb_t, p_list);
}

pcb_t *removeProcQ(struct list_head *head) {
    if (list_empty(head)) {
        return NULL;
    } else {
        pcb_t* proc = container_of(list_next(head), pcb_t, p_list); // is a queue, last in first out
        list_del(&proc->p_list);
        return proc;
    }
}

pcb_t *outProcQ(struct list_head *head, pcb_t *p) {
    if(!list_empty(head)){
        struct list_head* iter;
        list_for_each(iter, head) {
            pcb_t* proc = container_of(iter, pcb_t, p_list);
            if (p == proc){
                list_del(iter);
                return p;
            }
        }
    }
    return NULL;
}

int emptyChild(pcb_t *p) {
    return list_empty(&p->p_child);
}

void insertChild(pcb_t *prnt, pcb_t *p) {
    list_add_tail(&p->p_sib, &prnt->p_child);
    p->p_parent = prnt;
}

pcb_t *removeChild(pcb_t *p) {
    if (!emptyChild(p)) {
        pcb_t* f_child = container_of(list_next(&p->p_child), pcb_t, p_sib);
        list_del(&f_child->p_sib);
        return f_child;
    }
    return NULL;
}

pcb_t *outChild(pcb_t *p) {
    if(p->p_parent != NULL){
        list_del(&p->p_sib);
        return p;
    }
    return NULL;
}
