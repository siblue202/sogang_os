#include "list.h"
#include "hash.h"
#include "bitmap.h"
#include "debug.h"
#include "hex_dump.h"
#include "round.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct set_list name_list[10];
struct set_hash name_hash[10];
struct set_bitmap name_bitmap[10];


// searching function
typedef enum{
    CREATE=0, DUMPDATA, DELETE, QUIT,
    //list
    LIST_PUSH_BACK, LIST_PUSH_FRONT, LIST_FRONT, LIST_BACK, LIST_POP_BACK, LIST_POP_FRONT, LIST_INSERT_ORDERED,
    LIST_INSERT, LIST_EMPTY, LIST_SIZE, LIST_MAX, LIST_MIN, LIST_REMOVE, LIST_REVERSE, LIST_SHUFFLE, LIST_SORT, LIST_SPLICE, 
    LIST_SWAP, LIST_UNIQUE,
    //hash
    HASH_INSERT, HASH_APPLY, HASH_DELETE, HASH_EMPTY, HASH_SIZE,
    HASH_CLEAR, HASH_FIND, HASH_REPLACE,
    //bitmap
    BITMAP_MARK, BITMAP_ALL, BITMAP_ANY, BITMAP_CONTAINS, BITMAP_COUNT, BITMAP_DUMP, BITMAP_EXPAND, BITMAP_SET_ALL, BITMAP_FLIP, BITMAP_NONE,
    BITMAP_RESET, BITMAP_SCAN_AND_FLIP, BITMAP_SCAN, BITMAP_SET_MULTIPLE, BITMAP_SET, BITMAP_SIZE, BITMAP_TEST

} FUNCTION_NAME;

char * function_name_list[] = {
    "create", "dumpdata", "delete", "quit", 
    //list
    "list_push_back", "list_push_front", "list_front", "list_back", "list_pop_back", "list_pop_front", "list_insert_ordered",
    "list_insert", "list_empty", "list_size", "list_max", "list_min", "list_remove", "list_reverse", "list_shuffle", "list_sort", "list_splice",
    "list_swap", "list_unique",
    //hash
    "hash_insert", "hash_apply", "hash_delete", "hash_empty",
    "hash_size", "hash_clear", "hash_find", "hash_replace",
    //bitmap
    "bitmap_mark", "bitmap_all", "bitmap_any", "bitmap_contains", "bitmap_count", "bitmap_dump", "bitmap_expand", "bitmap_set_all", "bitmap_flip", "bitmap_none",
    "bitmap_reset", "bitmap_scan_and_flip", "bitmap_scan", "bitmap_set_multiple", "bitmap_set", "bitmap_size", "bitmap_test"
};

FUNCTION_NAME get_function_name(char* name){
    for(int i=0; i<sizeof(function_name_list)/sizeof(char *); i++){
        if(strcmp(name, function_name_list[i]) == 0){
            return i;
        }
    }
    return -1;
}

int searching_list(char * name){
    for(int i=0; i<10; i++){
        if(strcmp(name_list[i].name, name) == 0){
            return i;
        }
    }
    return -1;
}

int searching_hash(char * name){
    for(int i=0; i<10; i++){
        if(strcmp(name_hash[i].name, name) == 0){
            return i;
        }
    }
    return -1;
}

int searching_bitmap(char * name){
    for(int i=0; i<10; i++){
        if(strcmp(name_bitmap[i].name, name) == 0){
            return i;
        }
    }
    return -1;
}


