#include <stdint.h>
#include <list.h>
#include <hash.h>
#include <stdbool.h>

#define VM_BIN      0           // binary 파일로 부터 데이터를 로드 
#define VM_FILE     1           // mapping된 파일로 부터 데이터를 로드 
#define VM_ANON     2           // swap영역으로 부터 데이터를 로드 

struct vm_entry{
    uint8_t type;               // VM_BIN, VM_FILE, VM_ANON
    void *vaddr;                // vm_entry가 관리하는 가상페이지 번호 
    bool writable;              // T : 해당 주소에 write 가능 
                                // F : 해당 주소에 wrtie 불가능 
    bool is_loaded;             // physical mem의 탑재 여부를 알려주는 flag
    struct file* file;          // 가상주소와 mapping된 파일 

    /* Memory Mapped File */
    struct list_elem mmap_elem; // mmap 리스트 element

    size_t offset;              // 읽어야 할 파일 offset
    size_t read_bytes;          // 가상페이지에 쓰여져 있는 데이터 크기 
    size_t zero_bytes;          // 0으로 채울 남은 페이지의 바이트 

    /* Swapping */
    size_t swap_slot;           // swap slot

    struct hash_elem elem;      // hash table element
};

void vm_init(struct hash *vm); // hash_init()으로 hash table 초기화 
bool insert_vme(struct hash *vm, struct vm_entry *vme); // vm_entry를 hash_table에 삽입
bool delete_vme(struct hash *vm, struct vm_entry *vme); // vm_entry를 hash_table에서 삭제 
struct vm_entry *find_vme(void *vaddr); // vm_entry 검색 후 반환 
void vm_destroy(struct hash *vm); // hash table의 bucket list와 vm_entry 제거 
