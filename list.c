#include "list.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SIZE 5

list lst = NULL;
int EOF_flag = 0;        

extern char **environ;          
static char str[SIZE];          
static char *buf;               
static int str_count;          
static int index_count = 0;    
static int current_word;        
static int realloc_count = 1;   
static int list_size;           
static int q_counter = 0;
extern void error(int, char *message, list *l);  

static void term_list(void);   
static void null_list(void);   
static int getsym(char str_prototype[SIZE], char); 
static void add_sym(char p);                        
static void add_word(void);                        
static void add_word_lite(char *p);                


void  
build_list(void) {
    char b, c;
    char peeped_char = '\0';
    int repeat_it = 1;      // Repeat if error or EOF
    int is_quote = 0;       // Quotes
    int is_one_quote = 0;   // Single quotes
    int quote_in_words = 0; // Amount of quotes in the word
    int quote_one_in_words = 0; // Amount of single quotes in the word
    int is_backslash = 0;  
    buf = NULL;            
    index_count = 0;       
    realloc_count = 1;      
    for (int i = 0; i < SIZE; i++)
        str[i] = '\0';
    if (EOF_flag == 1) {
        lst = NULL;
        return;
    }
    rewind(stdin);
    while (repeat_it != 0) {
        repeat_it = 0;
        str_count = 0;
        null_list();
        fgets(str, SIZE + 1, stdin);
        while ((b = getsym(str, peeped_char)) != EOF) {
            peeped_char = '\0';
            if (is_backslash == 1) {
                is_backslash = 0;
                add_sym(b);
            }
            else if ((b == '&') || (b == '|') || (b == ';') || (b == '>') || (b == '\'') ||
                     (b == '<') || (b == ')') || (b == '(') || (b == '#') || (b == '"')) {
                if (((is_quote == 1) || (is_one_quote == 1)) && ((b != '"') && (b != '\'')))
                    add_sym(b);
                else {
                    add_word();
                    if ((b == '"') || (b == '\'')) {
                        if ((b == '"') && (is_one_quote != 1) && (quote_one_in_words == 0)) {
                            if (is_quote == 0)
                                is_quote = 1;
                            else
                                is_quote = 0;
                            if (quote_in_words == 0)
                                quote_in_words++;
                            else
                                quote_in_words--;
                        }
                        else if ((b == '\'') && (is_quote != 1) && (quote_in_words == 0)) {
                            if (is_one_quote == 0)
                                is_one_quote = 1;
                            else
                                is_one_quote = 0;
                            if (quote_one_in_words == 0)
                                quote_one_in_words++;
                            else
                                quote_one_in_words--;
                        }
                        else if ((b == '\'') && (is_quote == 1) && (quote_in_words > 0)) {
                            q_counter = 1;
                            add_sym(b);
                        }
                        else if ((b == '"') && (is_one_quote == 1) && (quote_one_in_words > 0)) {
                            q_counter = 1;
                            add_sym(b);
                        }
                    } else if (b == '#') {
                        if ((is_quote == 1) || (is_one_quote == 1))
                            add_sym(b);
                        else if ((str[SIZE - 1] == '\n') || (str[SIZE - 1] == '\0'))
                            break;
                        else if (strstr(str, " #") != NULL) {
                            while (b != '\n')
                            b = getchar();
                            break;
                        }
                        else
                            add_sym(b);
                    } else {
                        buf = calloc(2, sizeof(char));
                        buf[0] = b;
                        buf[1] = '\0';
                        if  ((b == '&') || (b == '|') || (b == '>')) {
                            c  = getsym(str, peeped_char);
                            if (b == c) {
                                buf[1] = c;
                                buf = realloc(buf, 3);
                                buf[2] = '\0';
                            } else
                                peeped_char = c;
                        }
                        add_word_lite(buf);
                    }
                }
            }
            else if (b == '\\') {
                 if (is_one_quote == 1)
                    add_sym(b);
                else {
                    /* If it is just '\', then check next */
                    printf(">");
                    c  = getsym(str, peeped_char);
                    if (b == c)
                        /* It is \ */
                        add_sym(b);
                    else {
                        peeped_char = c;
                        if ((is_quote == 0) && (is_quote == 0)) {
                            is_backslash = 1;
                        }
                        else if (is_quote == 1) {
                            is_backslash = 1;
                            if ((c != '$') && (c != '"'))
                                add_sym(b);
                        }
                    }
                }
            }
            else if ((b == ' ') && ((is_quote == 1) || (is_one_quote == 1)))
                add_sym(b);
            else if ((b == ' ') || (b == '\n') || (b == '\t')) {
                add_word();
                q_counter = 0;
                if (b == '\n')
                    break;
            }
            else if (((b >= 'a') && (b <= 'z')) || ((b >= 'A') && (b <= 'Z')) ||
                     ((b >= '0') && (b <= '9')) ||
                     (b == '$') || (b == '_') || (b == '/') || (b == '.') || (b == '-') || (b == ':') || (b == '!') || (b == ','))
                add_sym(b);
            else {
                repeat_it = 1;
                is_quote = 0;
                is_one_quote = 0;
                is_backslash = 0;
                quote_in_words = 0;
                quote_one_in_words = 0;
                free(buf);
                term_list();
                delete_list(lst);
                null_list();
                buf = NULL;
                index_count = 0;
                realloc_count = 1;  
                int clear_stdin = 1;
                for (int i = 0; i < SIZE; i++) {
                    if ((str[i] == '\n') || (str[i] == '\0')) {
                        clear_stdin = 0;
                        break;
                    }
                }
                for (int i = 0; i < SIZE; i++)
                    str[i] = '\0';
                if (clear_stdin)
                    while (((b = getchar()) != EOF) && (b != '\n'));
                rewind(stdin);
                /* Searching for errors */
                error(1, NULL, NULL);
                printf("(my_shell) $ ");
            }
        }
        if ((is_quote == 1) || (quote_in_words == 1) || (quote_one_in_words == 1) || (is_one_quote == 1)) {
            repeat_it = 1;
            is_quote = 0;
            is_one_quote = 0;
            quote_in_words = 0;
            quote_one_in_words = 0;
            is_backslash = 0;
            free(buf);
            term_list();
            delete_list(lst);
            null_list();
            buf = NULL;
            index_count = 0;
            realloc_count = 1; 
            int clear_stdin = 1;
            for (int i = 0; i < SIZE; i++) {
                if ((str[i] == '\n') || (str[i] == '\0')) {
                    clear_stdin = 0;
                    break;
                }
            }
            for (int i = 0; i < SIZE; i++)
                str[i] = '\0';
            if (clear_stdin)
                while (((b = getchar()) != EOF) && (b != '\n'));
            rewind(stdin);
            /* Searching for errors */
            error(1, NULL, NULL);
            printf("(my_shell) $ ");
        }
    }
    /* Saving the last word */
    if (buf != NULL)
        add_word();
    term_list();
    list_size = 0;
    current_word = 0;
    if (b == EOF)
        EOF_flag = 1;
}



