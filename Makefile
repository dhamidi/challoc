PREFIX=/usr/local
CC=gcc
CFLAGS=-O2
LIBFILES=libchalloc.so
INCFILES=challoc.h

.PHONY: install
install: libchalloc.so
	mkdir -p $(PREFIX)/include/challoc
	cp -vf $(INCFILES) $(PREFIX)/include/challoc/
	cp -vf $(LIBFILES) $(PREFIX)/bin/

.PHONY: uninstall
uninstall:
	rm -vf $(foreach file,$(INCFILES),$(PREFIX)/include/challoc/$(file))
	rmdir -v $(PREFIX)/include/challoc
	rm -vf $(foreach file,$(LIBFILES),$(PREFIX)/bin/$(file))

libchalloc.so: challoc.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,libchalloc.so -o $@ $< $(LDFLAGS)

challoc.o: challoc.c challoc.h
	$(CC) $(CFLAGS) -fPIC -c -o $@ $< $(LDFLAGS)