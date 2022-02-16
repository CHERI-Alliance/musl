#include <errno.h>
#include <pty.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>

int test_openpty(void) {
	int m = 0, s = 0;
	char name[20];
	char* name_s;

	if(openpty(&m, &s, name, NULL, NULL) == -1)
		return 1;

	// Check fd are valid
	if (m == 0 || s == 0)
		return 2;
	if (!(ttyname(m) && (name_s = ttyname(s))))
		return 3;

	if (strncmp(name_s, name, 20))
		return 4;

	write(m, "abcd\n", 5);
	read(s, name, 4);
	if (strncmp(name, "abcd", 4)) return 5;
	return 0;
}

int test_forkpty(void) {
	int m, pid, status, r = 0;
	char name[20];
	char rec[7];
	fd_set fds;
	struct timeval timeout = {.tv_sec = 0, .tv_usec = 20000};

	FD_ZERO(&fds);

	if ((pid = forkpty(&m, name, NULL, NULL)) == -1)
		return 1;

	if (pid == 0) {
		// Check STDIN and STDOUT are assigned
		FD_SET(STDIN_FILENO, &fds);
		if (select(STDIN_FILENO+1, &fds, NULL, NULL, &timeout) > 0)
			exit(3);
		read(STDIN_FILENO, name, 4);
		write(STDOUT_FILENO, "efgh\n", 5);
		if (strncmp(name, "abcd", 4))
			exit(4);
		exit(0);
	} else if (m == 0 || !ttyname(m)) {
		r = 2;
	} else {
		write(m, "abcd\n", 5);
		while (select(m+1, &fds, NULL, NULL, &timeout) > 0)
			read(m, rec, 5);
		if (!r && strncmp(rec, "efgh", 4))
			r = 5;
	}
	if (waitpid(pid, &status, WNOHANG) == -1)
		return 6;
	if (WIFEXITED(status))
		r = WEXITSTATUS(status);
	return r;
}

int main (int argc, char * argv[]) {
	if (argc < 2)
		return -1;
	switch (argv[1][0]){
	case '0':
		return test_openpty();
	case '1':
		return test_forkpty();
	default:
		return -2;
	}
}
