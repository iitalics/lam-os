#include "datum.h"
#include <misc/misc.h>
#include <memory/memory.h>

vm_cell vm_make_bits_only (u32 size)
{
    struct vm_data* dat = k_alloc(sizeof(struct vm_data) + size);
    dat->size = size;
    dat->info = VM_DATA_INFO_BITS_ONLY;
    return dat;
}

vm_cell vm_make_fields (u32 nfields)
{
    uptr size = nfields * sizeof(vm_cell);
    struct vm_data* dat = k_alloc(sizeof(struct vm_data) + size);
    dat->size = size;
    dat->info = VM_DATA_INFO_BITS_ONLY;
    mem_clear(dat->fields, size);
    return dat;
}
