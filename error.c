#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "list.h"
#include "exec.h"

int brackets_count;
int *commands_in_brackets;
int error_flag;
int cur_word_count;

void error(list *l){
    l = l + 1;
    printf("_error_, my Gusness\n");
}


void error_f(void) {
    cur_word_count = 0;
    if (brackets_count != 0) {
        brackets_count = 0;
        commands_in_brackets = realloc(commands_in_brackets, 0);
        free(commands_in_brackets);
    }
    error_flag = 1;
}
