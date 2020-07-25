#include "syscall.h"

int
main()
{
    SpaceId newProc;
    OpenFileId input = ConsoleInput;
    OpenFileId output = ConsoleOutput;
    char prompt[3], ch, buffer[60];
    int i;

    prompt[0] = '=';
    prompt[1] = '>';
    prompt[2] = ' ';
    while( 1 )
    {

	Write(prompt, 3, output);

	i = 0;
	
        do {
        
            Read(&buffer[i], 1, input); 

        } while( buffer[i++] != '\n' );

	    buffer[--i] = '\0';

        if( i > 0 ) {
            if(buffer[0] == '-' && buffer[1] == 'x'){
                newProc = Exec(buffer + 3);
                Join(newProc);
            }
            else if(buffer[0] == 'l' && buffer[1] == 's' && buffer[2] == '\0'){
                Ls();
            }
            else if(buffer[0] == 'q'){
                Exit(0);
            }
            else if(buffer[0] == 'p' && buffer[1] == 'w' && buffer[2] == 'd' && buffer[3] == '\0'){
                Pwd();
            }
            else if(buffer[0] == 'c' && buffer[1] == 'd'){
                Cd(buffer + 3);
            }
            else if(buffer[0] == 'm' && buffer[1] == 'k' && buffer[2] != 'd'){
                Create(buffer + 3);
            }
            else if(buffer[0] == 'm' && buffer[1] == 'k' && buffer[2] == 'd'){
                CrDir(buffer + 6);
            }
            else if(buffer[0] == 'r' && buffer[1] == 'm' && buffer[3] != '-'){
                Remove(buffer + 3);
            }
            else if(buffer[0] == 'r' && buffer[1] == 'm' && buffer[3] == '-'){
                RmDir(buffer + 6);
            }
        }
    }
}

