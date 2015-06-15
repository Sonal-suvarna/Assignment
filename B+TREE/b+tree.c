#include "b+tree.h"

unsigned long rmalloc (int amount) {

        unsigned long * rptr;
        rptr = (void *) malloc ( amount );
        return (unsigned long)rptr;
}

field * make_field (unsigned long key) {

        field * field_pointer;
        field_pointer = (field *) malloc (sizeof(field));
        if(field_pointer == NULL) {
                printf("Memory not loiallocated\n");
                exit(0);
        }
        else {
                field_pointer->value = key;
        }
        return field_pointer;

}

bucket * make_bucket () {

        bucket * bucket_pointer;
        bucket_pointer = (bucket *) malloc (sizeof(bucket));
        if(bucket_pointer == NULL) {
                printf("Memory not allocated\n");
                exit(0);
        }
        bucket_pointer->key = (unsigned long * ) malloc ( (order -1) *
                sizeof(unsigned long));
        if(bucket_pointer->key == NULL) {
                printf("Memory not allocated\n");
                exit(0);
        }
        bucket_pointer->b_pointer = malloc (order * sizeof(void *));
        if(bucket_pointer->b_pointer == NULL) {
                printf("Memory not allocated\n");
                exit(0);
        }
        bucket_pointer->is_leaf = NULL;
        bucket_pointer->parent = NULL;
        bucket_pointer->key_counter = 0;
        return bucket_pointer;

}

bucket * make_leaf() {

        bucket * leaf = make_bucket();
        leaf->is_leaf = true;
        return leaf;
}

int div_point (int size) {
        if(size%2 == 0) {
                return size/2;
        }
                else {
                        return (size/2 + 1);
                }
}
bucket * search_leaf (bucket * root , unsigned long key , bool verbose) {

        int i = 0;
        bucket * p = root;
        if(p == NULL) {
                if(verbose)
                printf("The tree dosnt exists\n");
                return p;
        }
        while(!p->is_leaf) {
                while(i<p->key_counter) {
                        if(key >= p->key[i]) i++;
                        else break;
                }
                p = (bucket *)p->b_pointer[i];
        }
        return p;
}

field * key_exists (bucket * root , unsigned long key , bool verbose) {

        int i;
        bucket * ptr = search_leaf(root , key , false);
        if(ptr == NULL) {
                return NULL;
        }
        for(i=0 ; i<ptr->key_counter ; i++)
                if(ptr->key[i] == key) break;
        if(i == ptr->key_counter)
                return NULL;
        else
                return (field *)ptr->b_pointer[i];

}

bucket * create_new_tree(unsigned long key , field * pointer) {

        bucket * root = make_leaf();
        root->key[0] = key;
        root->b_pointer[0] = pointer;
        root->parent = NULL;
        root->b_pointer[order - 1] = NULL;
        root->key_counter++;
        return root;

}

bucket * insert_to_leaf (bucket * leaf, unsigned long key , field *
        pointer) {

                int insertion_point = 0, i;
                while(insertion_point < leaf->key_counter &&
                        leaf->key[insertion_point] < key)
                                insertion_point++;
                for(i=leaf->key_counter ; i>insertion_point ; i--) {
                        leaf->key[i] = leaf->key[i-1];
                        leaf->b_pointer[i] = leaf->b_pointer[i-1];
                }
                leaf->key[insertion_point] = key;
                leaf->b_pointer[insertion_point] = pointer;
                leaf->key_counter++;
                return leaf;

        }

bucket * insert_to_leaf_after_split (bucket * root , unsigned long key ,
                bucket * leaf ,field * pointer) {

                        bucket * new_leaf;
                        unsigned long * temp_key;
                        void ** temp_pointer;
                        int insert_index, split_point, new_key, i, j;
                        new_leaf = make_leaf();
                        temp_key = malloc (order*sizeof(unsigned long));
                        if(temp_key == NULL) {
                                printf("Memory not allocated\n");
                                exit(0);
                        }
                        temp_pointer = malloc (order * sizeof(void *));
                        if(temp_pointer == NULL) {
                                printf("Memory not allocated\n");
                                exit(0);
                        }
                        insert_index = 0;
                        while(insert_index < order-1 &&
                                leaf->key[insert_index] < key)
                                insert_index++;
                        for(i=0,j=0 ; i<leaf->key_counter ; i++,j++) {
                                if(j == insert_index) j++;
                                temp_key[j] = leaf->key[i];
                                temp_pointer[j] = leaf->b_pointer[i];
                        }
                        temp_key[insert_index] = key;
                        temp_pointer[insert_index] = pointer;
                        leaf->key_counter = 0;
                        split_point = div_point (order-1);
                        for(i=0 ; i<split_point ; i++) {
                                leaf->b_pointer[i] = temp_pointer[i];
                                leaf->key[i] = temp_key[i];
                                leaf->key_counter++;
                        }
                        for(i=split_point,j=0 ; i<order ; i++,j++) {
                                new_leaf->b_pointer[j] = temp_pointer[i];
                                new_leaf->key[j] = temp_key[i];
                                new_leaf->key_counter++;
                        }
                                free(temp_key);
                                free(temp_pointer);
                        new_leaf->b_pointer[order-1] =
                                        leaf->b_pointer[order-1];
                        leaf->b_pointer[order-1] = new_leaf;
                        for(i=leaf->key_counter ; i<order-1 ; i++)
                                leaf->b_pointer[i] = NULL;
                        for(i=new_leaf->key_counter ; i<order-1 ; i++)
                                new_leaf->b_pointer[i] = NULL;
                        new_leaf->parent = leaf->parent;
                        new_key = new_leaf->key[0];
                        return insert_to_parent(root,new_key,leaf,
                                new_leaf);

}

