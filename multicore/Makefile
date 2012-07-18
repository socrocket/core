configurations := $(wildcard cfg.*.json)
targets := $(configurations:.json=.sim.log)

SIMULATOR=../build/multicore/systemc/multicore.lt.platform
SLEEP=sleep
UART=nc localhost

all: $(targets)

%.sim.log: %.json Makefile
	@-bash -c \
		true ; \
	  echo "export JSONCONFIG=\"$(<)\""; \
	  export JSONCONFIG="$(<)"; \
		echo "$(SIMULATOR) > $(<:.json=.sim.log) 2>&1 &"; \
	  $(SIMULATOR) > $(<:.json=.sim.log) 2>&1 & \
	  $(SLEEP) 6 ; \
	  echo "$(UART) $(shell cat $< | grep port | sed 's/ *"port" : //g' | sed 's/,//g') > $(<:.json=.uart.log) 2>&1 &"; \
	  $(UART) $(shell cat $< | grep port | sed 's/ *"port" : //g' | sed 's/,//g') > $(<:.json=.uart.log) 2>&1 & \
		$(SLEEP) 2 ; \
		wait ; \
	  true
