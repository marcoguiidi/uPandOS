#ifndef INITIAL_H_INCLUDED
#define INITIAL_H_INCLUDED

#include "/usr/include/umps3/umps/types.h"
#include "../../headers/listx.h"
#include "../../headers/types.h"
#include "../../headers/const.h"

/*
Declare the Level 3 global variables (1.1)
*/

extern int process_count;
extern int soft_block_count;
extern struct list_head ready_queue;
extern pcb_t* current_process;

/*
queue num
0   line 3 dev. 0
1   line 3 dev. 1
2   line 3 dev. 2
3   line 3 dev. 3
4   line 3 dev. 4
5   line 3 dev. 5
6   line 3 dev. 6
7   line 3 dev. 7

8   line 4 dev. 0
9   line 4 dev. 1
10  line 4 dev. 2
11  line 4 dev. 3
12  line 4 dev. 4
13  line 4 dev. 5
14  line 4 dev. 6
15  line 4 dev. 7

16 line 5 dev. 0
17
18
19
20
21
22
23

24 line 6 dev. 0
25
26
27
28
29
30
31

32 line 7 dev. 0
33
34
35
36
37
38
39

40 line 8 dev. 0
41
42
43
44
45
46
47

48 blocked pbcs waiting a message
49 bolcked pbcs waiting a pseudo clock tick

*/
#define BLOCKED_QUEUE_NUM SEMDEVLEN+2
#define BLOKEDRECV 48
#define BOLCKEDPSEUDOCLOCK 49
extern struct list_head blocked_pcbs[BLOCKED_QUEUE_NUM]; // last one is for the Pseudo-clock


extern passupvector_t* passupvector;

extern pcb_t* ssi_pcb;

/** last used pid */
extern unsigned int lastpid;

/**
return a not used pid
*/



#endif