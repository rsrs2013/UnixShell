#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/param.h>


void getPrompt(char * prompt, long unsigned int prompt_size, char * cwd)
{
    char * c = NULL;
    memset(prompt,'\0',sizeof(char) * prompt_size);
    memset(cwd, '\0', sizeof(char) * MAXPATHLEN);
    strcpy(prompt, "1730sh:");
     if (getcwd(cwd, prompt_size) == NULL) 
    {
         perror("getcwd() error");
        return;
    }
    else
    {
        char *homedir = getenv("HOME");
        if (homedir != NULL) 
        {
                if((c = strnstr(cwd, homedir, strlen(homedir)) )!= NULL)
                {
                    *(c + strlen(homedir) - 1) = '~';
                    cwd = c + strlen(homedir) - 1;

                }
        }
        else
        {
            perror("getenv(HOME) error");
            return;   
        }
    }
    if (strlen(cwd) + 1 >
             prompt_size - strlen(prompt))
                 perror("prompt would be truncated");
         (void)strncat(prompt, cwd,
             prompt_size - strlen(prompt) - 1);

    strcat(prompt, "$ ");

}

int main()
{
    int pid;
    char line[81];
    char prompt[MAXPATHLEN];
    char cwd[MAXPATHLEN];
    char *token;
    char *separator = " \t\n";
    char **args;
    char **args2;
    char *cp;
    char *ifile;
    char *ofile;
    int i;
    int j;
    int err;
    enum ENUM {READ,WRITE,APPEND};
    int state =  0;
    char c;
 
    args = malloc(80 * sizeof(char *));
    args2 = malloc(80 * sizeof(char *));

    /* On launch set the cwd to HOME*/
    char *homedir = getenv("HOME");
    if(homedir == NULL)
    {
        perror("getenv(HOME) error");
        exit(EXIT_FAILURE);
    }
    if(chdir(homedir) < 0)
    {
        perror("Couldn't change the directory");
        exit(EXIT_FAILURE);
    }
    else
        getPrompt(prompt,MAXPATHLEN,cwd);
  
    while (1) {
        
        write(1, prompt, strlen(prompt));
        state = 0; // read write append state;
    
        i = 0;
         while(1)
        {
               if(read(0, &c, sizeof(char)))
                    if(c == '\n') break;
               line[i++] =  c;

        }
        line[i] = '\0';

        // split up the line
        i = 0;
        while (1) {
            token = strtok((i == 0) ? line : NULL, separator);
            if (token == NULL)
                break;
            args[i++] = strdup(token);              /* build command array */
        }
        args[i] = NULL;
        if (i == 0)
            continue;

        // assume no redirections
        ofile = NULL;
        ifile = NULL;

        // split off the redirections
        j = 0;
        i = 0;
        err = 0;
        while (1) {
           
            cp = args[i++];
            if (cp == NULL)
                break;

            switch (*cp) {
            case '<':
               
                if (cp[1] == 0)
                    cp = args[i++];
                else
                    ++cp;
                ifile = cp;
                if (cp == NULL)
                    err = 1;
                else
                    if (cp[0] == 0)
                        err = 1;
                break;

            case '>':
                state = WRITE;
                if (cp[1] == 0)
                    cp = args[i++];
                else if(cp[1] == '>')
                {
                    state = APPEND; // state toappending a file
                    cp = args[i++]; // get the file to append
                }
                else
                    ++cp;
                
                ofile = cp;        // set the file name to write output. 
                if (cp == NULL)
                    err = 1;
                else
                    if (cp[0] == 0)
                        err = 1;
                break;

            default:                // when it is not redirection symbol
                args2[j++] = cp;
                break;
            }
        }
        args2[j] = NULL;            // set the last element of args to NULL

        // we got something like "cat <"
        if (err)
            continue;

        // no child arguments
        if (j == 0)
            continue;

        if(strcmp(args2[0],"exit")==0)  /* Exit from the shell program */
        {
            exit(0);
        }

        if(strcmp(args2[0],"cd")==0)    /* Change the directory */
        {   
            if(args2[1] == NULL || args2[1][0] == '~')
            {
                //printf("The argument is %s\n", args2[1]);
                char * str, * homedir;
                if ((homedir = getenv("HOME")) == NULL) 
                {
                     perror("getcwd() error");
                    continue;
                }
                str = (char *)malloc(sizeof(char) * MAXPATHLEN);
                strncpy(str, homedir, strlen(homedir));
                if(args2[1])
                    strncat(str + strlen(homedir),(*(args2+1)+1),strlen(args2[1]) - 1);
                free(args2[1]);
                args2[1] = strdup(str);
                free(str);

            }

            if(chdir(args2[1]) < 0)
                perror("Couldn't change the directory");
            else{
                 getPrompt(prompt,MAXPATHLEN,cwd);
            }
            continue;
        }

        switch (pid = fork()) {
        case 0:
            // open stdin
            if (ifile != NULL) {
                int fd = open(ifile, O_RDONLY);

                if (dup2(fd, STDIN_FILENO) == -1) {
                    perror("dup2 failed");
                }

                close(fd);
            }

            // open stdout
            if (ofile != NULL) {
                int fd2;

                if ((state == WRITE) && (fd2 = open(ofile, O_WRONLY|O_CREAT| O_TRUNC, 0644)) < 0) 
                {
                    perror("couldn't open output file.");
                    exit(0);
                }
                if((state == APPEND) && (fd2 = open(ofile, O_CREAT|O_APPEND|O_WRONLY, 0644)) < 0)
                {
                    perror("couldn't open output file.");
                    exit(0);
                }

                dup2(fd2, STDOUT_FILENO);
                close(fd2);
            }

            execvp(args2[0], args2);        /* child */
            signal(SIGINT, SIG_DFL);
            perror("Error no such program\n");
            exit(1);
            break;

        case -1:
        {
            /* unlikely but possible if hit a limit */
            char * strr = "ERROR can't create child process!\n";
            write(1,strr,strlen(strr));
            break;
        }    

        default:
            wait(NULL);
        }
    }

    exit(0);
}
