# MicroPandOS su umps3

## build

```bash
make
```

## run

```bash
umps3 umps3.json
```

## Aspetti implementativi

### Phase2

#### Code dei processi bloccati

> initial.h

Per l'implementazione delle code dei processi bloccati abbiano deciso di
raggruppare tutte le code dei processi bloccati in un array di liste di
lunghezza  `SEMDEVLEN + 2`. Le liste da 0 a 47 sono per i processi bloccati da
devices, la lista 48 mantiene i processi bloccati a causa di una 
`RECEIVEMESSAGE` bloccante, la 49 mantiene i processi in attesa del clock.

#### Scheduler e salvataggio del processo corrente

> scheduler.c

Quando il processo corrente viene spostato in altre code o viene ucciso viene
settato a `NULL` per evitare di inserire lo stesso processo in più code diverse
o più volte, questo comporta che quando si chiama lo scheduler e si deve andare
nello stato di `HALT` la ssi se ha finito il suo lavoro si troverà nella coda
dei processi bloccati in attesa di un messaggio. 

#### Gestione risposte DOIO da SSI

Per inviare il risultato di una DOIO la ssi delega il compito di mandare la
risposta al PCB al gestore degli interrupts in modo da mandare un messaggio
in meno. Per mandare il messaggio il si emula una SENDMESSAGE con
`msg->m_sender = ssi_pcb;` in modo che per il ricevente il messaggio venga dalla
ssi.

### Phase3

#### Assegnamento degli stack per i processi utente

> phase2/initial.c line 49 (getuserstack)

Gli stack per tutti i processi escluso il kernel vengono assegnati nel modo 
seguente: la grandezza degli stack è di una pagina, il primo stack è situato
nell'ultima pagina disponibile in memoria (ramtop), gli stack seguenti sono
situati una pagina prima del precedente.

#### Small Support Level Optimizations implemented

Sono state implementate le seguenti ottimizzazioni
(MicroPandOSPhase3Spec sezione 11): punti 1, 2, 3, 6.
