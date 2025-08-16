#ifndef MEM_ALLOC_H
#define MEM_ALLOC_H

#include <stddef.h>   // size_t

/**
 * Minimal heap allocator using sbrk().
 * Thread-safe via a global mutex.
 *
 * Public API mirrors malloc/calloc/realloc/free with different names to avoid clashes:
 *   - alloc_init() / alloc_shutdown()
 *   - alloc_mem(size)          -> malloc(size)
 *   - set_mem_zero(n, size)    -> calloc(n, size)
 *   - re_alloc(ptr, size)      -> realloc(ptr, size)
 *   - release(ptr)             -> free(ptr)
 *   - print_mem_list()         -> debug helper
 *
 * Notes:
 * - No block splitting/coalescing (for simplicity). Reuse is based on a simple free-list scan.
 * - Last-block trimming uses sbrk() to return memory to the OS if the freed block is at the break.
 */

#ifdef __cplusplus
extern "C" {
#endif

void  alloc_init(void);
void  alloc_shutdown(void);

void* alloc_mem(size_t size);                    // like malloc
void* set_mem_zero(size_t num, size_t nsize);    // like calloc
void* re_alloc(void* block, size_t size);        // like realloc
void  release(void* block);                      // like free

void  print_mem_list(void);                      // debug helper

#ifdef __cplusplus
}
#endif

#endif // MEM_ALLOC_H
