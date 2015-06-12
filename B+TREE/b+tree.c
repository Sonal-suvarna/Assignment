#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>
#include <stdlib.h>
#define MAXFIELD 18

#ifndef ORDER
#define ORDER 3
#endif

int order=ORDER;
static int no_of_entries;

typedef struct field {
        unsigned long value;
}field;

typedef struct bucket {
        unsigned long *key;
        void **bpointer;
        struct bucket *parent;
        struct bucket *next;
        int key_counter;
        bool is_leaf;
}bucket;

bool verbose_output = false;

unsigned long rmalloc (int amount);
bucket * find_leaf( bucket * root, unsigned long key, bool verbose );
field * key_exists( bucket * root, unsigned long key, bool verbose);
int cut( int length );
field * make_field(unsigned long key);
bucket * make_bucket( void );
bucket * make_leaf( void );
int get_left_index(bucket * parent, bucket * left);
bucket * insert_into_leaf( bucket * leaf, unsigned long key, field * pointer );
bucket * insert_into_leaf_after_splitting(bucket * root, bucket * leaf, unsigned long key, field * pointer);
bucket * insert_into_node(bucket * root, bucket * n, int left_index, unsigned long key, bucket * right);
bucket * insert_into_node_after_splitting(bucket * root, bucket * old_node, int left_index, unsigned long key, bucket * right);
bucket * insert_into_parent(bucket * root, bucket * left, unsigned long key, bucket * right);
bucket * insert_into_new_root(bucket * left, unsigned long key, bucket * right);
bucket * insert( bucket * root, int key );


unsigned long rmalloc (int amount) {

        unsigned long * rptr;
        rptr = (void *) malloc ( amount );
        return (unsigned long)rptr;
}

bucket * find_leaf( bucket * root, unsigned long key, bool verbose ) {
        int i = 0;
        bucket * c = root;
        if (c == NULL) {
                if (verbose) 
                        printf("Tree is empty\n");
                return c;
        }
        while (!c->is_leaf) {
                if (verbose) {
                        printf("[");
                        for (i = 0; i < c->key_counter - 1; i++)
                                printf("%d ", c->key[i]);
                        printf("%d] ", c->key[i]);
                }
                i = 0;
                while (i < c->key_counter) {
                        if (key >= c->key[i]) i++;
                        else break;
                }
                if (verbose)
                        printf("%d ->\n", i);
                c = (bucket *)c->bpointer[i];
        }
        if (verbose) {
                printf("Leaf [");
                for (i = 0; i < c->key_counter - 1; i++)
                        printf("%d ", c->key[i]);
                printf("%d] ->\n", c->key[i]);
        }
        return c;
}


field * key_exists( bucket * root, unsigned long key, bool verbose) {

        int i = 0;
        bucket * c = find_leaf( root, key, verbose);
        if (c == NULL) return NULL;
        for (i = 0; i < c->key_counter; i++)
                if (c->key[i] == key) break;
        if (i == c->key_counter) 
                return NULL;
        else
                return (field *)c->bpointer[i];

}

int cut( int length ) {

        if (length % 2 == 0)
                return length/2;
        else
                return length/2 + 1;

}

field * make_field(unsigned long key) {

        field * field_ptr;
        field_ptr = (field *) malloc (sizeof(field));
        if(field_ptr == NULL)
        {
                printf("Couldnt allocate memory\n");
                exit(0);
        }
        else {
                field_ptr->value = key;
        }
        return field_ptr;

}

bucket * make_bucket( void ) {

        bucket * bucket_ptr;
        bucket_ptr = (bucket *) malloc(sizeof(bucket));
        if (bucket_ptr == NULL) {
                printf("Couldnt create memory\n");
                exit(0);
        }
        bucket_ptr->key = (unsigned long *) malloc( (order - 1) * sizeof(unsigned long) );
        if (bucket_ptr->key == NULL) {
                printf("Couldnt create memory\n");
                exit(0);
        }
        bucket_ptr->bpointer = malloc( order * sizeof(void *) );
        if (bucket_ptr->bpointer == NULL) {
                printf("Couldnt create memory\n");
                exit(0);
        }
        bucket_ptr->is_leaf = false;
        bucket_ptr->key_counter = 0;
        bucket_ptr->parent = NULL;
        bucket_ptr->next = NULL;
        return bucket_ptr;

}

bucket * make_leaf( void ) {

        bucket * leaf = make_bucket();
        leaf->is_leaf = true;
        return leaf;

}

