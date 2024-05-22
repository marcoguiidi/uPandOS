#ifndef KLOG_H
#define KLOG_H

void next_line(void);
void next_char(void);

// Print str to klog
void klog_print(char *str);

/* 
*   Funzione per la stampa di numeri nei registri di memoria usati per il debugging.
*   N.B. La funzione stampa numeri in un intervallo compreso tra 0 e 99
*/
void klog_print_dec(unsigned int num);

// Princ a number in hexadecimal format (best for addresses)
void klog_print_hex(unsigned int num);

#define KLOG_ERROR(explaintext)\
    klog_print(explaintext);\
    next_line();\
    klog_print(__FILE__);\
    klog_print(" at line: ");\
    klog_print_dec(__LINE__);\
    next_line();

#define KLOG_PANIC(explaintext)\
    KLOG_ERROR(explaintext)\
    PANIC();

#endif