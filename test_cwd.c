#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>


int main()
{   
    char s[100];
  
    // printing current working directory
    printf("%s\n", getcwd(s, 100));
  
    // using the command
    chdir("~/Documents/MyDocuments/cppPractice");
  
    // printing current working directory
    printf("%s\n", getcwd(s, 100));
  
    // after chdir is executed
    return 0;
}