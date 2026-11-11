# Makefile for Attack of the PETSCII Robots - MEGA65 Port

# Build options
COMPILER ?= cc65
# Options: cc65 or oscar64

# CC65 Toolchain
CC65 = cc65
CL65 = cl65
CA65 = ca65
AR65 = ar65
CC1541 = cc1541

# Oscar64 Toolchain
OSCAR64 ?= oscar64
OSCAR_INC ?= /usr/local/include/oscar64

CFLAGS_CC65 = -O -Oi -Or -Os -Cl -t c64 -I include -I include/mega65
LDFLAGS_CC65 = -C mega65.cfg

SRCS = src/petrobots.c src/PlatformMega65.c src/tileset_data.c
OBJS = $(SRCS:.c=.o)
LIBS = lib/libmega65.a

PRG = PETRobots.prg
D81 = PETRobots.d81

all: $(PRG) $(D81)

ifeq ($(COMPILER),oscar64)

$(PRG): $(SRCS)
	$(OSCAR64) -tm=c64 -i=$(OSCAR_INC) -i=include -i=include/mega65 $(SRCS) -o=$@

else

$(PRG): $(OBJS) $(LIBS)
	$(CL65) $(LDFLAGS_CC65) -o $@ $(OBJS) $(LIBS)

%.o: %.c
	$(CL65) $(CFLAGS_CC65) -c $< -o $@

endif

$(D81): $(PRG)
	$(CC1541) -n "petrobots" -i 65 $@
	$(CC1541) -f petrobots -w $(PRG) $@

clean:
	rm -f $(OBJS) $(PRG) $(D81)

.PHONY: all clean

release: $(PRG)
	mkdir -p release
	$(CC1541) -n "petrobots" -i 65 release/PETRobots.d81
	$(CC1541) -f petrobots -w $(PRG) release/PETRobots.d81
	@echo "Created release/PETRobots.d81"
