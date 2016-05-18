ROOT := ../..
include $(ROOT)/Common.mk

ST=$(BIN_DIR)/scribble-tool
projections: master.spr w0.spr w1.spr

master.spr: Protocol.spr
	$(ST) Protocol.spr --project Master > master.spr

w0.spr: Protocol.spr
	$(ST) Protocol.spr --project Worker0 > w0.spr

w1.spr: Protocol.spr
	$(ST) Protocol.spr --project Worker1 > w1.spr

master: master.c master.spr
	$(CC) $(CFLAGS) -o master master.c $(LDFLAGS)

w0: w0.c w0.spr
	$(CC) $(CFLAGS) -o w0 w0.c $(LDFLAGS)

w1: w1.c w1.spr
	$(CC) $(CFLAGS) -o w1 w1.c $(LDFLAGS)

.PHONY: programs
programs: master w0 w1

.PHONY: clean
clean:
	rm -f master.spr w0.spr w1.spr
	rm -f w0 w1 master

.PHONY: all
all: projections programs
