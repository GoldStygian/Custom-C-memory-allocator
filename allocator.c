#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "allocator.h"

/*
utente chiama malloc()
    -> controlla se c'è un blocco libero di dimensione sufficiente
        -> se sì, restituisce il puntatore al blocco
        -> se no, chiama sbrk() per allocare più memoria
            -> aggiorna il program break
            -> restituisce il puntatore al nuovo blocco

*/

/* TO DO:
    - shrink dei blocchi 
    - thread safety
    - magic number
    - mmap per blocchi grandi 
*/

struct header{

    size_t prgbreak;
    struct block *first_block;
    bool initialized;
};

struct block{

    struct block *prev;
    struct block *next;
    bool in_use;
    size_t length;
};

struct header header;

void debug_print_blocks(void);

/*void initialize_allocator() {

    header.prgbreak = sbrk(0);
    header.first_block = NULL;

}*/

void *allocate_memory(size_t size) {

    // CASO A: non esiste ancora nessun blocco -> dobbiamo crearne uno da zero
    if (header.first_block == NULL) {

        //initialize_allocator();

        header.initialized = true;

        struct block *new_block = request_memory_from_os(size);
        new_block->prev = NULL;
        new_block->next = NULL;
        new_block->in_use = true;
        new_block->length = size;

        header.first_block = new_block;

        return (void *)(new_block + 1); // puntatore ai dati, dopo l'header

    }


    // CASO B: esistono già blocchi -> scorri la lista cercando uno libero e abbastanza grande
    struct block *current = header.first_block;
    struct block * last = NULL; 
    while (current != NULL) {
        if (!current->in_use && current->length >= size) {
            current->in_use = true;
            return (void *)(current + 1);
        }
        last = current; // memorizzo l'ultimo blocco visitato
        current = current->next; // se non lo trovo vado avanti nella lista
    }


    // CASO C: nessun blocco libero adatto trovato -> chiedi altra memoria al SO e AGGANCIA il nuovo blocco alla fine della lista esistente
    // in questo punto del codice current sarà NULL, quindi dobbiamo tornare indietro all'ultimo blocco della lista
    struct block *new_block = request_memory_from_os(size);
    new_block->prev = last;
    new_block->next = NULL;
    new_block->in_use = true;
    new_block->length = size;
    last->next = new_block; // aggancio il nuovo blocco alla fine della lista

    return (void *)(new_block + 1); // puntatore ai dati, dopo l'header

}

struct block *request_memory_from_os(size_t size) {
    size_t total_size = sizeof(struct block) + size; 
    struct block *new_blk = (struct block *)sbrk(total_size);
    
    if (new_blk == (void *)-1) {
        return NULL; // sbrk fallito, memoria esaurita
    }
    
    header.prgbreak = (size_t)sbrk(0); // aggiorna il break corrente
    return new_blk;
}

void free_memory(void *ptr) {
    //debug_print_blocks();
    if (ptr == NULL) return;

    //struct block *blk = (struct block *)ptr -1; // ottieni il puntatore al blocco a partire dal puntatore ai dati

    struct block *current = header.first_block;
    while (current != NULL) {
        
        if ((void *)(current + 1) == ptr) {
            if (!current->in_use) {
                return; // evita una doppia liberazione
            }
        
            current->in_use = false; // segna il blocco come libero
            return; // esci dalla funzione dopo aver liberato il blocco
        }

        current = current->next; // se non lo trovo vado avanti nella lista

    }

    //debug_print_blocks();

}

// ----------

void debug_print_blocks(void) {
    struct block *current = header.first_block;
    size_t index = 0;

    printf("=== Lista blocchi allocatore ===\n");
    printf("Program break: %p\n", (void *)header.prgbreak);

    while (current != NULL) {
        printf(
            "[%zu] blocco=%p, prev=%p, next=%p, stato=%s, dimensione=%zubyte\n",
            index,
            (void *)current,
            (void *)current->prev,
            (void *)current->next,
            current->in_use ? "occupato" : "libero",
            current->length
        );

        current = current->next;
        index++;
    }

    if (index == 0) {
        printf("(lista vuota)\n");
    }

    printf("===============================\n\n");
}