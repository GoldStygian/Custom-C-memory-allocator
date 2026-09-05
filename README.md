# Note Teoriche
- Regioni di memoria di un programma:
    | Segmento | Contenuto | Gestione | Direzione di Crescita (Tipica) |
    | --- | --- | --- | --- |
    | Text | Codice eseguibile | Statico (dal loader) | Fisso |
    | Data | Globali/Statici inizializzati | Statico | Fisso |
    | BSS | Globali/Statici non inizializzati | Statico | Fisso |
    | Heap | Memoria dinamica | Manuale (malloc/free) | Verso l'alto |
    | Stack | Locali/Parametri/Frame | Automatico | Verso il basso |
- 8 bit = 1 byte
    - Quasi tutti i computer moderni (32-bit e 64-bit) sono byte-addressable: ogni indirizzo punta a un singolo byte (8 bit).  Un sistema a 64 bit può quindi indirizzare fino a 2⁶⁴ byte, non 2⁶⁴ word da 8 byte.
    - Quello che cambia passando a 64 bit è la dimensione dei puntatori (e di long in LP64), non quella di int.
    - A livello hardware, la CPU legge/scrive dalla RAM in word da 64 bit (o anche più, es.  128 bit con AVX). Ma il processore ha la logica per estrarre e ricomporre byte, half-word e word di dimensioni minori da quella parola. Quindi quando accedi a un int da 4 byte, la CPU può leggerlo in una singola operazione di 4 byte (o anche di 8 byte e mascherare i bit non necessari), ma in memoria occupa esattamente 4 byte, non 8. 
    In sintesi: la dimensione di int è definita dal data model del compilatore/OS, non dalla width della CPU. Su praticamente tutte le piattaforme comuni (x86-64, ARM64), sizeof(int) == 4 sia a 32 che a 64 bit.
- Perche serve un allocatore? Analizziamo prima lo standard attuale:
    - Il program break (Concetto) è l'indirizzo che segna la fine dell'heap. xSpostarlo verso l'alto = allocare; verso il basso = liberare.
    - brk() e il suo wrapper sbrk() \[system call in Linux\] \[Per allocazioni piccole\]: 
    int   brk(void *addr);       // imposta il "program break" a un indirizzo assoluto
    void *sbrk(intptr_t incr);   // sposta il break di incr byte (relativo)   