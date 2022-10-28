#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>

int main(int argc, char **argv) 
{
        char *homedir = getenv("~");

        if (homedir != NULL) 
        {
                printf("Home dir in enviroment");
                printf("%s\n", homedir);
        }

      /*  uid_t uid = getuid();
        struct passwd *pw = getpwuid(uid);

        if (pw == NULL) 
        {
                printf("Failed\n");
                exit(EXIT_FAILURE);
        }

        printf("%s\n", pw->pw_dir); */

        return 0;
}