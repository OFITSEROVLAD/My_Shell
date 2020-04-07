#include <stdio.h>
#include <unistd.h>
#include <limits.h>

int
main(int argc, char *argv[])
{
    char str[256];
    // printf("%s$ ", getcwd(NULL, PATH_MAX));
    while (fgets(str, sizeof(str), stdin) != NULL) {
        
        if (!fork()) {
            execlp("sh", "sh", "-c", str, NULL);
            return 1;
        }
        // printf("%s$ ", getcwd(NULL, PATH_MAX));
    }

    return 0;
}