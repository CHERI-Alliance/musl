#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    char *shell;
    FILE *shellFile = fopen("/etc/shells", "r");

    if(shellFile)
    {
        char buffer[1024];
        shell = getusershell();
        if(!shell) return 2;

        while(fgets(buffer, sizeof(buffer), shellFile))
        {
            /* Remove trailing new line character from fgets. */
            buffer[strcspn(buffer, "\n")] = 0;

            /* We found a line in /etc/shells containing what getusershell() returned. */
            if(!strcmp(buffer, shell))
            {
                fclose(shellFile);
                return 0;
            }
        }

        fclose(shellFile);
        /* We did not find an entry in /etc/shells matching what getsuershell() returned. */
        return 2;
    }
    else
    {
        shell = getusershell();
        if(strcmp(shell, "/bin/sh") != 0) return 3;

        shell = getusershell();
        if(strcmp(shell, "/bin/csh") != 0) return 4;
    }

    return 0;
}
