#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <linux/input.h>

static volatile sig_atomic_t got_SIGCHLD = 0;

static void child_sig_handler(int sig)
{
	/* got_SIGCHLD = 1; */
}

static void read_event(int fd)
{
	struct input_event event;

	read(fd, &event, sizeof(struct input_event));
	if (event.type == 0)
		return;

	char type[6], code[6];
	char value[12];

	sprintf(type, "%d", event.type);
	sprintf(code, "%d", event.code);
	sprintf(value, "%d", event.value);

	switch (fork()) {
		case -1:
			break;
		case 0:
			execl("./eventd.sh", "eventd.sh", type, code, value, NULL);
			break;
		default:
			break;
	}

	int status;
	wait(&status);
}

static void select_events(int fd)
{
	sigset_t sigmask, empty_mask;
	struct sigaction sa;
	fd_set rd;
	int nfds, r;

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGCHLD);
	if (sigprocmask(SIG_BLOCK, &sigmask, NULL) == -1) {
		perror("sigprocmask");
		exit(EXIT_FAILURE);
	}

	sa.sa_flags = 0;
	sa.sa_handler = child_sig_handler;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}

	sigemptyset(&empty_mask);

	while (1) {
		FD_ZERO(&rd);
		FD_SET(fd, &rd);
		nfds = fd + 1;

		r = pselect(nfds, &rd, NULL, NULL, NULL, &empty_mask);

		if (r == -1 && errno != EINTR) {
			perror("pselect");
			exit(EXIT_FAILURE);
		}

		if (got_SIGCHLD)
			return;

		if (FD_ISSET(fd, &rd))
			read_event(fd);
	}
}

int main(int argc, char* argv[])
{
	int fd = -1;
	char name[256] = "Unknown";

	if ((fd = open(argv[1], O_RDONLY)) < 0) {
		perror("evdev open");
		exit(EXIT_FAILURE);
	}

	if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0) {
		perror("evdev ioctl");
		exit(EXIT_FAILURE);
	}

	printf("The device on %s says its name is %s\n", argv[1], name);
	select_events(fd);
	close(fd);

	return 0;
}
