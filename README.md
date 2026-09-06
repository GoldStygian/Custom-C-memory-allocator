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
    - Il program break (Concetto) è l'indirizzo che segna la fine dell'heap. Spostarlo verso l'alto = allocare; verso il basso = liberare.

## Perche serve un allocatore
Per evitare di effettuare continue system call che richiedono un context switch verso il kernel

### Perche non si usa solo mmap()
1. Overhead della system call

Sia sbrk che mmap sono system call, quindi entrambe richiedono un context switch verso il kernel. Ma il costo di mmap è generalmente più alto di sbrk, perché deve fare più lavoro interno: cercare una regione libera nello spazio degli indirizzi, aggiornare le tabelle di pagina, creare nuove strutture dati nel kernel (VMA, "virtual memory area") per tracciare quella mappatura.

Se dovessi chiamare mmap per ogni malloc(16) che fai nel tuo programma, il costo della system call dominerebbe completamente il tempo di esecuzione. Un allocatore deve essere velocissimo — spesso viene chiamato migliaia di volte al secondo.

2. Granularità: le pagine

mmap lavora a livello di pagine di memoria (tipicamente 4KB su x86). Non puoi mappare "20 byte": il kernel ti arrotonda sempre almeno a una pagina intera.

Quindi se il tuo programma fa continuamente piccole allocazioni (8, 16, 32 byte...), usare mmap per ciascuna significherebbe sprecare enormi quantità di memoria — ogni allocazione da pochi byte occuperebbe comunque un'intera pagina da 4KB, con frammentazione interna pazzesca.

3. Frammentazione dello spazio degli indirizzi e delle TLB

Ogni mmap crea una nuova voce nella tabella delle VMA del kernel. Avere migliaia di piccole mappature indipendenti:

rallenta le operazioni del kernel che devono attraversare quella lista (es. gestione dei page fault)
aumenta la pressione sulla TLB (Translation Lookaside Buffer, la cache hardware degli indirizzi tradotti), perché tante piccole regioni sparse nello spazio virtuale generano più "cache miss" nella traduzione indirizzo virtuale → fisico, rispetto ad avere pochi grandi segmenti contigui.
4. sbrk/heap sono ottimi per il pattern tipico di allocazione

La maggior parte dei programmi fa esattamente questo: tante piccole allocazioni, spesso allocate e liberate in ordine simile (LIFO-ish), che si prestano bene a un unico heap contiguo gestito internamente con free list, coalescing, ecc. — esattamente la struttura a "blocchi" di cui parlavamo. L'allocatore fa il lavoro pesante una volta sola (poche chiamate sbrk per ottenere grosse fette), e poi serve migliaia di richieste utente senza mai toccare il kernel.

Quindi la strategia ibrida ha senso perché...
Caratteristica	sbrk (heap)	mmap
Costo per chiamata	medio	più alto
Granularità	libera (l'allocatore gestisce byte a byte via blocchi)	pagina intera (4KB min)
Adatto a	tante piccole allocazioni	poche allocazioni grandi
Restituzione memoria al SO	solo dalla fine, se contiguo	immediata e indipendente, con munmap
Frammentazione indirizzi kernel	bassa (un solo segmento)	cresce con il numero di mappature
In pratica

Per le piccole allocazioni: conviene ammortizzare il costo di poche sbrk su tantissime richieste malloc, gestendo la suddivisione internamente (i "blocchi" della tua guida).

Per le allocazioni grandi (es. 10MB): il discorso si ribalta. Un'allocazione così grande, se stesse nell'heap tradizionale, rischierebbe di "bloccare" il break in una posizione alta finché non viene liberata — impedendo di restituire quella memoria al sistema anche se tutto il resto dell'heap è vuoto. Con mmap, invece, quella singola grande allocazione vive per conto suo e torna al sistema immediatamente e in modo pulito con munmap, non appena fai free().

Ecco perché glibc (e la maggior parte degli allocatori seri) usa entrambe le strategie, scegliendo in base alla dimensione della richiesta — non è un compromesso ma la combinazione dei punti di forza di ciascun meccanismo.

### Funzioni brk(), sbrk() e mmap()

brk() e il suo wrapper sbrk() \[system call in Linux\] \[Per allocazioni piccole\]: 
- int   brk(void *addr);       // imposta il "program break" a un indirizzo assoluto
- void *sbrk(intptr_t incr);   // sposta il break di incr byte (relativo)

[codice][dati statici][ HEAP --------> ][spazio non mappato...][stack]
                                       ^
                                  program break

Ogni processo Linux ha un segmento di memoria chiamato heap, che si trova subito dopo i dati statici del programma (.data, .bss). Il punto che segna la fine corrente di questo segmento si chiama program break (o "break"). Tutto ciò che sta prima del break è memoria valida e utilizzabile dal processo. Tutto ciò che sta dopo non è ancora mappato: se ci provi ad accedere, ottieni un segmentation fault.

brk e sbrk sono le due chiamate di sistema che permettono di spostare questo confine, cioè di far crescere (o restringere) l'heap.

#### brk()
Imposta il program break a un indirizzo assoluto specificato da addr. Se addr è maggiore dell'indirizzo corrente, l'heap cresce (nuova memoria diventa disponibile); se è minore, l'heap si restringe (memoria viene rilasciata al sistema).

Ritorna 0 in caso di successo, -1 in caso di errore.

#### sbrk()
Questa è la versione più comoda, ed è quella usata quasi sempre nella pratica. Invece di un indirizzo assoluto, prende un incremento relativo:

- sbrk(4096) → sposta il break in avanti di 4096 byte, facendo crescere l'heap di 4KB, e restituisce il vecchio valore del break (cioè l'inizio della nuova memoria appena allocata).
- sbrk(-4096) → restringe l'heap di 4KB.
- sbrk(0) → non modifica nulla, ma restituisce l'indirizzo corrente del break — utile solo per "interrogare" dove si trova.

NOTA: Entrambi modificano la regione dell'heap

#### mmap()

brk/sbrk operano esclusivamente spostando il confine dell'heap classico — quella singola regione contigua che si trova subito dopo i dati statici del programma.

mmap, invece, opera in una regione completamente diversa dello spazio degli indirizzi (spesso chiamata "mmap region" o "memory mapping segment", tipicamente vicino allo stack, ma la posizione esatta dipende dal sistema/ASLR). Con mmap puoi:

creare una nuova mappatura di memoria ovunque il kernel trovi spazio libero nello spazio virtuale
farlo in modo completamente indipendente da altre mappature: ogni chiamata mmap è una regione a sé, che puoi liberare con munmap senza影响are le altre
mappare non solo memoria anonima (per allocazioni), ma anche file (memory-mapped file I/O), librerie condivise (.so), memoria condivisa tra processi, ecc.

# Note Progettuali

- size_t è un tipo intero senza segno definito nella libreria standard di C (tipicamente in <stddef.h>) utilizzato per rappresentare la dimensione di oggetti in byte e i contatori restituiti da operatori come sizeof.  Essendo senza segno, non può contenere valori negativi, il che lo rende ideale per indicare lunghezze, indici di array e dimensioni di buffer