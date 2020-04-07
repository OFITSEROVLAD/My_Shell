#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

enum {
    SIZE = 5
};

char *cur_lex;// текущий символ
int cur_word_count = 0;// индекс текущего слова
int brackets_count = 0; // счетчик для контроля кол-ва скобок         
int error_flag = 0;  // 1 если выявится ошибка

void error(list *l); 
static tree make_f_cmd(void);             
static void add_arg(tree t);             
void error_f(void);     


tree 
build_tree(list l) {
    tree beg_cmd; 
    tree prev_cmd;  
    tree cur_cmd; 

    /* ОШИБКА */
    if ((l == NULL) || ((cur_lex = l[cur_word_count++]) == NULL)) {
        cur_word_count = 0;
        return NULL;
    }

    beg_cmd = cur_cmd = make_f_cmd();
    prev_cmd = cur_cmd;

    /* ОШИБКА НА ЛЕКСИЧЕСКОМ ЭТАПЕ */
    if (!(strcmp(cur_lex, ")")) || !(strcmp(cur_lex, "<")) || !(strcmp(cur_lex, ">")) || !(strcmp(cur_lex, ">")) || !(strcmp(cur_lex, ";")) || !(strcmp(cur_lex, "&")) || !(strcmp(cur_lex, "||")) || !(strcmp(cur_lex, "&&")) || !(strcmp(cur_lex, "|"))) {
        error_f();
        error(&l);
        return cur_cmd;
    }


    add_arg(cur_cmd);
    

    cur_lex = l[cur_word_count++];

    while ((cur_lex != NULL) && (!error_flag)) {
        /* ОШИБКА ЕСЛИ НИЧЕГО НЕТ ПОСЛЕ КОМАНДЫ */
        if (((strcmp(cur_lex, "||") == 0) || (strcmp(cur_lex, "&&") == 0) || (strcmp(cur_lex, "|") == 0)) && (l[cur_word_count] == NULL)) {
            error_f();
            error(&l);
            return cur_cmd;
        }

        /* ОШИБКА */
        if (strcmp(cur_lex, "(") == 0) {
            error_f();
            error(&l);
            return cur_cmd;
        }


/*---------------------------ЭТАП ПЕРЕНАПРАВЛЕНИЯ ВВОДА-ВЫВОДА------------------------------*/

        /* СЛУЧАЙ '<' */
        else if (strcmp(cur_lex, "<") == 0) {
            /* ОШИБКА*/
            if (l[cur_word_count] == NULL)  {
                error_f();
                error(&l);
                return cur_cmd;
            }

            cur_lex = l[cur_word_count++];
            /* ОШИБКА */
            if (cur_cmd->infile != NULL) {
                error_f();
                error(&l);
                return cur_cmd;
            }

            long tmp = strlen(cur_lex);
            cur_cmd->infile= calloc(tmp + 1, sizeof(char));
            strncpy(cur_cmd->infile, cur_lex, tmp + 1);
            cur_lex = l[cur_word_count++];
        }

        /* СЛУЧАЙ '>' */
        else if ((strcmp(cur_lex, ">") == 0) || (strcmp(cur_lex, ">>") == 0)) {
            /* ОШИБКА */
            if (cur_cmd->outfile != NULL) {
                error_f();
                error(&l);
                return cur_cmd;
            }
            /* ОШИБКА */
            if (l[cur_word_count] == NULL)  {
                error_f();
                error(&l);
                return cur_cmd;
            }
            if (strcmp(cur_lex, ">>") == 0)
                cur_cmd->append = 1;

            cur_lex = l[cur_word_count++];
            long tmp = strlen(cur_lex);
            cur_cmd->outfile= calloc(tmp + 1, sizeof(char));
            strncpy(cur_cmd->outfile, cur_lex, tmp + 1);
            cur_lex = l[cur_word_count++];
        }

/*--------------------------------------ЭТАП КОНВЕЕР--------------------------------------*/

        else if (strcmp(cur_lex, "|") == 0) {
            cur_cmd = build_tree(l);
            prev_cmd->pipe = cur_cmd;
            if (cur_cmd->backgrnd == 1)
                prev_cmd->backgrnd = 1;
            if (cur_cmd->next != NULL) {
                prev_cmd->next = cur_cmd->next;
                cur_cmd->next = NULL;
                prev_cmd->type = cur_cmd->type;
                cur_cmd->type = NXT;
            }
            prev_cmd = cur_cmd;
        }
/*------------------------------------ОБРАБОТКА КОМАНДЫ &&------------------------------------*/
        else if (strcmp(cur_lex, "&&") == 0) {
            cur_cmd = build_tree(l);
            prev_cmd->next = cur_cmd;
            prev_cmd->type = AND;
            prev_cmd = cur_cmd;
        }


/*------------------------------------ЭТАП ФОНОВЫЙ РЕЖИМ------------------------------------*/
        else if (strcmp(cur_lex, "&") == 0) {
            cur_cmd->backgrnd = 1;
            if ((l[cur_word_count] != NULL) && (l[cur_word_count][0] == ')')) {
                cur_lex = l[cur_word_count++];
                continue;
            }
            cur_cmd = build_tree(l);
            prev_cmd->next = cur_cmd;
            prev_cmd->type = NXT;
            prev_cmd = cur_cmd;
        }
        else {

            if ((cur_cmd->infile != NULL) || (cur_cmd->outfile != NULL) || (cur_cmd->psubcmd != NULL)) {
                error_f();
                error(&l);
                return cur_cmd;
            }
            add_arg(cur_cmd);
            cur_lex = l[cur_word_count++];
        }
    }
    
    cur_word_count = 0;
    return beg_cmd;
}