int 
getsym(char f_str[SIZE],char p_c) {
    if (p_c != '\0')
        return p_c;
    if ((str_count == SIZE) || ((index_count > 0) && (buf[index_count - 1] == '\n'))) {
        fgets(str, SIZE + 1, stdin);
        str_count = 0;
    }
    if (f_str[str_count] != '\0') {
        return f_str[str_count++];
    } else {
        return EOF;
    }
}


 void 
 add_sym(char b) {
    if (index_count != 0) {
        if (index_count == (SIZE * realloc_count))
            buf = realloc(buf, (++realloc_count * SIZE));
        buf[index_count++] = b;
    } else {
        buf = calloc(SIZE, sizeof(char));
        buf[index_count++] = b;
    }
}


char *
replace(char *str, char *orig, char *rep, int t){
    static char buffer1[4096];
    static char buffer2[4096];
    static char buffer3[4096];
    static char buffer4[4096];
    char *p;
    char *clean = NULL;
    p = clean;
    if (!(p = strstr(str, orig)))
        return str;
    if (t == 1) {
        strncpy(buffer1, str, p-str+1);
        buffer1[p-str] = '\0';
        sprintf(buffer1+(p-str), "%s%s", rep, p+strlen(orig));
        return buffer1;
    } else if (t == 2) {
        strncpy(buffer2, str, p-str+1);
        buffer2[p-str] = '\0';
        sprintf(buffer2+(p-str), "%s%s", rep, p+strlen(orig));
        return buffer2;
    } else if (t == 3) {
        strncpy(buffer3, str, p-str+1);
        buffer3[p-str] = '\0';
        sprintf(buffer3+(p-str), "%s%s", rep, p+strlen(orig));
        return buffer3;
    } else {
        strncpy(buffer4, str, p-str+1);
        buffer4[p-str] = '\0';
        sprintf(buffer4+(p-str), "%s%s", rep, p+strlen(orig));
        return buffer4;
    }
}

