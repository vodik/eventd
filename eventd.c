#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/input.h>

static void read_event(int fd)
{
	struct input_event event;

	read(fd, &event, sizeof(struct input_event));
	if (event.type == 0)
		return;

	char type[6], code[6]; /* u16: 5 digits + null */
	char value[12];        /* s32: 10 digits + sign + null */

	sprintf(type, "%d", event.type);
	sprintf(code, "%d", event.code);
	sprintf(value, "%d", event.value);

	switch (fork()) {
		case -1:
			err(1, "fork failed");
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
	fd_set rd;
	int nfds, r;

	while (1) {
		FD_ZERO(&rd);
		FD_SET(fd, &rd);
		nfds = fd + 1;

		r = select(nfds, &rd, NULL, NULL, NULL);
		if (r == -1 && errno != EINTR)
			err(1, "select falsed");

		if (FD_ISSET(fd, &rd))
			read_event(fd);
	}
}

int main(int argc, char* argv[])
{
	int fd = -1;
	char name[256] = "Unknown";

	if (argc < 2)
		errx(1, "not enough arguments");

	fd = open(argv[1], O_RDONLY);
	if (fd < 0)
		err(1, "evdev open failed");

	if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0)
		err(1, "evdev ioctl failed");

	printf("The device on %s says its name is %s\n", argv[1], name);
	select_events(fd);
	close(fd);

	return 0;
}
