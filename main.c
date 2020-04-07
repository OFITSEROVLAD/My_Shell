#include <stdio.h>
#include <setjmp.h>
#include <signal.h>

#include "list.h"
#include "tree.h"
#include "exec.h"

list lst;
int EOF_flag;
int error_flag;
intlist * intL;
tree t = NULL;


int 
main(int argc, char * argv[]) {
    signal(SIGINT, SIG_IGN);
    argc++;
    argv++;
   
    while (1) {
        error_flag = 0;                   
        delete_list(lst);            
        clear_tree(t);  
        clear_zombie(intL);  

        if (EOF_flag != 1) {        
            printf("(my_shell) $ ");
        }  

        build_list();               
        if (EOF_flag == 1) {
            break;
        }

        t = build_tree(lst);       
        if (error_flag == 0) {
            execute_tree(t);
        }
    }
    
    clear_zombie(intL);
    return 0;
}

