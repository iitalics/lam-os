#include "memory.h"
#include <misc/misc.h>
#include <kernel/kernel.h>

//#define MEMORY_DEBUG_DUMP

/******* misc. *********/

// panic when out of memory?
int panic_on_oom = 1;

// debug using writef
#ifdef MEMORY_DEBUG_DUMP
# define mem_debugf writef
#else
# define mem_debugf(...)
#endif

// ceiling / floor alignment
static inline u32 align_up (u32 k)
{
    if (k & (KALLOC_ALIGN_TO - 1))
        return 1 + (k | (KALLOC_ALIGN_TO - 1));
    else
        return k;
}
static inline u32 align_down (u32 k)
{
    return k & ~(KALLOC_ALIGN_TO - 1);
}




/******* bins *********/

struct chunk_free {
    uptr width;
    void* in_mem_prev;
    struct chunk_free* bin_next;
    struct chunk_free* bin_prev;
};
struct chunk_used {
    uptr width;
    void* in_mem_prev;
    char data[0];
};

// chunk flags
#define CF_FREE       0
#define CF_USED       1
#define CF_BOUNDARY   3
#define CF_OF(c)      ((c)->width & 3)
#define CF_SET(c,v)   ((c)->width = ((c)->width & ~3) | (v))

#define OVERHEAD   (sizeof(struct chunk_used))
#define BIN_COUNT          72
#define BIN_COUNT_UNSORT   48
#define FIRST_BIN_SIZE     8
static const uptr bin_size[BIN_COUNT] = {
    // 48 unsorted bins
    FIRST_BIN_SIZE,
    16,  24,  32,  40,  48,  56,  64,  72,  80,  88,  96,  104, 112, 120, 128,
    136, 144, 152, 160, 168, 176, 184, 192, 200, 208, 216, 224, 232, 240, 248, 256,
    264, 272, 280, 288, 296, 304, 312, 320, 328, 336, 344, 352, 360, 368, 376, 384,
    // 24 sorted bins
    392,   512,    768,    1024,   1536,   2048,   3072,   4096,
    6144,  8192,   12288,  16384,  24576,  32768,  49152,  65536,
    98304, 131072, 196608, 262144, 393216, 524288, 786432, 1048576,
};

static struct chunk_free* bins[BIN_COUNT] = {0};

static inline u16         bin_size_to_ix (uptr size);
static inline int         should_split (uptr outer_width, uptr inner_width);
static struct chunk_used* claim_chunk (struct chunk_free* cf);
static void               init_free_chunk (void* ptr, void* prev, uptr width);

// allocate arbitrary chunk
void* k_alloc (uptr size)
{
    size = align_up(size);
    uptr targ_width = size + OVERHEAD;
    u16 ix = bin_size_to_ix(size);

    mem_debugf("alloc; size={du}, targ_width={du}, ix={du}\n",
               size, targ_width, (u32) ix);

    struct chunk_free* c_free = NIL;
    for (; ix < BIN_COUNT; ix++) {
        c_free = bins[ix];
        while (c_free && c_free->width < targ_width)
            c_free = c_free->bin_next;

        if (c_free)
            break;
    }

    if (c_free == NIL) {
        // TODO: ask for memory
        if (panic_on_oom)
            kernel_panicf("out of memory in k_alloc()");
        else
            return NIL;
    }

    mem_debugf("found free chunk {p}; ix={du}\n", c_free, (u32) ix);

    // claim the chunk so that it is no longer in the free bins
    uptr found_width = c_free->width;
    struct chunk_used* c_used = claim_chunk(c_free);

    // split it if necessary
    if (should_split(found_width, targ_width)) {
        uptr excess = found_width - targ_width;
        mem_debugf("  splitting; {du} + {du} = {du}\n",
                   targ_width, excess, found_width);
        init_free_chunk(targ_width + (void*) c_used, c_used, excess);
        c_used->width = targ_width;
        CF_SET(c_used, CF_USED);
    }

    return c_used->data;
}

