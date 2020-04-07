#ifndef tree_h
#define tree_h
#include "list.h"

enum type_of_next{
    NXT, AND, OR   
};

struct cmd_inf {
    list argv;      //список из импни команды и аргументов
    char *infile;   // Input
    char *outfile;  // Output
    int append;     // =1, >>
    int backgrnd;   // фон реж
    struct cmd_inf* psubcmd; // subshell
    struct cmd_inf* pipe; // канал
    struct cmd_inf* next; // Next cmd after ';' or '&'
    enum type_of_next type;// след команда после && 
};

typedef struct cmd_inf *tree;
typedef struct cmd_inf node;

tree build_tree(list);    
void print_tree(tree, int);
void clear_tree(tree); 
#endif
