#include "memory.h"
#include <misc/misc.h>
#include <kernel/kernel.h>

// panic when out of memory?
int panic_on_oom = 1;


// chunks
// "cap" means amount of usuable memory in a chunk
// "size" means total size of the chunk (cap + overhead)
#define OVERHEAD        (2 * sizeof(u32))
#define MIN_CHUNK_SIZE  (OVERHEAD + 2 * sizeof(struct free_chunk*))
struct allo_chunk {
    u32 size_status; // LSB is set to denote 'in use'
    char contents[0];
};
struct free_chunk {
    u32 size;
    struct free_chunk* bin_prev;
    struct free_chunk* bin_next;
};

// pointer to the size marker at the end of this chunk
static u32* tail_size_ptr (struct free_chunk* fc);
// initialize a new chunk
static void init_new_chunk (void* addr, u32 size);
// unlink chunk from bins
static void unlink_chunk (struct free_chunk* fc, u8 bin);



// bins
#define BIN_COUNT        64
#define BINS_UNSORTED    32
#define ALIGN_TO         8
static const u32 bin_cap[BIN_COUNT] =
    {
        // UNSORTED
        8,   16,  24,  32,  40,  48,  56,  64,  72,  80,  88,  96,  104, 112, 120, 128,
        136, 144, 152, 160, 168, 176, 184, 192, 200, 208, 216, 224, 232, 240, 248, 256,
        // SORTED
        264, 280, 304, 336, 384, 448, 512, // 1 << 9
        768, 1024, // 1 << 10
        2048, // 1 << 11
        4096, // 1 << 12
        8192, // 1 << 13
        16384, // 1 << 14
        24576, 32768, // 1 << 15
        49152, 65536,  // 1 << 16
        98304, 131072, // 1 << 17
        196608, 262144, // 1 << 18
        393216, 524288, // 1 << 19
        1048576,// 1 << 20
    };
static struct free_chunk* bins[BIN_COUNT] = {0};

// get bin for a certain capacity
static inline u8 cap_to_bin (u32 size)
{
    if (size <= bin_cap[BINS_UNSORTED - 1])
        return size / ALIGN_TO - 1;
    else
        kernel_panicf("free_size_to_bin({du})", size);
}
// get bin for a certain size
static inline u8 size_to_bin (u32 size)
{
    return cap_to_bin(size - OVERHEAD);
}

// alignment functions
static inline u32 align_ceil (u32 x)
{
    if (x & (ALIGN_TO - 1))
        return (x | (ALIGN_TO - 1)) + 1;
    else
        return x;
}


// allocate some memory
void* alloc (u32 cap)
{
    cap = align_ceil(cap);
    u8 bin = cap_to_bin(cap);

    // search for a chunk at least this size
    struct free_chunk* fc;
    while ((fc = bins[bin]) == NIL) {
        bin++;
        if (bin == BIN_COUNT) {
            if (panic_on_oom)
                kernel_panicf("Ran out of memory in alloc()");
            else
                return NIL;
        }
    }

    // unlink chunk from bins
    unlink_chunk(fc, bin);

    // should we shrink it?
    if (bin < BINS_UNSORTED && fc->size >= (cap + OVERHEAD + MIN_CHUNK_SIZE)) {
        u32 new_size = cap + OVERHEAD;
        u32 excess = fc->size - new_size;
        // initialize the excess chunk
        init_new_chunk(new_size + (char*) fc, excess);
        // shrink this chunk size
        fc->size = *tail_size_ptr(fc) = new_size;
    }

    // set this chunk as 'in use'
    struct allo_chunk* ac = (void*) fc;
    ac->size_status |= 1;
    return ac->contents;
}

// free some memory
void free (void* ptr)
{
}

// pointer to the size marker at the end of this chunk
static u32* tail_size_ptr (struct free_chunk* fc)
{
    return ((u32*) (fc->size + (uptr) fc)) - 1;
}

// initialize a new chunk
static void init_new_chunk (void* addr, u32 size)
{
    struct free_chunk* fc = (void*) addr;
    fc->size = size;
    *tail_size_ptr(fc) = size;

    u32 bin = size_to_bin(size);
    fc->bin_next = bins[bin];
    fc->bin_prev = NIL;
    bins[bin]->bin_prev = fc;
    bins[bin] = fc;
}
// unlink chunk from bins
static void unlink_chunk (struct free_chunk* fc, u8 bin)
{
    if (fc->bin_prev == NIL)
        bins[bin] = fc->bin_next;
    else
        fc->bin_prev->bin_next = fc->bin_next;

    if (fc->bin_next != NIL)
        fc->bin_next->bin_prev = fc->bin_prev;
}



// allocate page aligned pages
void* alloc_pages (u32 count)
{
    if (panic_on_oom)
        kernel_panicf("Ran out of memory in alloc_pages()");
    else
        return NIL;
}

// free page aligned pages
void free_pages (void* ptr, u32 count)
{
}






// available memory regions
#define MAX_AVAIL_REGIONS   32

struct mem_reg {
    u8 in_use;
    u32 start;
    u32 end;
};

static struct mem_reg avail[32] = {{0}};


static void init_new_region (struct mem_reg* reg)
{
    u32 start = reg->start;
    if (start & 7)
        start = (start | 7) + 1;

    ((u32*) start)[0] = 0;
    ((u32*) start)[1] = 0;

    // TODO: preallocate some useful chunks
}

// notify memory management of a new region of available RAM
void memory_ram_avail (u32 start, u32 end)
{
    for (u32 i = 0; i < MAX_AVAIL_REGIONS; i++) {
        if (!avail[i].in_use) {
            avail[i].in_use = 1;
            avail[i].start = start;
            avail[i].end = end;
            init_new_region(&avail[i]);
            return;
        }
    }
    kernel_panicf("Too many memory regions, failed to add {p}.",
                  (void*) start);
}