// free allocated memory
void k_free (void* ptr)
{
    if (ptr == NIL)
        return;

    struct chunk_used* c_used = (void*) ((uptr) ptr - OVERHEAD);

    // NOTE: the following two checks could be omitted if we want
    //       to go balls deep and assume we'll never do anything wrong,
    //       but maybe a kernel panic is a better idea
    // check that the chunk ptr is properly aligned, and that it is
    // marked as free.
    if ((uptr) c_used != align_down((uptr) c_used))
        kernel_panicf("free unaligned ptr: {p}\n", ptr);
    if (CF_OF(c_used) != CF_USED)
        kernel_panicf("corrupt / double free: {p}\n", ptr);

    uptr width = c_used->width & ~3;
    mem_debugf("freeing chunk {p}; width={du}\n", c_used, (u32) width);

    struct chunk_free* prev_chunk = c_used->in_mem_prev;
    struct chunk_free* next_chunk = width + (void*) c_used;

    // TODO: heuristic to not always join with adjacent chunks because
    //       creating large free chunks may cause slow allocations

    void* final_chunk = c_used;

    // join with left chunk if free
    if (CF_OF(prev_chunk) == CF_FREE) {
        width += prev_chunk->width;
        final_chunk = claim_chunk(prev_chunk);
        mem_debugf("join left {p}; width={du}\n",
                   prev_chunk, width);

        prev_chunk = prev_chunk->in_mem_prev;
    }

    // join with right chunk if free
    if (CF_OF(next_chunk) == CF_FREE) {
        width += next_chunk->width;
        claim_chunk(next_chunk);
        mem_debugf("join right {p}; width={du}\n",
                   next_chunk, width);
    }

    init_free_chunk(final_chunk, prev_chunk, width);
}

static inline u16 bin_size_to_ix (uptr size)
{
    if (size < bin_size[BIN_COUNT_UNSORT]) {
        return (size - FIRST_BIN_SIZE) / KALLOC_ALIGN_TO;
    }
    // TODO: binary search?
    for (u16 i = BIN_COUNT - 1; ; i--) {
        if (bin_size[i] <= size)
            return i;
    }
}

static void init_free_chunk (void* ptr, void* prev, uptr width)
{
    struct chunk_free* c_free = ptr;
    c_free->width = width;
    c_free->in_mem_prev = prev;

    // TODO: insert, sorted.
    u16 ix = bin_size_to_ix(width - OVERHEAD);
    c_free->bin_next = bins[ix];
    c_free->bin_prev = NIL;
    bins[ix] = c_free;
    mem_debugf("new empty chunk {p}; ix={du}, prev {p}\n",
               ptr, (u32) ix, prev);
}

static struct chunk_used* claim_chunk (struct chunk_free* cf)
{
    u16 ix = bin_size_to_ix(cf->width - OVERHEAD);
    if (cf->bin_prev == NIL) {
        bins[ix] = cf->bin_next;
        if (cf->bin_next != NIL)
            cf->bin_next->bin_prev = NIL;
    }
    else {
        cf->bin_prev->bin_next = cf->bin_next;
        if (cf->bin_next != NIL)
            cf->bin_next->bin_prev = cf->bin_prev;
    }
    cf->width |= 1;
    return (struct chunk_used*) cf;
}

static inline int should_split (uptr found_width, uptr targ_width)
{
    return found_width >= targ_width + FIRST_BIN_SIZE + OVERHEAD;
}









/******* memory regions *********/

struct mem_reg {
    // data about this region
    u8 in_use;
    uptr start;
    uptr end;
};

// available memory regions
#define MAX_AVAIL_REGIONS   16
static struct mem_reg avail[MAX_AVAIL_REGIONS] = {{0}};

static void init_new_region (struct mem_reg* region, uptr start, uptr end)
{
    region->in_use = 1;
    // align end and start addresses
    region->start = start = align_up(start);
    region->end = end = align_down(end);
    // TODO: use a few pages at a time, when asked.
    // create boundary chunks
    struct chunk_free* bc_left = (void*) start;
    struct chunk_free* bc_right = (void*) ((uptr) end - sizeof(struct chunk_free));
    bc_left->width =
        bc_right->width = sizeof(struct chunk_free) & CF_BOUNDARY;
    // init middle chunk
    init_free_chunk(bc_left + 1, bc_left, (uptr) bc_right - (uptr) (bc_left + 1));
}

// notify memory management of a new region of available RAM
void memory_ram_avail (uptr start, uptr end)
{
    for (u32 i = 0; i < MAX_AVAIL_REGIONS; i++) {
        if (!avail[i].in_use) {
            init_new_region(&avail[i], start, end);
            return;
        }
    }
    kernel_panicf("Too many memory regions, failed to add {p}.",
                  (void*) start);
}
