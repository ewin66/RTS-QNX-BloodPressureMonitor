#
#	Makefile for ForkProcesses directory
#

DEBUG = -g
CC = qcc
LD = qcc

TARGET = -Vgcc_ntox86_64
#TARGET = -Vgcc_ntoppcbe
#TARGET = -V3.3.5,gcc_ntoarmle

CFLAGS += $(DEBUG) $(TARGET)
LDFLAGS+= $(DEBUG) $(TARGET)

BINS = startup TerminalInput HeartRateMonitor LCDTerminalOutput BloodPressureMonitor PumpProcess BPTransducerMonitor PulseTransducerMonitor

all: $(BINS)

clean:
	rm -f *.o *.tmp $(BINS)