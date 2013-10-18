# VERSION = $(shell git describe --tags)

CFLAGS := -std=c99 \
	-Wall -Wextra -pedantic \
	-Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes \
	-D_GNU_SOURCE \
	${CFLAGS}

LDLIBS = -larchive -lalpm -lgpgme -lcrypto -lssl
PREFIX = /usr

all: eventd
eventd: eventd.o

clean:
	${RM} eventd *.o

.PHONY: clean install uninstall
