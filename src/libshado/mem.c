#include "../include/libshado.h"
#include "../include/printf.h"

#define DEBUG_MEM 1
#define FLAG_MEM_ALLOCATED 1
#define FLAG_MEM_MAPPED 2

static const int PAGE_SIZE = 4096;
static const int RESIZE_THRESHOLD = 16;
static const int MMAP_THRESHOLD = 128*1024;
static const int HEADER_SIZE = 4;
static const int HEADER_FULL_SIZE = HEADER_SIZE * 2;

static uint32_t HEADER_SIZE_MASK = 0xFFFFFFF8;
static uint32_t HEADER_FLAGS_MASK = 7; /* 0111 */

/* 
 * CHUNK_HEADER {
 *     size: 29bits - 8bit aligned
 *     flags: 3bits
 * }
 * 
 * HEAP_CHUNK = {
 *     header: 4bytes
 *     data: $size bytes
 *     header: 4bytes
 * }
 * 
 * MMAP_CHUNK = {
 *     header: 4bytes
 *     data: $size bytes
 * }
 *
 * Header Flags:
 *     Bit 0 - 0 = free, 1 = allocated
 *     Bit 1 - 0 = heap, 1 = mmap
 *     Bit 2 - reserved
 *
 * Heap Layout:
 * |-H1-SIZE-FLAGS-| < heap start, first chunk header
 * |---------------|
 * |---------------|
 * |---------------|
 * |---------------|
 * |-H1-SIZE-FLAGS-| < first chunk header end
 * |-H2-SIZE-FLAGS-| < second chunk header begin
 * |---------------|
 * |---------------|
 * |---------------|
 * |---------------|
 * |-H2-SIZE-FLAGS-| < second chunk header end
 * |---------------| < heap end (current brk value)
 */

static uint8_t *p_heap_start = NULL;
static uint8_t *p_first_free = NULL;
static uint8_t *p_heap_end = NULL;

#define P_AS_U32(p) ((uint32_t*)p)

static inline uint32_t chunk_size (uint8_t *p) {
    return *P_AS_U32(p) & HEADER_SIZE_MASK;
}

static inline bool is_chunk_alloc (uint8_t *p) {
    return *P_AS_U32(p) & FLAG_MEM_ALLOCATED;
}

static inline bool is_chunk_free (uint8_t *p) {
    return !is_chunk_alloc(p);
}

static inline bool is_chunk_mapped (uint8_t *p) {
    return *P_AS_U32(p) & FLAG_MEM_MAPPED;
}

static inline uint8_t *next_chunk (uint8_t *p) {
    return (p + chunk_size(p) + HEADER_FULL_SIZE);
}

static inline uint8_t *chunk_set_size (uint8_t *p, size_t size) {
    *P_AS_U32(p) = size;
    *P_AS_U32(p+size+HEADER_SIZE) = size;
    return p;
}

static inline uint8_t *chunk_set_flags (uint8_t *p, uint8_t flags) {
    *P_AS_U32(p) |= flags;
    *P_AS_U32(p+chunk_size(p)+HEADER_SIZE) |= flags;
    return p;
}

static inline uint8_t *chunk_unset_flags (uint8_t *p, uint8_t flags) {
    *P_AS_U32(p) &= ~flags;
    *P_AS_U32(p+chunk_size(p)+HEADER_SIZE) &= ~flags;
    return p;
}

static inline size_t align_8 (size_t size) {
    uint32_t flags = size & HEADER_FLAGS_MASK;
    return flags ? size + 8 - flags : size;
}

