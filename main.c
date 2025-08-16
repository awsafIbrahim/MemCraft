#include "mem_alloc.h"
#include <stdio.h>

int main(void) {
    alloc_init();

    void* p1 = alloc_mem(32);     // 32 bytes
    void* p2 = alloc_mem(64);     // 64 bytes
    void* p3 = alloc_mem(128);    // 128 bytes
    void* p4 = set_mem_zero(15, 1); // 15 bytes zeroed

    printf("After initial allocations:\n");
    print_mem_list();

    /* Grow p2 to 100 bytes (new block + copy + free old) */
    p2 = re_alloc(p2, 100);

    /* Free some blocks */
    release(p3);
    release(p4);

    printf("\nAfter realloc/free:\n");
    print_mem_list();

    /* Optional: free the remaining blocks */
    release(p1);
    release(p2);

    printf("\nAfter releasing all:\n");
    print_mem_list();

    alloc_shutdown();
    return 0;
}
