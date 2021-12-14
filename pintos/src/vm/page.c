#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <hash.h>
#include "threads/vaddr.h"
#include "vm/page.h"
#include "threads/thread.h"

static unsigned vm_hash_func (const struct hash_elem *e, void *aux);
static bool vm_less_func (const struct hash_elem *a, const struct hash_elem *b, void *aux);

void vm_init(struct hash *vm){
    hash_init(vm, vm_hash_func, vm_less_func, NULL);
}

static unsigned
vm_hash_func(const struct hash_elem *e, void *aux){
    struct vm_entry *vm_e = hash_entry(e, struct vm_entry, elem);
    return hash_int(vm_e->vaddr);
}

static bool
vm_less_func (const struct hash_elem *a, const struct hash_elem *b, void *aux){
    struct vm_entry *vm_left = hash_entry(a, struct vm_entry, elem);
    struct vm_entry *vm_right = hash_entry(b, struct vm_entry, elem);

    return vm_left->vaddr < vm_right->vaddr;
}

bool 
insert_vme(struct hash *vm, struct vm_entry *vme){
    struct hash_elem *e = hash_insert(vm, &vme->elem);
    if (e != NULL){
        return true;
    } else {
        return false;
    }
}

bool 
delete_vme(struct hash *vm, struct vm_entry *vme){
    struct hash_elem *e = hash_delete(vm, &vme->elem);
    if (e != NULL){
        return true;
    } else {
        return false;
    }
}

// vaddr에 해당하는 vm_entry 반환 
struct vm_entry *
find_vme(void *vaddr){
    void *pg_num = pg_roung_down(vaddr);
    struct vm_entry *target = (struct vm_entry *)malloc(sizeof(struct vm_entry));
    target->vaddr = pg_num;
    struct hash_elem *e = hash_find(&thread_current()->vm, &target->elem);
    free(target);

    if(e != NULL){
        return hash_entry(e, struct vm_entry, elem);
    } else{
        return NULL;
    }
}

void 
vm_destroy(struct hash *vm){
    hash_destroy(vm, NULL);
}