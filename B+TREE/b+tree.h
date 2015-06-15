#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdbool.h>

#define MAXFIELD 18
#define ORDER 3

int order = ORDER;
int static no_of_entries;
bool verbose_output = false;

typedef struct bucket {
        unsigned long *key;
        void **b_pointer;
        bool is_leaf;
        struct bucket *parent;
        int key_counter;
        struct bucket *next;
} bucket;

typedef struct field {
        unsigned long value;
} field;

bucket * queue = NULL;

unsigned long rmalloc (int amount);

field * make_field (unsigned long key);

bucket * make_bucket ();

bucket * make_leaf ();

int div_point (int size);

bucket * search_leaf (bucket * root , unsigned long key , bool verbose);

field * key_exits (bucket * root , unsigned long key , bool verbose);

bucket * create_new_tree(unsigned long key , field * pointer);

bucket * insert_to_leaf (bucket * leaf, unsigned long key , field *
        pointer);
        
bucket * insert_to_leaf_after_split (bucket * root , unsigned long key ,
                bucket * leaf ,field * pointer);
                
bucket * make_new_root(unsigned long key , bucket * left ,
                bucket * right);
                
int get_left_postion(bucket * parent , bucket * left);

bucket * insert_to_node (bucket * root , bucket * p ,
                unsigned long key , int index , bucket * right);
                
bucket * insert_to_split_node (bucket * root , bucket * old_parent ,
                unsigned long key , int index , bucket * right);
                
bucket * insert_to_parent (bucket * root , unsigned long key , bucket *
                left , bucket * right);
                
bucket * master_insert (bucket * root , unsigned long key);