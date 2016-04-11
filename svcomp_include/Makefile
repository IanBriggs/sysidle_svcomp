
CFLAGS:=-lpthread ${CFLAGS}

BINDIR=bin

_BINS=sysidle sysidle_fail sysidle_sat sysidle_sat_fail
BINS=$(patsubst %,$(BINDIR)/%,$(_BINS))

.PHONY: all
all: ${BINS}

${BINDIR}/%: %.c
	$(CC) $< svcomp_replacements.c -o $@ ${CFLAGS}

