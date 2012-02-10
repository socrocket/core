TARGETS = AES bcnt blit crc des engine fib fir2 hanoi heapsort matrix pocsag queens quicksort

# Now some configuration flags specific for the use with the TSIM LEON2/3 simulator
CFLAGS = -msoft-float -mcpu=v8 -DTSIM_DISABLE_CACHE -DLEON3_CACHE -fno-inline
# Finally remap of the text and data segments to reside into separate memory addresses
# CFLAGS = $(CFLAGS) -Wl,-Ttext=0x8e000000 -Wl,-Tdata=0x40000000

all: $(addsuffix .O0, $(TARGETS)) $(addsuffix .O1, $(TARGETS)) $(addsuffix .O2, $(TARGETS)) $(addsuffix .O3, $(TARGETS))

$(addsuffix .O0, $(TARGETS)): $(addsuffix .c, $(basename $@))
ifneq ("$(CC_CROSS)", "")
	$(CC_CROSS) -o $@ -g3 -O0 $(CFLAGS) $(addsuffix .c, $(basename $@))
else
	echo "CC_CROSS environmental variable not set"
	exit 1
endif

$(addsuffix .O1, $(TARGETS)): $(addsuffix .c, $(basename $@))
ifneq ("$(CC_CROSS)", "")
	$(CC_CROSS) -o $@ -g3 -O1 $(CFLAGS) $(addsuffix .c, $(basename $@))
else
	echo "CC_CROSS environmental variable not set"
	exit 1
endif

$(addsuffix .O2, $(TARGETS)): $(addsuffix .c, $(basename $@))
ifneq ("$(CC_CROSS)", "")
	$(CC_CROSS) -o $@ -g3 -O2 $(CFLAGS) $(addsuffix .c, $(basename $@))
else
	echo "CC_CROSS environmental variable not set"
	exit 1
endif

$(addsuffix .O3, $(TARGETS)): $(addsuffix .c, $(basename $@))
ifneq ("$(CC_CROSS)", "")
	$(CC_CROSS) -o $@ -g3 -O3 $(CFLAGS) $(addsuffix .c, $(basename $@))
else
	echo "CC_CROSS environmental variable not set"
	exit 1
endif

run: $(addsuffix .O0, $(TARGETS)) $(addsuffix .O1, $(TARGETS)) $(addsuffix .O2, $(TARGETS)) $(addsuffix .O3, $(TARGETS))
	./getSpeedResults.py $(SIMULATOR) $(addsuffix .O0, $(TARGETS)) $(addsuffix .O1, $(TARGETS)) $(addsuffix .O2, $(TARGETS)) $(addsuffix .O3, $(TARGETS))

clean:
	rm $(addsuffix .O0, $(TARGETS)) $(addsuffix .O1, $(TARGETS)) $(addsuffix .O2, $(TARGETS)) $(addsuffix .O3, $(TARGETS)) TRAP_stats.csv *~ -rf

