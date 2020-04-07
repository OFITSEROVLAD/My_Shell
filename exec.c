#include "exec.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define SIZE 5

int exec_conv(tree); 
int exec_cd(char *argv[]);    
int exec_pwd(tree);     
int exec_cmd (tree);       
void add_zombie(int);          
void kill_process(void);   
int EOF_flag;

int start_pid;                
intlist * intL = NULL;      


int 
exec_conv(tree T) {
    int prev = -1;
    tree P = T;
    tree Q = T;
    int pid = 0;
    int fd[2];
    int status = 0;      

    while (P != NULL) {
        if ((P->argv != NULL) && (!strcmp(P->argv[0], "cd"))) {
            if (prev != -1) {
                close(prev);
            }

            if (P->pipe != NULL) {
                if (pipe(fd) < 0) {
                    exit(1);
                }
                prev = fd[0];
                close(fd[1]);
            }
            status = exec_cd(T->argv);
            return status;
        }
        else if ((P->argv != NULL) && (strcmp(P->argv[0], "exit") == 0)) {
            if (getpid() != start_pid) {
                if (T->argv[1] != NULL) {
                    fprintf(stderr, "\n_error_");
                    exit(1);
                }
                exit(0);
            } else {
                EOF_flag = 1;
                if (T->argv[1] != NULL) {
                    fprintf(stderr, "\n_error_");
                    return 1;
                }
                return 0;
            }
        }

        if (P == T) {
            if (P->pipe != NULL) {
                if (pipe(fd) < 0)
                    exit(1);
                if ((pid = fork()) == 0) {
                    dup2(fd[1], 1);
                    close(fd[0]);
                    close(fd[1]);
                    exec_cmd(P);
                }
                prev = fd[0];
                close(fd[1]);
            }
            else if ((pid = fork()) == 0)
                exec_cmd(P);
        }
        else if (P->pipe == NULL) {
            if ((pid = fork()) == 0) {
                dup2(prev, 0);
                close(prev);
                exec_cmd(P);
            }
            close(prev);
        }
        else {
            if (pipe(fd) < 0)
                exit(1);
            if ((pid = fork()) == 0) {
                dup2(prev, 0);
                dup2(fd[1], 1);
                close(fd[0]);
                close(fd[1]);
                close(prev);
                exec_cmd(P);
            }
            close(fd[1]);
            close(prev);
            prev = fd[0];
        }
        Q = P;
        P = P->pipe;
    }
    while (wait(&status) != -1);
    return status;
}

int 
exec_cmd(tree T) {
    int in, out = 0, res = 0;
    if (T->backgrnd == 1)
        signal(SIGINT, SIG_IGN);
    else
       signal(SIGINT, SIG_DFL);
    if  (T->infile != NULL) {
        if ((in = open(T->infile, O_RDONLY)) < 0) {
            exit(1);
        }  
        dup2(in, 0);
        close(in);
    }
    
    /* Input  */
    if  (T->outfile != NULL) {
        if (T->append == 1) {
            if ((out = open(T->outfile, O_WRONLY | O_CREAT |O_APPEND, 0777)) < 0)
                exit(1);
        }
        else {
            if ((out = open(T->outfile, O_WRONLY | O_CREAT |O_TRUNC, 0777)) < 0)
                exit(1);
        }
        dup2(out, 1);
        close(out);
    }
    /* Background */
    if (T->backgrnd == 1) {
        int f = open("/dev/null", O_RDONLY);
        dup2(f, 0);
        if (fork() == 0) {
            execvp(T->argv[0], T->argv);
            exit(0);
        }
        else
            kill(getpid(), SIGKILL);
    }

    else if ((T->argv != NULL) && (strcmp(T->argv[0], "pwd") == 0)) {
        res = exec_pwd(T);
        exit(res);
    }
    else {
        execvp(T->argv[0], T->argv);
        exit(1);
    }
}


int 
execute_tree (tree T) {
    int res = 0;
    tree P = T;
    start_pid = getpid();
    /* While tree is not */
    while (P != NULL) {
        res = exec_conv(P);
        if ((P->type == AND) && (res != 0)) {
            while ((P->next != NULL) && (P->type == AND)) {
                    P = P->next;
            }
        }
        else if ((P->type == OR) && (res == 0)) {
            while ((P->next != NULL) && (P->type == OR)) {
                P = P->next;
            }
        }
        P = P->next;
    }
    return res;
}


int exec_cd(char *argv[]) {
    char *s;
    if (argv[1] == NULL) {
        s = getenv("HOME");
        if (s == NULL) {
            fprintf(stderr, "_error_\n");
            return 1;
        }
        else
            chdir(s);
    }
    return 0;
}

int 
exec_pwd(tree T) {
    char * dir;
    int attempt = 1;
    //w Error 
    if (T->argv[1] != NULL) {
        fprintf(stderr, "_error_\n");
        return 1;
    }

    dir = calloc(SIZE, sizeof(char));
    getcwd(dir, attempt++ * SIZE);
    while (dir[0] == '\0') {
        dir = realloc(dir, SIZE * attempt * sizeof(char));
        getcwd(dir, attempt++ * SIZE);
    }
    fprintf(stdout, "%s\n", dir);
    free(dir);
    exit(0);
}


void 
kill_process(void) {
    int pid;
    pid = getpid();
    if (start_pid != pid)
    	exit(9);
}


void 
add_zombie(int pid) {
    intlist * tmp;
    tmp = malloc(sizeof(intlist));
    tmp->pid = pid;
    tmp->next = intL;
    intL = tmp;
}


void 
clear_zombie(intlist * intL) {
    int status;
    intlist * tmp;
    while (intL != NULL) {
        waitpid(intL->pid, &status, 0);
        tmp = intL;
        intL = intL->next;
        free(tmp);
    }
    return;
}
