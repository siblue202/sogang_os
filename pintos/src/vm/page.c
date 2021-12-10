#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <hash.h>
#include "threads/vaddr.h"

static unsigned vm_hash_func (const struct hash_elem *e, void *aux);
static bool vm_less_func (const struct hash_elem *a, const struct hash_elem *b);

void vm_init(struct hash *vm){
    hash_init(vm, vm_hash_func, vm_less_func, NULL);
}

static unsigned
vm_hash_func(const struct hash_elem *e, void *aux){
    struct vm_entry *vm_e = hash_entry(e, struct vm_entry, elem);
    return hash_init(vm_e->vaddr);
}

static bool
vm_less_func (const struct hash_elem *a, const struct hash_elem *b){
    struct vm_entry *vm_left = hash_entry(a, struct vm_entry, elem);
    struct vm_entry *vm_right = hash_entry(b, struct vm_entry, elem);

    return vm_left->vaddr < vm_right->vaddr;
}

bool 
insert_vme(struct hash *vm, struct vm_entry *vme){
    struct hash_elem *e = hash_insert(vm, vme->elem);
    if (e != NULL){
        return true
    } else {
        return false;
    }
}

bool 
delete_vme(struct hash *vm, struct vm_entry *vme){
    struct hash_elem *e = hash_delete(vm, vme->elem);
    if (e != NULL){
        return true
    } else {
        return false;
    }
}

struct vm_entry *
find_vme(void *vaddr){
    void *pg_num = pg_roung_down(vaddr);
    strcut hash_elem *e = hash_find()
}

