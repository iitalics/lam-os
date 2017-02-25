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
    struct chunk_free* bin_next;
    struct chunk_free* bin_prev;
};
struct chunk_used {
    uptr width; // LSB is set to 1 to denote 'in-use'
    char data[0];
};
#define OVERHEAD   (2 * sizeof(uptr))

/* TODO: make the following architecture specific */
// how many different bins are there
#define BIN_COUNT       72
// how many bins are unsorted
#define BINS_UNSORTED   48
// size of first bin
#define FIRST_BIN_SIZE  8
// bin sizes
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
static inline uptr*       chunk_tail_width_ptr (void* chunk);
static void               init_empty_chunk (void* ptr, uptr width);
static struct chunk_used* remove_chunk_ix (struct chunk_free* cf, u16 ix);
static struct chunk_used* remove_chunk (struct chunk_free* cf);
static inline int         should_split (uptr found_width, uptr targ_width);
static int                is_first_chunk (void* chunk);
static int                is_last_chunk (void* chunk);

// allocate arbitrary chunk
void* k_alloc (uptr size)
{
    size = align_up(size);
    uptr targ_width = size + OVERHEAD;
    u16 ix = bin_size_to_ix(size);

    mem_debugf("alloc; size={du}, targ_width={du}, ix={du}\n",
               size, targ_width, (u32) ix);
    struct chunk_free* cf = NIL;
    for (; ix < BIN_COUNT; ix++) {
        cf = bins[ix];
        while (cf && cf->width < targ_width)
            cf = cf->bin_next;

        if (cf)
            break;
    }

    if (cf == NIL) {
        if (panic_on_oom)
            kernel_panicf("out of memory in k_alloc()");
        else
            return NIL;
    }

    mem_debugf("found free chunk; ix={du}\n", (u32) ix);

    uptr found_width = cf->width;
    struct chunk_used* cu = remove_chunk_ix(cf, ix);

    if (should_split(found_width, targ_width)) {
        uptr excess = found_width - targ_width;
        void* next = targ_width + (void*) cu;
        mem_debugf("  splitting; {du} + {du} = {du}\n",
                   targ_width, excess, found_width);
        cu->width = targ_width | 1;
        *chunk_tail_width_ptr(cu) = targ_width;
        init_empty_chunk(next, excess);
    }

    return cu->data;
}

// free allocated memory
void k_free (void* ptr)
{
    if (ptr == NIL)
        return;

    struct chunk_used* cu = (void*) ((uptr) ptr - sizeof(uptr));

    // NOTE: the following two checks could be omitted if we want
    //       to go balls deep and assume we'll never do anything wrong,
    //       but maybe a kernel panic is a better idea
    // check that the chunk ptr is properly aligned
    if ((uptr) cu != align_down((uptr) cu))
        kernel_panicf("free unaligned ptr: {p}\n", ptr);
    // use the second 'width' marker at the end as a sort of
    // checksum/magic number
    if (cu->width != (1 | *chunk_tail_width_ptr(cu)))
        kernel_panicf("double free: {p}\n", ptr);

    uptr width = cu->width - 1;
    mem_debugf("freeing chunk; width={du}\n", (u32) width);

    // TODO: heuristic to not always join with adjacent chunks because
    //       creating large free chunks may cause slow allocations

    void* final_chunk = cu;

    // join with left chunk if free
    if (!is_first_chunk(cu)) {
        u32 prev_chunk_width = ((uptr*) cu)[-1];
        struct chunk_free* prev_chunk = (void*) ((uptr) cu - prev_chunk_width);
        if ((prev_chunk->width & 1) == 0) {
            width += prev_chunk_width;
            final_chunk = prev_chunk;
            remove_chunk(prev_chunk);
            mem_debugf("join left {p}; width={du}\n",
                       prev_chunk, width);
        }
    }

    // join with right chunk if free
    if (!is_last_chunk(cu)) {
        struct chunk_free* next_chunk = (void*) ((uptr) cu + cu->width - 1);
        if ((next_chunk->width & 1) == 0) {
            width += next_chunk->width;
            remove_chunk(next_chunk);
            mem_debugf("join right {p}; width={du}\n",
                       next_chunk, width);
        }
    }

    init_empty_chunk(final_chunk, width);
}

static inline u16 bin_size_to_ix (uptr size)
{
    if (size < bin_size[BINS_UNSORTED]) {
        return (size - FIRST_BIN_SIZE) / KALLOC_ALIGN_TO;
    }
    // TODO: binary search?
    for (u16 i = BIN_COUNT - 1; ; i--) {
        if (bin_size[i] <= size)
            return i;
    }
}

static inline uptr* chunk_tail_width_ptr (void* chunk)
{
    struct chunk_free* cf = chunk;
    uptr width = cf->width & ~1;
    return &((uptr*) (width + (uptr) chunk))[-1];
}

static void init_empty_chunk (void* ptr, uptr width)
{
    struct chunk_free* cf = ptr;
    u16 ix = bin_size_to_ix(width - OVERHEAD);
    cf->width = width;
    *chunk_tail_width_ptr(ptr) = width;
    // TODO: insert, sorted.
    cf->bin_next = bins[ix];
    cf->bin_prev = NIL;
    bins[ix] = cf;
    mem_debugf("new empty chunk {p}; ix={du}\n",
               ptr, (u32) ix);
}

static struct chunk_used* remove_chunk_ix (struct chunk_free* cf, u16 ix)
{
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

static struct chunk_used* remove_chunk (struct chunk_free* cf)
{
    return remove_chunk_ix(cf, bin_size_to_ix(cf->width - OVERHEAD));
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

static int is_first_chunk (void* chunk)
{
    for (u32 i = 0; i < MAX_AVAIL_REGIONS; i++) {
        if (avail[i].in_use && avail[i].start == (uptr) chunk)
            return 1;
    }
    return 0;
}
static int is_last_chunk (void* chunk)
{
    u32 width = ((struct chunk_free*) chunk)->width & ~1;
    uptr end = width + (uptr) chunk;
    for (u32 i = 0; i < MAX_AVAIL_REGIONS; i++) {
        if (avail[i].in_use && avail[i].end == end)
            return 1;
    }
    return 0;
}

static void init_new_region (struct mem_reg* region, uptr start, uptr end)
{
    region->in_use = 1;
    // align end and start addresses
    region->start = start = align_up(start);
    region->end = end = align_down(end);
    // register a huge chunk
    // TODO: split into smaller chunk
    init_empty_chunk((void*) start, end - start);
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
