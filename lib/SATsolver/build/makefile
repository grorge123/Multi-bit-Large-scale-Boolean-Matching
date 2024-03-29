CC=gcc
CFLAGS=-W -Wall -O3 -DNEMBEDDED -DNDEBUG -DNMETRICS -DNSTATISTICS
LD=gcc
AR=ar

VPATH=../src:../test

%.o: %.c ../[st]*/*.h makefile
	$(CC) $(CFLAGS) -c $<

APPSRC=application.c handle.c parse.c witness.c

LIBSRT=$(sort $(wildcard ../src/*.c))
LIBSUB=$(subst ../src/,,$(LIBSRT))
LIBSRC=$(filter-out main.c $(APPSRC),$(LIBSUB))

TSTSRT=$(sort $(wildcard ../test/*.c))
TSTSUB=$(subst ../test/,,$(TSTSRT))
TSTSRC=$(filter-out test.c,$(TSTSUB))

APPOBJ=$(APPSRC:.c=.o)
LIBOBJ=$(LIBSRC:.c=.o)
TSTOBJ=$(APPOBJ) $(TSTSRC:.c=.o)

INCLUDES=-I../$(shell pwd|sed -e 's,.*/,,')

all: libkissat.a kissat
test: all tissat
	./tissat

REMOVE=*.gcda *.gcno *.gcov gmon.out *~ *.proof

clean:
	rm -f kissat tissat
	rm -f *.o *.a
	rm -f $(REMOVE)
	cd ../src; rm -f $(REMOVE)
	cd ../test; rm -f $(REMOVE)

coverage:
	@gcov -o . -s ../src/*.[ch] 2>&1 | \
	../scripts/filter-coverage-output.sh
indent:
	indent ../*/*.[ch]

kissat: main.o $(APPOBJ) libkissat.a makefile
	$(LD) -o $@ $< $(APPOBJ) -L. -lkissat -lm

tissat: test.o $(TSTOBJ) libkissat.a makefile
	$(LD) -o $@ $< $(TSTOBJ) -L. -lkissat -lm

build.h:
	../scripts/generate-build-header.sh > $@

collect.o: sort.c
dense.o: sort.c
propagate.o: assign.c
watch.o: sort.c

build.o: build.c build.h ../[st]*/*.h makefile
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

test.o: test.c build.h ../[st]*/*.h makefile
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

libkissat.a: $(LIBOBJ) $(APPOBJ) makefile
	$(AR) rcs $@ $(LIBOBJ) $(APPOBJ)


.PHONY: all clean coverage indent test build.h

