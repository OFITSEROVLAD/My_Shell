#include <unistd.h>
#include "tree.h"

typedef struct backgrndList {   /* список из зомби */
    int pid;
    struct backgrndList *next;
} intlist;

int execute_tree(tree);         
void clear_zombie(intlist *); 
