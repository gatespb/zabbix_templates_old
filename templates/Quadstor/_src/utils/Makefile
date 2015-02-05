include ../build/rules.mk

QUADSTOR_ROOT=/usr/src/quadstor
QUADSTOR_INSTALL_ROOT=/quadstor

CFLAGS += -Wall -I$(QUADSTOR_ROOT)/includes -I$(QUADSTOR_ROOT)/common -I$(QUADSTOR_ROOT)/core -Wl,-rpath=$(QUADSTOR_ROOT)/library/common -Wl,-rpath=$(QUADSTOR_ROOT)/library/client -Wl,-rpath=/quadstor/lib -Wl,-rpath=$(QUADSTOR_ROOT)/library/server

LDLIBS += -L../library/common/ -L../library/client -L../library/server -ltlmsg -ltlclnt

all: qstat

install:
        @set -e; \
        mkdir -p $(QUADSTOR_INSTALL_ROOT)/bin; \
        cp -f qstat $(QUADSTOR_INSTALL_ROOT)/bin;

clean:
        rm -f *.o qstat

clobber: clean