int main(int argc, char *argv[]){
    char str[100];
    char *string_parameter[6];

    for(int i=0; i<10; i++){
        name_list[i].e_list = NULL;
        strcpy(name_list[i].name, "\0");
        name_hash[i].e_hash = NULL;
        strcpy(name_hash[i].name, "\0");
        name_bitmap[i].e_bitmap = NULL;
        strcpy(name_bitmap[i].name, "\0");
    }

    do{
        fgets(str, sizeof(str), stdin);
        str[strlen(str)-1] = '\0';
        // ClearLineFromReadBuffer();
        char *ptr = strtok(str, " ");
        int cnt =0;  

        while(ptr != NULL){
            string_parameter[cnt] = ptr;
            cnt ++;
            ptr = strtok(NULL, " ");
        } 

        switch (get_function_name(string_parameter[0])){

        case CREATE:{
            if (strcmp(string_parameter[1], "list") == 0) {
                int index = 0;
                while(name_list[index].e_list){
                    index++;
                }
                strcpy(name_list[index].name, string_parameter[2]);
                name_list[index].e_list = (struct list*) malloc(sizeof(struct list));
                list_init(name_list[index].e_list);

            } else if (strcmp(string_parameter[1], "hashtable") ==0 ){
                int index = 0;
                while(name_hash[index].e_hash){
                    index++;
                }
                strcpy(name_hash[index].name, string_parameter[2]);
                name_hash[index].e_hash = (struct hash*) malloc(sizeof(struct hash));
                hash_init(name_hash[index].e_hash, my_hash_hash_func, my_hash_less_func, NULL);    

            } else if (strcmp(string_parameter[1], "bitmap") == 0){
                int index = 0; 
                while(name_bitmap[index].e_bitmap){
                    index++;
                }
                struct bitmap * creation_bitmap = bitmap_create(atoi(string_parameter[3]));
                if(creation_bitmap != NULL){
                    name_bitmap[index].e_bitmap = creation_bitmap;
                    strcpy(name_bitmap[index].name, string_parameter[2]);
                }
            }
            break;
        }

        case DUMPDATA:{
            char subtext[3];
            strncpy(subtext, string_parameter[1], 3);
            subtext[2] = '\0';

            int index = 11;
            if (strcmp(subtext, "li\0") == 0){
                index = searching_list(string_parameter[1]);
            
                struct list_elem *e;
                for(e=list_begin(name_list[index].e_list) ; e!= list_end(name_list[index].e_list) ; e=e->next){
                    struct list_item *temp = list_entry(e, struct list_item, elem);
                    int temp_data = temp->data;
                    printf("%d ", temp_data);
                }
                printf("\n");

            } else if(strcmp(subtext, "ha\0") == 0){
                index = searching_hash(string_parameter[1]);

                struct hash* h = name_hash[index].e_hash;
                struct hash_iterator i;

                hash_first(&i, h);

                while(hash_next(&i)){
                    struct hash_item *temp = hash_entry(hash_cur(&i), struct hash_item, elem);
                    printf("%d ", temp->data);
                }
                printf("\n");

            } else if(strcmp(subtext, "bm\0") == 0){
                index = searching_bitmap(string_parameter[1]);

                struct bitmap * e = name_bitmap[index].e_bitmap;
                
                for(int i =0; i<bitmap_size(e); i++){
                    if(bitmap_test(name_bitmap[index].e_bitmap, i)){
                        printf("1");
                    }else{
                        printf("0");
                    }
                }
                printf("\n");
            }

            break;
        }

        case DELETE:{
            char subtext[5];
            strncpy(subtext, string_parameter[1], 5);
            subtext[4] = '\0';

            int index = 11;
            if (strcmp(subtext, "list\0") == 0){
                index = searching_list(string_parameter[1]);

                while (!list_empty (name_list[index].e_list)){
                    struct list_elem *e = list_pop_front(name_list[index].e_list);
                    free(e);
                }
                free(name_list[index].e_list);
                name_list[index].e_list = NULL;
                strcpy(name_list[index].name, "\0");

            } else if(strcmp(subtext, "hash\0") == 0){
                index = searching_hash(string_parameter[1]);
                
                struct hash* h = name_hash[index].e_hash;

                hash_clear(h, my_deallocate_hash_action_func);

                free(name_hash[index].e_hash);
                name_list[index].e_list = NULL;
                strcpy(name_list[index].name, "\0");

            } else if(strcmp(subtext, "bitm\0") == 0){
                index = searching_bitmap(string_parameter[1]);

                bitmap_destroy(name_bitmap[index].e_bitmap);
            }

            break;
        }
        case QUIT:{
            /* code */
            break;
        }
        case LIST_PUSH_BACK:{

            struct list_item *inserted = (struct list_item *)malloc(sizeof(struct list_item));
            inserted->data = atoi(string_parameter[2]);

            int index = searching_list(string_parameter[1]);

            list_push_back(name_list[index].e_list, &inserted->elem);

            break;
        }
        case LIST_PUSH_FRONT:{
            struct list_item *inserted = (struct list_item *)malloc(sizeof(struct list_item));
            inserted->data = atoi(string_parameter[2]);

            int index = searching_list(string_parameter[1]);

            list_push_front(name_list[index].e_list, &inserted->elem);

            break;
        }

        case LIST_FRONT:{
            int index = searching_list(string_parameter[1]);
            
            struct list_elem *e = list_front(name_list[index].e_list);

            struct list_item *temp = list_entry(e, struct list_item, elem);
            int temp_data = temp->data;
            printf("%d\n", temp_data);

            break;

        }

        case LIST_BACK:{
            int index = searching_list(string_parameter[1]);
            
            struct list_elem *e = list_back(name_list[index].e_list);

            struct list_item *temp = list_entry(e, struct list_item, elem);
            int temp_data = temp->data;
            printf("%d\n", temp_data);

            break;
        }

        case LIST_POP_BACK:{
            int index = searching_list(string_parameter[1]);

            struct list_elem *e = list_pop_back(name_list[index].e_list);
            free(e);

            break;
        }

        case LIST_POP_FRONT:{
            int index = searching_list(string_parameter[1]);

            struct list_elem *e = list_pop_front(name_list[index].e_list);
            free(e);

            break;
        }

        case LIST_INSERT_ORDERED:{
            struct list_item *inserted = (struct list_item *)malloc(sizeof(struct list_item));
            inserted->data = atoi(string_parameter[2]);

            int index = searching_list(string_parameter[1]);

            list_insert_ordered(name_list[index].e_list, &inserted->elem, my_list_less_func, NULL);

            break;
        }

        case LIST_INSERT:{
            struct list_item *inserted = (struct list_item *)malloc(sizeof(struct list_item));
            inserted->data = atoi(string_parameter[3]);

            int index = searching_list(string_parameter[1]);

            struct list_elem *e = list_begin(name_list[index].e_list);
            for(int i=0; i<atoi(string_parameter[2]); i++){
                e = list_next(e);
            }

            list_insert(e, &inserted->elem);

            break;
        }
        case LIST_EMPTY:{
            int index = searching_list(string_parameter[1]);

            if(list_empty(name_list[index].e_list)){
                printf("true\n");
            } else{
                printf("false\n");
            }

            break;
        }

        case LIST_SIZE:{
            int index = searching_list(string_parameter[1]);

            int cnt = list_size(name_list[index].e_list);
            printf("%d\n", cnt);

            break;
        }

        case LIST_MAX:{
            int index = searching_list(string_parameter[1]);

            struct list_elem * max = list_max(name_list[index].e_list, my_list_less_func, NULL);

            struct list_item *temp = list_entry(max, struct list_item, elem);
            int temp_data = temp->data;
            printf("%d\n", temp_data);

            break;
        }

        case LIST_MIN:{
            int index = searching_list(string_parameter[1]);

            struct list_elem * min = list_min(name_list[index].e_list, my_list_less_func, NULL);
            
            struct list_item *temp = list_entry(min, struct list_item, elem);
            int temp_data = temp->data;
            printf("%d\n", temp_data);
            break;
        }
        
        case LIST_REMOVE:{
            int index = searching_list(string_parameter[1]);

            struct list_elem *e = list_begin(name_list[index].e_list);
            for(int i=0; i<atoi(string_parameter[2]); i++){
                e = list_next(e);
            }

            list_remove(e);
            free(e);

            break;
        }

        case LIST_REVERSE:{
            int index = searching_list(string_parameter[1]);
            
            list_reverse(name_list[index].e_list);

            break;
        }

        case LIST_SHUFFLE:{
            int index = searching_list(string_parameter[1]);

            list_shuffle(name_list[index].e_list);
            break;
        }

        case LIST_SORT:{
            int index = searching_list(string_parameter[1]);

            list_sort(name_list[index].e_list, my_list_less_func, NULL);

            break;
        }

        case LIST_SPLICE:{

            int index_inserted = searching_list(string_parameter[1]);
            int index_spliced = searching_list(string_parameter[3]);

            struct list_elem *e_before = list_begin(name_list[index_inserted].e_list);
            for(int i=0; i<atoi(string_parameter[2]); i++){
                e_before = list_next(e_before);
            }

            struct list_elem *e_first = list_begin(name_list[index_spliced].e_list);
            struct list_elem *e_last = list_begin(name_list[index_spliced].e_list);

            for(int i=0; i<atoi(string_parameter[4]); i++){
                e_first = list_next(e_first);
            }

            for(int i=0; i<atoi(string_parameter[5]); i++){
                e_last = list_next(e_last);
            }

            list_splice(e_before, e_first, e_last);

            break;
        }

        case LIST_SWAP:{
            int index = searching_list(string_parameter[1]);

            struct list_elem *e_left = list_begin(name_list[index].e_list);
            struct list_elem *e_right = list_begin(name_list[index].e_list);

            for(int i=0; i<atoi(string_parameter[2]); i++){
                e_left = list_next(e_left);
            }

            for(int i=0; i<atoi(string_parameter[3]); i++){
                e_right = list_next(e_right);
            }

            list_swap(e_left, e_right);

            break;
        }

        case LIST_UNIQUE:{
            int index = searching_list(string_parameter[1]);
            int index_duplicate = searching_list(string_parameter[2]);

            list_unique(name_list[index].e_list, name_list[index_duplicate].e_list, my_list_less_func, NULL);
            
            break;
        }

        case HASH_INSERT:{
            int index = searching_hash(string_parameter[1]);

            struct hash_item * inserted_h = (struct hash_item*)malloc(sizeof(struct hash_item));
            inserted_h->data = atoi(string_parameter[2]);

            hash_insert(name_hash[index].e_hash, &inserted_h->elem);

            break;
        }

        case HASH_APPLY:{
            char subtext[7];
            strncpy(subtext, string_parameter[2], 7);
            subtext[6] = '\0';
            
            int index = searching_hash(string_parameter[1]);

            if(strcmp(subtext, "square\0") == 0){
                hash_apply(name_hash[index].e_hash, my_square_hash_action_func);
            } else if(strcmp(subtext, "triple\0") == 0){
                hash_apply(name_hash[index].e_hash, my_triple_hash_action_func);
            }
            break;
        }

        case HASH_DELETE:{
            int index = searching_hash(string_parameter[1]);

            struct hash_item *target_hash = (struct hash_item*)malloc(sizeof(struct hash_item));
            target_hash->data = atoi(string_parameter[2]);

            struct hash_elem * e_deallocate = hash_delete(name_hash[index].e_hash, &target_hash->elem);
            if(!e_deallocate){
                my_deallocate_hash_action_func(e_deallocate, NULL);
            }
            free(target_hash);

            break;
        }

        case HASH_EMPTY:{
            int index = searching_hash(string_parameter[1]);
            
            bool is_empty = hash_empty(name_hash[index].e_hash);
            printf("%s\n", is_empty ? "true" : "false"); 

            break;
        }

        case HASH_SIZE:{
            int index = searching_hash(string_parameter[1]);
            
            int size = hash_size(name_hash[index].e_hash);
            printf("%d\n", size);

            break;
        }

        case HASH_CLEAR:{
            int index = searching_hash(string_parameter[1]);    

            hash_clear(name_hash[index].e_hash, my_deallocate_hash_action_func);
            
            break;
        }

        case HASH_FIND:{
            int index = searching_hash(string_parameter[1]);

            struct hash_item * target_hash = (struct hash_item *)malloc(sizeof(struct hash_item));
            target_hash->data = atoi(string_parameter[2]);

            struct hash_elem *finded_hash = hash_find(name_hash[index].e_hash, &target_hash->elem);
            
            if(finded_hash != NULL){

                struct hash_item * temp = hash_entry(finded_hash, struct hash_item, elem);
                printf("%d\n", temp->data);
            }
            free(target_hash);

            break;
        }

        case HASH_REPLACE:{
            int index = searching_hash(string_parameter[1]);
            
            struct hash_item * inserted_hash = (struct hash_item *)malloc(sizeof(struct hash_item));
            inserted_hash->data = atoi(string_parameter[2]);

            struct hash_elem * old = hash_replace(name_hash[index].e_hash, &inserted_hash->elem);
            if(!old){
                my_deallocate_hash_action_func(old, NULL);
            }
            break;
        }

        case BITMAP_MARK :{
            int index = searching_bitmap(string_parameter[1]);

            bitmap_mark(name_bitmap[index].e_bitmap, atoi(string_parameter[2]));
            break;
        }

        case BITMAP_ALL :{
            int index = searching_bitmap(string_parameter[1]);

            bool is_all = bitmap_all(name_bitmap[index].e_bitmap, atoi(string_parameter[2]), atoi(string_parameter[3]));
            printf("%s\n", is_all ? "true" : "false"); 
          
            break;
        }

        case BITMAP_ANY :{
            int index = searching_bitmap(string_parameter[1]);

            bool is_any = bitmap_any(name_bitmap[index].e_bitmap, atoi(string_parameter[2]), atoi(string_parameter[3]));
            printf("%s\n", is_any ? "true" : "false"); 

            break;
        }

        case BITMAP_CONTAINS :{
            int index = searching_bitmap(string_parameter[1]);

            bool value;
            if(strcmp(string_parameter[4], "true") == 0){
                value = 1;
            } else if(strcmp(string_parameter[4], "false") == 0){
                value = 0;
            }

            bool is_contains = bitmap_contains(name_bitmap[index].e_bitmap, atoi(string_parameter[2]), atoi(string_parameter[3]), value);
            printf("%s\n", is_contains ? "true" : "false"); 
            
            break;
        }

        case BITMAP_COUNT :{
            int index = searching_bitmap(string_parameter[1]);

            bool value;
            if(strcmp(string_parameter[4], "true") == 0){
                value = 1;
            } else if(strcmp(string_parameter[4], "false") == 0){
                value = 0;
            }

            int cnt = bitmap_count(name_bitmap[index].e_bitmap, atoi(string_parameter[2]), atoi(string_parameter[3]), value);
            printf("%d\n", cnt); 

            break;
        }

        case BITMAP_DUMP :{
            int index = searching_bitmap(string_parameter[1]);

            bitmap_dump(name_bitmap[index].e_bitmap);

            break;
        }

        case BITMAP_EXPAND :{
            int index = searching_bitmap(string_parameter[1]);

            name_bitmap[index].e_bitmap = bitmap_expand(name_bitmap[index].e_bitmap, atoi(string_parameter[2]));

            break;
        }

        case BITMAP_SET_ALL :{
            int index = searching_bitmap(string_parameter[1]);

            bool value;
            if(strcmp(string_parameter[2], "true") == 0){
                value = 1;
            } else if(strcmp(string_parameter[2], "false") == 0){
                value = 0;
            }

            bitmap_set_all(name_bitmap[index].e_bitmap, value);

            break;
        }

        case BITMAP_FLIP :{
            int index = searching_bitmap(string_parameter[1]);

            bitmap_flip(name_bitmap[index].e_bitmap, atoi(string_parameter[2]));

            break;
        }

        case BITMAP_NONE :{
            int index = searching_bitmap(string_parameter[1]);

            bool is_none = bitmap_none(name_bitmap[index].e_bitmap, atoi(string_parameter[2]), atoi(string_parameter[3]));

            printf("%s\n", is_none ? "true" : "false"); 

            break;
        }

        case BITMAP_RESET :{
            int index = searching_bitmap(string_parameter[1]);

            bitmap_reset(name_bitmap[index].e_bitmap, atoi(string_parameter[2]));

            break;
        }

        case BITMAP_SCAN_AND_FLIP :{
            int index = searching_bitmap(string_parameter[1]);

            bool value;
            if(strcmp(string_parameter[4], "true") == 0){
                value = 1;
            } else if(strcmp(string_parameter[4], "false") == 0){
                value = 0;
            }

            size_t index_scan = bitmap_scan_and_flip(name_bitmap[index].e_bitmap, atoi(string_parameter[2]), atoi(string_parameter[3]), value);
            printf("%zu\n", index_scan);

            break;
        }

        case BITMAP_SCAN :{
            int index = searching_bitmap(string_parameter[1]);

            bool value;
            if(strcmp(string_parameter[4], "true") == 0){
                value = 1;
            } else if(strcmp(string_parameter[4], "false") == 0){
                value = 0;
            }

            size_t index_scan = bitmap_scan(name_bitmap[index].e_bitmap, atoi(string_parameter[2]), atoi(string_parameter[3]), value);
            printf("%zu\n", index_scan);

            break;
        }

        case BITMAP_SET_MULTIPLE :{
            int index = searching_bitmap(string_parameter[1]);

            bool value;
            if(strcmp(string_parameter[4], "true") == 0){
                value = 1;
            } else if(strcmp(string_parameter[4], "false") == 0){
                value = 0;
            }

            bitmap_set_multiple(name_bitmap[index].e_bitmap, atoi(string_parameter[2]), atoi(string_parameter[3]), value);
    
            break;
        }

        case BITMAP_SET :{
            int index = searching_bitmap(string_parameter[1]);

            bool value;
            if(strcmp(string_parameter[3], "true") == 0){
                value = 1;
            } else if(strcmp(string_parameter[3], "false") == 0){
                value = 0;
            }

            bitmap_set(name_bitmap[index].e_bitmap, atoi(string_parameter[2]), value);

            break;
        }

        case BITMAP_SIZE :{
            int index = searching_bitmap(string_parameter[1]);

            int size = bitmap_size(name_bitmap[index].e_bitmap);
            printf("%d\n", size);

            break;
        }

        case BITMAP_TEST :{
            int index = searching_bitmap(string_parameter[1]);

            bool is_test = bitmap_test(name_bitmap[index].e_bitmap, atoi(string_parameter[2]));
            printf("%s\n", is_test ? "true" : "false"); 

            break;
        }
 
        default:
            break;
            
        }

    } while(strcmp(string_parameter[0], "quit") != 0);
   

    return 0;
}