int get_left_index(bucket * parent, bucket * left) {

        int left_index = 0;
        while (left_index <= parent->key_counter && 
                        parent->bpointer[left_index] != left)
                left_index++;
        return left_index;
}

bucket * insert_into_leaf( bucket * leaf, unsigned long key, field * pointer ) {

        int i, insertion_point;
        insertion_point = 0;
        while (insertion_point < leaf->key_counter && leaf->key[insertion_point] < key)
                insertion_point++;

        for (i = leaf->key_counter; i > insertion_point; i--) {
                leaf->key[i] = leaf->key[i - 1];
                leaf->bpointer[i] = leaf->bpointer[i - 1];
        }
        leaf->key[insertion_point] = key;
        leaf->bpointer[insertion_point] = pointer;
        leaf->key_counter++;
        return leaf;
}

bucket * insert_into_leaf_after_splitting(bucket * root, bucket * leaf, unsigned long key, field * pointer) {

        bucket * new_leaf;
        unsigned long * temp_keys;
        void ** temp_pointers;
        int insertion_index, split, new_key, i, j;
        new_leaf = make_leaf();
        temp_keys = (unsigned long *)malloc( order * sizeof(unsigned long) );
        if (temp_keys == NULL) {
                printf("No space\n");
                exit(0);
        }
        temp_pointers = malloc( order * sizeof(void *) );
        if (temp_pointers == NULL) {
                printf("No space\n");
                exit(0);
        }
        insertion_index = 0;
        while (insertion_index < order - 1 && leaf->key[insertion_index] < key)
                insertion_index++;
        for (i = 0, j = 0; i < leaf->key_counter; i++, j++) {
                if (j == insertion_index) j++;
                temp_keys[j] = leaf->key[i];
                temp_pointers[j] = leaf->bpointer[i];
        }
        temp_keys[insertion_index] = key;
        temp_pointers[insertion_index] = pointer;
        leaf->key_counter = 0;
        split = cut(order - 1);
        for (i = 0; i < split; i++) {
                leaf->bpointer[i] = temp_pointers[i];
                leaf->key[i] = temp_keys[i];
                leaf->key_counter++;
        }
        for (i = split, j = 0; i < order; i++, j++) {
                new_leaf->bpointer[j] = temp_pointers[i];
                new_leaf->key[j] = temp_keys[i];
                new_leaf->key_counter++;
        }
        free(temp_pointers);
        free(temp_keys);
        new_leaf->bpointer[order - 1] = leaf->bpointer[order - 1];
        leaf->bpointer[order - 1] = new_leaf;

        for (i = leaf->key_counter; i < order - 1; i++)
                leaf->bpointer[i] = NULL;
        for (i = new_leaf->key_counter; i < order - 1; i++)
                new_leaf->bpointer[i] = NULL;
        new_leaf->parent = leaf->parent;
        new_key = new_leaf->key[0];
        return insert_into_parent(root, leaf, new_key, new_leaf);
        //return tempo;

}

bucket * insert_into_node(bucket * root, bucket * n, 
                int left_index, unsigned long key, bucket * right) {
        int i;
        for (i = n->key_counter; i > left_index; i--) {
                n->bpointer[i + 1] = n->bpointer[i];
                n->key[i] = n->key[i - 1];
        }
        n->bpointer[left_index + 1] = right;
        n->key[left_index] = key;
        n->key_counter++;
        return root;
}

