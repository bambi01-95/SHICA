# make test
# make test-cycle
# make test-opt

OFLAGS += -g
CFLAGS += -I/opt/local/include -Wall
LDLIBS += -L/opt/local/lib
LDLIBS += -lgc -lm

PROGS = rctest mstest

all : rctest mstest

opt : rctest.c mstest.c rcgc.c msgc.c fatal.c
	$(MAKE) $(PROGS) OFLAGS="-DNDEBUG -O3"

sys : rctest.c mstest.c rcgc.c msgc.c fatal.c
	$(MAKE) $(PROGS) OFLAGS="-DNDEBUG -O3 -DSYSGC=1"

test : all .FORCE
	time ./mstest < test.txt
	time ./rctest < test.txt

test-opt : clean opt .FORCE
	time ./mstest < test-opt.txt
	time ./rctest < test-opt.txt

test-big : clean all .FORCE
	time ./mstest -m 32M < test.txt
	time ./rctest -m 32M < test.txt

test-cycle : all .FORCE
	time ./mstest < test-cycle.txt
	time ./rctest < test-cycle.txt

test-sys : clean sys
	time ./mstest < test-sys.txt
	time ./rctest < test-sys.txt

% : %.c rcgc.c msgc.c fatal.c
	$(CC) $(CFLAGS) $(OFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

%.c : %.leg
	leg -o $@ $<

clean : .FORCE
	rm -rf $(PROGS) *.dSYM *~

.FORCE :