tree make_f_cmd() {
    tree t = NULL;
    t = calloc(1, sizeof(*t));
    t->argv = NULL;
    t->infile = NULL;
    t->outfile = NULL;
    t->append = 0;
    t->backgrnd = 0;
    t->psubcmd = NULL;
    t->pipe = NULL;
    t->next = NULL;
    return t;
}



void 
add_arg(tree t) {
    int i = 0;
    if (t->argv == NULL)
        t->argv = calloc(1, sizeof(*t->argv));
    while (t->argv[i] != NULL)
        i++;
    long tmp = strlen(cur_lex);
    t->argv[i] = calloc(tmp + 1, sizeof(char));
    strncpy(t->argv[i++], cur_lex, tmp + 1);
    t->argv = realloc(t->argv, ((i + 1) * sizeof(*t->argv)));
    t->argv[i] = NULL;
}



void 
make_shift(int n) {
    while (n--)
        putc(' ', stderr);
}

void 
print_argv(char **p, int shift) {
    char **q = p;
    if (p != NULL) {
        while (*p != NULL) {
            make_shift(shift);
            fprintf(stderr, "argv[%d]=%s\n",(int) (p - q), *p);
            p++;
        }
    }
}

void 
print_tree(tree t, int shift) {
    char **p;
    if (t == NULL)
        return;
    p = t->argv;
    if (p != NULL)
        print_argv(p, shift);
    else {
        make_shift(shift);
        fprintf(stderr, "psubshell\n");
    }
    make_shift(shift);
    if (t->infile == NULL)
        fprintf(stderr, "infile=NULL\n");
    else
        fprintf(stderr, "infile=%s\n", t->infile);
    make_shift(shift);
    if (t->outfile == NULL)
        fprintf(stderr, "outfile=NULL\n");
    else
        fprintf(stderr, "outfile=%s\n", t->outfile);
    make_shift(shift);
    fprintf(stderr, "append=%d\n", t->append);
    make_shift(shift);
    fprintf(stderr, "background=%d\n", t->backgrnd);
    make_shift(shift);
    fprintf(stderr, "type=%s\n", t->type == NXT ? "NXT": t->type ==OR ? "OR": "AND");
    make_shift(shift);
    if(t->psubcmd == NULL)
        fprintf(stderr, "psubcmd=NULL \n");
    else {
        fprintf(stderr, "psubcmd---> \n");
        print_tree(t->psubcmd, shift + 5);
    }
    make_shift(shift);
    if(t->pipe == NULL)
        fprintf(stderr, "pipe=NULL \n");
    else {
        fprintf(stderr, "pipe---> \n");
        print_tree(t->pipe, shift + 5);
    }
    make_shift(shift);
    if(t->next == NULL)
        fprintf(stderr, "next=NULL \n");
    else {
        fprintf(stderr, "next---> \n");
        print_tree(t->next, shift + 5);
    }
}



void 
clear_tree(tree T) {
    if (T == NULL)
        return;
    clear_tree(T->next);
    clear_tree(T->pipe);
    clear_tree(T->psubcmd);
    free(T->infile);
    free(T->outfile);
    delete_list(T->argv);
    free(T);
    T = NULL;
}