bucket * insert_into_node_after_splitting(bucket * root, bucket * old_node, int left_index, 
                unsigned long key, bucket * right) {

        int i, j, split, k_prime;
        bucket * new_node, * child;
        unsigned long * temp_keys;
        bucket ** temp_pointers;

        /* First create a temporary set of keys and pointers
        * to hold everything in order, including
        * the new key and pointer, inserted in their
        * correct places. 
        * Then create a new node and copy half of the 
        * keys and pointers to the old node and
        * the other half to the new.
        */

        temp_pointers = malloc( (order + 1) * sizeof(bucket *) );
        if (temp_pointers == NULL) {
        printf("No space\n");
        exit(0);
        }
        temp_keys = malloc( order * sizeof(unsigned long) );
        if (temp_keys == NULL) {
        printf("No space\n");
        exit(0);
        }
        for (i = 0, j = 0; i < old_node->key_counter + 1; i++, j++) {
                if (j == left_index + 1) j++;
                temp_pointers[j] = old_node->bpointer[i];
        }

        for (i = 0, j = 0; i < old_node->key_counter; i++, j++) {
                if (j == left_index) j++;
                temp_keys[j] = old_node->key[i];
        }

        temp_pointers[left_index + 1] = right;
        temp_keys[left_index] = key;

        /* Create the new node and copy
        * half the keys and pointers to the
        * old and half to the new.
        */  
        split = cut(order);
        new_node = make_bucket();
        old_node->key_counter = 0;
        for (i = 0; i < split - 1; i++) {
                old_node->bpointer[i] = temp_pointers[i];
                old_node->key[i] = temp_keys[i];
                old_node->key_counter++;
        }
        old_node->bpointer[i] = temp_pointers[i];
        k_prime = temp_keys[split - 1];
        for (++i, j = 0; i < order; i++, j++) {
                new_node->bpointer[j] = temp_pointers[i];
                new_node->key[j] = temp_keys[i];
                new_node->key_counter++;
        }
        new_node->bpointer[j] = temp_pointers[i];
        free(temp_pointers);
        free(temp_keys);
        new_node->parent = old_node->parent;
        for (i = 0; i <= new_node->key_counter; i++) {
                child = new_node->bpointer[i];
                child->parent = new_node;
        }

        /* Insert a new key into the parent of the two
        * nodes resulting from the split, with
        * the old node to the left and the new to the right.
        */

        return insert_into_parent(root, old_node, k_prime, new_node);
}

bucket * insert_into_parent(bucket * root, bucket * left, unsigned long key, bucket * right) {

        int left_index;
        bucket * parent;
        parent = left->parent;
        /* Case: new root. */
        if (parent == NULL)
                return insert_into_new_root(left, key, right);
        /* Case: leaf or node. (Remainder of
        * function body.)  
        */
        /* Find the parent's pointer to the left 
        * node.
        */
        left_index = get_left_index(parent, left);
        /* Simple case: the new key fits into the node. 
        */
        if (parent->key_counter < order - 1)
                return insert_into_node(root, parent, left_index, key, right);
        /* Harder case:  split a node in order 
        * to preserve the B+ tree properties.
        */
        return insert_into_node_after_splitting(root, parent, left_index, key, right);

}

bucket * insert_into_new_root(bucket * left, unsigned long key, bucket * right) {

        bucket * root = make_bucket();
        root->key[0] = key;
        root->bpointer[0] = left;
        root->bpointer[1] = right;
        root->key_counter++;
        root->parent = NULL;
        left->parent = root;
        right->parent = root;
        return root;
}

bucket * start_new_tree(unsigned long key, field * pointer) {

        bucket * root = make_leaf();
        root->key[0] = key;
        root->bpointer[0] = pointer;
        root->bpointer[order - 1] = NULL;
        root->parent = NULL;
        root->key_counter++;
        return root;

}

bucket * insert( bucket * root, int key ) {

        field * pointer;
        bucket * leaf;
        if (key_exists(root, key, false) != NULL)
        return root;
        pointer = make_field(key);
        if (root == NULL)
        return start_new_tree(key, pointer);
        leaf = find_leaf(root, key, false);
        if (leaf->key_counter < order - 1) {
                leaf = insert_into_leaf(leaf, key, pointer);
                return root;
        }
        return insert_into_leaf_after_splitting(root, leaf, key, pointer);

}

int main()
{

        unsigned long key;
        int choice,amount;
        bucket *root;
        root = NULL;
        verbose_output = false;
        unsigned long ptr;
        while(1) {
                        if(root == NULL)
                        {
                                printf("\nTree is initially empty\n");
                        }
        printf("\n1.INSERT\n2.DELETION\n3.EXIT\n");
        printf("Enter the operation you like to perform\t: ");
        if(scanf("%d",&choice)!=3) {
                switch(choice) {
                case 1:
                        if(no_of_entries<MAXFIELD) {
                        printf("Enter the amount of data you want to allocate\t: ");
                        scanf("%d",&amount);
                        ptr = rmalloc (amount);
                        printf("The memory address is\t:  %lu\n",ptr);
                        }
                        else {
                                printf("Tree is already full\n");
                                exit(0);
                        }
                        root = insert(root, ptr);
                        if(root != NULL)
                        no_of_entries++;
                        printf("\nInsertion of %d element is successful\n",no_of_entries);
                        break;
                case 2: 
                        printf("\nNot yet implemented\n");
                        break;
                case 3:
                        printf("\nYou have chosen to terminate\n");
                        exit(0);
                        break;
                }
        }
        }

}