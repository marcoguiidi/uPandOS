#ifndef KLOG_H
#define KLOG_H

// Print str to klog
void klog_print(char *str);

/* 
*   Funzione per la stampa di numeri nei registri di memoria usati per il debugging.
*   N.B. La funzione stampa numeri in un intervallo compreso tra 0 e 99
*/
void klog_print_dec(unsigned int num);

// Princ a number in hexadecimal format (best for addresses)
void klog_print_hex(unsigned int num);


#endif