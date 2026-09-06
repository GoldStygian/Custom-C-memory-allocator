#include <stdio.h>

/*

utente chiama malloc()
    -> controlla se c'è un blocco libero di dimensione sufficiente
        -> se sì, restituisce il puntatore al blocco
        -> se no, chiama sbrk() per allocare più memoria
            -> aggiorna il program break
            -> restituisce il puntatore al nuovo blocco

*/

struct header{

    size_t prgbreak;
};

struct block{

    struct block *prev;
    struct block *next;
    int in_use;
    size_t length;
};

struct header header;

void initialize_allocator() {

    header.prgbreak = sbrk(0);

}

int allocate_memory(size_t size) {

    if (header.prgbreak == 0) {
        header.prgbreak = (size_t)sbrk(0);
    }


}