bucket * make_new_root(unsigned long key , bucket * left ,
                bucket * right) {

                        bucket * root = make_bucket();
                        root->key[0] = key;
                        root->b_pointer[0] = left;
                        root->b_pointer[1] = right;
                        root->key_counter++;
                        root->parent = NULL;
                        left->parent = root;
                        right->parent = root;
                        return root;

                }

int get_left_position(bucket * parent , bucket * left) {

        int index = 0;
        while(index <= parent->key_counter &&
                parent->b_pointer[index]!= left) {
                        index++;
                }
                return index;

}

bucket * insert_to_node (bucket * root , bucket * p ,
                unsigned long key , int index , bucket * right) {

                        int i;
                        for(i=p->key_counter ; i>index ; i--) {
                                p->b_pointer[i+1] = p->b_pointer[i];
                                p->key[i] = p->key[i-1];
                        }
                        p->b_pointer[index+1] = right;
                        p->key[index] = key;
                        p->key_counter++;
                        return root;

                }

bucket * insert_to_split_node (bucket * root , bucket * old_parent ,
                unsigned long key , int index , bucket * right) {

                        int split_point,i,j,value;
                        unsigned long * temp_keyn;
                        bucket ** temp_pointern;
                        bucket *child, *new_node;
                        temp_keyn = malloc (order * sizeof(unsigned long));
                        if(temp_keyn == NULL) {
                                printf("Memory not allocated\n");
                                exit(0);
                        }
                        temp_pointern = malloc ((order+1) *
                                        sizeof(bucket *));
                        if(temp_pointern == NULL) {
                                printf("Memory not allocated\n");
                                exit(0);
                        }
                        for(i=0,j=0 ; i< old_parent->key_counter + 1 ;
                                i++,j++) {
                                        if(j == index+1) j++;
                                        temp_pointern[j] =
                                             old_parent->b_pointer[i];
                                }
                        for(i=0,j=0 ; i< old_parent->key_counter ;
                                i++,j++) {
                                        if(j == index) j++;
                                        temp_keyn[j] = old_parent->key[i];
                                }
                        temp_pointern[index+1] = right;
                        temp_keyn[index]=key;
                        split_point = div_point (order);
                        new_node = make_bucket();
                        old_parent->key_counter = 0;
                        for(i=0 ; i<split_point-1 ; i++) {
                                old_parent->key[i] = temp_keyn[i];
                                old_parent->b_pointer[i] = temp_pointern[i];
                                old_parent->key_counter++;
                        }
                        old_parent->b_pointer[i] = temp_pointern[i];
                        value = temp_keyn[split_point-1];
                        for(++i,j=0 ; i<order ; i++,j++) {
                                new_node->key[j] = temp_keyn[i];
                                new_node->b_pointer[j] = temp_pointern[i];
                                new_node->key_counter++;
                        }
                        new_node->b_pointer[j] = temp_pointern[i];
                                free(temp_keyn);
                                free(temp_pointern);
                        new_node->parent = old_parent->parent;
                        for(i=0 ; i<= new_node->key_counter ; i++) {
                                child = new_node->b_pointer[i];
                                child->parent = new_node;
                        }
                        return insert_to_parent (root,key,old_parent,
                                        child);
}

bucket * insert_to_parent (bucket * root , unsigned long key , bucket *
                left , bucket * right) {

                        bucket * parent;
                        parent = left->parent;
                        int left_index;
                        if(parent == NULL) {
                                return make_new_root(key,left,right);
                        }
                        left_index = get_left_position(parent , left);
                        if(parent->key_counter < order-1) {
                                return insert_to_node(root,parent,key,
                                        left_index,right);
                        }
                        return insert_to_split_node(root,parent,key,
                                        left_index,right);
}

bucket * master_insert (bucket * root , unsigned long key) {

        bucket * leaf;
        field * pointer;
/*Find if the key is  already present*/
        if(key_exists (root , key , false) != NULL) {
                return root;
        }
        pointer = make_field (key);
/*Checking if root is NULL*/
        if(root == NULL) {
                return create_new_tree (key , pointer);
        }
        leaf = search_leaf (root , key , false);
        if(leaf->key_counter < order-1) {
                leaf = insert_to_leaf (leaf , key , pointer);
                return root;
        }
        return insert_to_leaf_after_split (root , key , leaf , pointer);

}

int main()
{

        unsigned long key;
        int choice,amount;
        bucket *root;
        root = NULL;
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
                        root = master_insert(root, ptr);
                        if(root != NULL)
                        no_of_entries++;
                        printf("\nInsertion of %d element is successful\n",no_of_entries);
                        //print_tree(root);
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