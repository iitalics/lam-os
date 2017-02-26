#pragma once
#include <misc/ints.h>

/* cell data structure,
   to represent any object in the vm */

struct vm_data;
typedef struct vm_data* vm_cell;

struct vm_data {
    unsigned int size : 28;
    unsigned int info : 4;

    union {
        char data[0];
        vm_cell fields[0];
    };
};

// data with bits only; no children
// e.g., strings, bigints, floats
#define VM_DATA_INFO_BITS_ONLY    0x1
// objects with messages & mutable state
#define VM_DATA_INFO_OBJECT       0x2


/* create new datum */

vm_cell vm_make_bits_only (u32 size);
vm_cell vm_make_fields (u32 nfields);
#define vm_make_int(k) (vm_cell) (1 | (((uptr) (k)) << 1))


/* predicates */

#define VM_IS_UNINIT(c)  ((c) == NIL)
#define VM_IS_FIXNUM(c)  (1 & (uptr) (c))


/* accessors */

#define VM_TO_FIXNUM(c)  (((uptr) (c)) >> 1)