static void init_heap () {
    p_heap_start = mem_brk(NULL);
    p_heap_end = p_heap_start + PAGE_SIZE;
    mem_brk(p_heap_end);

    /* First free entry is at start of heap */
    p_first_free = p_heap_start;

    /* Nothing alloced yet, so set size to full */
    chunk_set_size(p_first_free, PAGE_SIZE-HEADER_FULL_SIZE);
    if (DEBUG_MEM) {
        printf("MEM: Init Heap:\n");
        printf("MEM: \tStart: %lX\n", p_heap_start);
        printf("MEM: \tEnd: %lX\n", p_heap_end);
        printf("MEM: \tFirst Size: %lX\n", chunk_size(p_first_free));
        printf("MEM: \tHeap Size: %lX\n", p_heap_end - p_heap_start);
    }
}

void print_heap () {
    if (!DEBUG_MEM) return;
    if (!p_heap_start) init_heap();
    printf("MEM: Heap Info %lX to lX (%ld):\n", p_heap_start, p_heap_end, p_heap_end - p_heap_start);

    uint8_t *p = p_heap_start;
    while (p < p_heap_end) {
        printf("\tMEM: %lX Size: %d - %s\n", p, chunk_size(p), is_chunk_free(p) ? "FREE" : "ALLOC");
        p = next_chunk(p);
    }
}

static uint8_t *find_first_fit (size_t size) {
    for (uint8_t *p = p_first_free; p < p_heap_end; p = next_chunk(p))
        if (is_chunk_free(p) && chunk_size(p) >= size) return p;
    return NULL; /* need mem */
}

static uint8_t *alloc_new_mem (size_t in_size) {
    int num_pages = (in_size / PAGE_SIZE) + 1;
    if (in_size % PAGE_SIZE < HEADER_FULL_SIZE)
        /* enough space for buf, but not for buf + headers */
        num_pages++;

    size_t size = num_pages * PAGE_SIZE;
    char *prev_end = (char*)p_heap_end;
    p_heap_end = mem_brk(p_heap_end + size);
    return chunk_set_size((uint8_t*)prev_end, size-HEADER_FULL_SIZE);
}

void *malloc(size_t in_size) {
    if (!p_heap_start) init_heap();

    if (in_size >= MMAP_THRESHOLD) {
    /* not gonna use heap, use mmap instead */
        uint8_t *p = mem_mmap(NULL, in_size+HEADER_SIZE_MASK, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        chunk_set_size(p, in_size);
        chunk_set_flags(p, FLAG_MEM_MAPPED | FLAG_MEM_ALLOCATED);

        /* user's ptr starts just past header */
        return p + HEADER_SIZE;
    }

    /* not mmap, use heap instead */
    /* 8 bit align the size */
    size_t size = align_8(in_size);
    uint8_t *p = find_first_fit(size);
    if (!p) p = alloc_new_mem(size);

    size_t csize = chunk_size(p);
    size_t diff = csize - size;
    if (diff > RESIZE_THRESHOLD) {
        size_t next_size = diff - HEADER_FULL_SIZE;
        chunk_set_size(p, size);
        chunk_set_size(next_chunk(p), next_size);
    }

    chunk_set_flags(p, FLAG_MEM_ALLOCATED);
    return p + HEADER_SIZE;
}

void free(void *up) {
    uint8_t *p = ((uint8_t*)up) - HEADER_SIZE;
    if (is_chunk_mapped(p)) {
        /* wasn't on heap: unmap it */
        int ret = mem_munmap(p, chunk_size(p)+HEADER_SIZE);
        if (DEBUG_MEM) printf("MEM: free mapped, returned %d\n", ret);
    }

    chunk_unset_flags(p, FLAG_MEM_ALLOCATED);
    if (is_chunk_free(next_chunk(p))) {
        /* if next chunk free: merge them */
        size_t new_size = chunk_size(p) + chunk_size(next_chunk(p)) + HEADER_FULL_SIZE;
        chunk_set_size(p, new_size);
    }

    if (next_chunk(p) >= p_heap_end)
        /* on last chunk of heap, actually free memory and move brk back */
        p_heap_end = mem_brk(p);

    if (p < p_first_free)
        p_first_free = p;
}