// add word

void 
add_word(void) {
    int check1 = 0; 
    int check2 = 0;
    /* Maybe it is the first word for this program */
    if ((index_count != 0) && (q_counter != 1)) {
        if (index_count == (SIZE * realloc_count))
            buf = realloc(buf, (realloc_count) * SIZE + 1);
        buf[index_count++] = '\0';
        buf = realloc(buf, index_count);
        /* Dir ot not? */
        if (strchr(buf, '$') != NULL) {
            while ((strstr(buf, "$HOME") != NULL) || (strstr(buf, "$SHELL") != NULL) || (strstr(buf, "$USER") != NULL) || (strstr(buf, "$EUID") != NULL)) {
                char *tmp = NULL;
                if (strstr(buf, "$SHELL") != NULL) {
                    tmp = replace(buf, "$SHELL", getenv("SHELL"), 1);
                    check1 = 1;
                }
                else
                    tmp = buf;
                if (strstr(buf, "$USER") != NULL) {
                    tmp = replace(tmp, "$USER", getenv("USER"), 2);
                    check2 = 1;
                }
                else if (check1 == 0)
                        tmp = buf;
                if (strstr(buf, "$HOME") != NULL)
                    tmp = replace(tmp, "$HOME", getenv("HOME"), 3);
                else if ((check1 == 0) && (check2 == 0))
                        tmp = buf;
                if (strstr(buf, "$EUID") != NULL) {
                    char timed[4];
                    sprintf(timed, "%u", getuid());
                    tmp = replace(tmp, "$EUID", timed, 4);
                }
                free(buf);
                buf = NULL;
                index_count = 0;
                realloc_count = 1;
                for (int i = 0; tmp[i] != '\0'; i++)
                    add_sym(tmp[i]);
                if (index_count == (SIZE * realloc_count))
                    buf = realloc(buf, (realloc_count) * SIZE + 1);
                buf[index_count++] = '\0';
                buf = realloc(buf, index_count);
            }
        }
        /* Adding buf to lst */
        add_word_lite(buf);
        index_count = 0;
        realloc_count = 1;
    }
}

// to the end

void 
add_word_lite(char *b) {
    if (current_word > list_size - 1)
        lst = realloc(lst, (list_size += SIZE) * sizeof(*lst));
    lst[current_word++] = b;
    buf = NULL;
}

// list creating

void 
null_list() {
    list_size = 0;
    current_word = 0;
    lst = NULL;
}

//Memory balance and NULL at the end of lst

void 
term_list() {
    if (lst == NULL)
        return;
    if (current_word > list_size - 1)
        lst = realloc(lst, (list_size + 1) * sizeof(*lst));
    lst[current_word] = NULL;
    lst = realloc(lst, (list_size = current_word + 1) * sizeof(*lst));
}

//list print 
void 
print_list(list l) {
    if (l == NULL)
        return;
    while (*l != NULL)
    printf("%s\n",*l++);
}

// list delete

void 
delete_list(list l) {
    if (l == NULL)
        return;
    int i;
    for (i = 0; l[i] != NULL; i++) {
        free(l[i]);
        l[i] = NULL;
    }    
    free(l);
    l = NULL;
}
