#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main (int argc, char* argv[]) {
	pid_t pid = fork();
	char location[20] = "/bin/";
	strcat(location, argv[1]);
	if( pid == 0) {
		execl(location, argv[1], NULL);
		exit(0);
	} else if ( pid > 0 ){
		int status;
		wait( &status);
		printf("%d, %s\n", status, argv[0]);
		printf("Hello from the parent\n");
	}
	return 0;
}
