#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main( )
{
  pid_t pid = fork( );

  if( pid == 0 )
  {

    char **arguments;

    arguments[0] = ( char * ) malloc( strlen( "-a" ) );
    arguments[1] = ( char * ) malloc( strlen( "-l" ) );
    arguments[2] = ( char * ) malloc( strlen( "-t" ) );

    strncpy( arguments[0], "-a", strlen( "-a" ) );
    strncpy( arguments[1], "-l", strlen( "-l" ) );
    strncpy( arguments[2], "-t", strlen( "-t" ) );

    // Notice you can add as many NULLs on the end as you want
    int ret = execvp( "ls", arguments );  
  
    if( ret == -1 )
    {
      perror("execl failed: ");
    }
  }
  else 
  {
    int status;
    wait( & status );
  }

  return 0;
}
