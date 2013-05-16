
CC		 = gcc
LD		 = $(CC)
LDD		 = ldd
SIZE		 = size
STRIP		 = strip

CFLAGS		+= -g3 -ggdb3 -fPIC
CFLAGS		+= -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes
CFLAGS		+= -Wmissing-declarations -Wshadow -Wpointer-arith
CFLAGS		+= -Wcast-qual -Wsign-compare -Wformat=2

LDFLAGS		+= -lmagic

SRC		 = identifile.c
PROG		 = $(SRC:.c=)

all:		clean $(PROG)

strip:		$(PROG)
		$(STRIP) $(PROG)

$(PROG):	$(SRC)
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
		-rm -f *.o core core.* $(PROG)

