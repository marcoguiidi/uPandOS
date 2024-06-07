# MicroPandOS su umps3

## build

```bash
make
```

## run

```bash
umps3 machine-config
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
nello stato di `HALT` l'ssi se ha finito il suo lavoro si troverà nella coda
dei processi bloccati in attesa di un messaggio. 