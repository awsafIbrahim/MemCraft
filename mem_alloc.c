#include "mem_alloc.h"
#include <unistd.h>    // sbrk
#include <string.h>    // memset, memcpy
#include <pthread.h>   // pthread_mutex_*
#include <stdio.h>     // printf
#include <stdint.h>    // intptr_t

/* ---- Internal structures ------------------------------------------------- */

/* 16-byte alignment for the block header */
typedef char ALIGN[16];

union header {
    struct {
        size_t size;           /* payload size in bytes (not including header) */
        unsigned is_free;      /* 1 = free, 0 = in use */
        union header* next;    /* next block in singly linked list */
    } mem;
    ALIGN _align;              /* force header alignment to 16 bytes */
};

typedef union header mem_header;

/* Global free-list (actually list of all blocks), protected by a mutex */
static mem_header* head = NULL;
static mem_header* tail = NULL;

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static int g_initialized = 0;

/* ---- Helpers -------------------------------------------------------------- */

/* Find a free block with size >= requested size (first-fit). */
static mem_header* find_free_mem(size_t size) {
    mem_header* curr = head;
    while (curr) {
        if (curr->mem.is_free && curr->mem.size >= size) {
            return curr;
        }
        curr = curr->mem.next;
    }
    return NULL;
}

/* Expand the heap using sbrk() and create a new block header. */
static mem_header* request_space(size_t total_size) {
    void* block = sbrk((intptr_t)total_size);
    if (block == (void*)-1) {
        return NULL;
    }
    return (mem_header*)block;
}

/* ---- Public API ----------------------------------------------------------- */

void alloc_init(void) {
    if (g_initialized) return;
    /* If you want explicit initialization of the mutex (beyond the static
       initializer), you could call pthread_mutex_init(&g_lock, NULL) here. */
    g_initialized = 1;
}

void alloc_shutdown(void) {
    /* Nothing to do in this simple design; we could walk the list and try to
       sbrk back to original if tail is at the break. Left as-is for clarity. */
    (void)0;
}

/* Equivalent to free(): mark a block free; if it’s the last block at program break,
   shrink the heap via sbrk(). */
void release(void* block) {
    if (!block) return;

    pthread_mutex_lock(&g_lock);

    mem_header* header = (mem_header*)block - 1;
    void* program_break = sbrk(0);

    /* If this block is at the end of the heap, return it to the OS. */
    if ((char*)block + header->mem.size == (char*)program_break) {
        if (head == tail) {
            head = tail = NULL;
        } else {
            /* Walk to the block just before tail */
            mem_header* temp = head;
            while (temp && temp->mem.next != tail) {
                temp = temp->mem.next;
            }
            if (temp) {
                temp->mem.next = NULL;
                tail = temp;
            }
        }
        /* Move the program break backwards by header + payload size */
        intptr_t dec = -(intptr_t)((sizeof(mem_header)) + header->mem.size);
        sbrk(dec);
        pthread_mutex_unlock(&g_lock);
        return;
    }

    /* Otherwise, just mark as free */
    header->mem.is_free = 1;

    pthread_mutex_unlock(&g_lock);
}

/* Equivalent to malloc(): allocate a block of at least `size` bytes. */
void* alloc_mem(size_t size) {
    if (!size) return NULL;

    pthread_mutex_lock(&g_lock);

    /* Try to reuse a free block */
    mem_header* header = find_free_mem(size);
    if (header) {
        header->mem.is_free = 0;    /* mark in-use */
        /* NOTE: we do not split large free blocks (simple design) */
        pthread_mutex_unlock(&g_lock);
        return (void*)(header + 1);
    }

    /* No suitable free block -> request new space from OS */
    size_t total_size = sizeof(mem_header) + size;
    header = request_space(total_size);
    if (!header) {
        pthread_mutex_unlock(&g_lock);
        return NULL;
    }

    header->mem.size   = size;
    header->mem.is_free= 0;
    header->mem.next   = NULL;

    if (!head) head = header;
    if (tail)  tail->mem.next = header;
    tail = header;

    pthread_mutex_unlock(&g_lock);
    return (void*)(header + 1);
}

/* Equivalent to calloc(): allocate n elements of size `nsize` and zero-initialize. */
void* set_mem_zero(size_t num, size_t nsize) {
    if (!num || !nsize) return NULL;

    /* Overflow check: num * nsize */
    size_t size = num * nsize;
    if (nsize != 0 && size / nsize != num) {
        return NULL; /* overflow */
    }

    void* block = alloc_mem(size);
    if (!block) return NULL;

    memset(block, 0, size);
    return block;
}

/* Equivalent to realloc(): resize a block. */
void* re_alloc(void* block, size_t size) {
    if (block == NULL) {
        return alloc_mem(size);
    }
    if (size == 0) {
        /* C realloc semantics vary; a common approach is to free and return NULL */
        release(block);
        return NULL;
    }

    mem_header* header = (mem_header*)block - 1;
    if (header->mem.size >= size) {
        /* The block is already large enough; keep it. */
        return block;
    }

    /* Need a larger block: allocate new, copy old, free old. */
    void* ret = alloc_mem(size);
    if (!ret) return NULL;

    memcpy(ret, block, header->mem.size);
    release(block);
    return ret;
}

/* Debug helper: print the block list. */
void print_mem_list(void) {
    pthread_mutex_lock(&g_lock);

    mem_header* curr = head;
    printf("head = %p, tail = %p\n", (void*)head, (void*)tail);
    while (curr) {
        printf("addr = %p, size = %zu, is_free = %u, next = %p\n",
               (void*)curr, curr->mem.size, curr->mem.is_free, (void*)curr->mem.next);
        curr = curr->mem.next;
    }

    pthread_mutex_unlock(&g_lock);
}
