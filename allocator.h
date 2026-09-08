#ifndef ALLOCATOR_H
#define ALLOCATOR_H

void *allocate_memory(size_t size);
struct block *request_memory_from_os(size_t size);
void free_memory(void *ptr);
void debug_print_blocks(void);

#endif // ALLOCATOR_H
