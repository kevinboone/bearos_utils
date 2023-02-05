# Sample Makefile for a C program that will run under BearOS.

# Define the name of the final executable
TARGET    := bearcalc

include ../Makefile.defs

# Add compiler flags for personal preference
EXTRA_CFLAGS=-Wall -Wextra -O3 

# Construct -I to indicate where the BearOS includes are
INCLUDE_CFLAGS=-I $(BEAROS)/include -I $(BEAROS)/bearos

# Construct the full gcc argument. -MMD enables generation of dependency files
CFLAGS=$(INCLUDE_CFLAGS) -mcpu=cortex-m0plus -MMD -mthumb -nostdlib -nostartfiles -ffreestanding $(EXTRA_CFLAGS) -DBEAROS 

# Make lists of files in my source, in the BearOS library source, and the BearOS start files 
#   (of which there should be only one)
MYSRCS    := $(shell find src/ -type f -name *.c)
LIBSRCS   := $(shell find $(BEAROS)/src/lib -type f -name *.c)
STARTSRCS := $(shell find $(BEAROS)/src/start -type f -name *.c)

# Use pattern substitution to work out where the object files will go, for each of the source files.
# So, for example, C files in the BearOS lib source will go into build/lib
MYOBJS    := $(patsubst src/%,build/my/%,$(MYSRCS:.c=.o))
LIBOBJS   := $(patsubst $(BEAROS)/src/lib/%,build/lib/%,$(LIBSRCS:.c=.o))
STARTOBJS := $(patsubst $(BEAROS)/src/start/%,build/start/%,$(STARTSRCS:.c=.o))

all: check-env $(TARGET) 

OBJS=$(MYOBJS) $(LIBOBJS) $(STARTOBJS)

DEPS=$(MYSRCS/.c=.d) $(LIBSRCS:.c=.d) $(STARTSRC:.c=.d)

# Rules for compiling my source, the library source, and start files

build/lib/%.o: $(BEAROS)/src/lib/%.c
	@mkdir -p build/lib
	$(CC) -Wall -O0 $(CFLAGS) -c $< -o $@ 

build/start/%.o: $(BEAROS)/src/start/%.c
	@mkdir -p build/start
	$(CC) -Wall -O0 $(CFLAGS) -c $< -o $@ 

build/my/%.o: src/%.c 
	@mkdir -p build/my
	$(CC) -Wall -O0 $(CFLAGS) -c $< -o $@ 

# Final link and post-process. We use objcopy to remove unnecessary note and comment sections
#   from the linker output

$(TARGET): $(STARTOBJS) $(MYOBJS) $(LIBOBJS) 
	$(LD) -T ${BEAROS}/link/script.ld -Ttext=$(LOADADDR) $(STARTOBJS) $(LIBOBJS) $(MYOBJS) $(NEWLIB) $(MATHLIB) $(LIBGCC) -o $(TARGET).elf 
	$(OBJCOPY) --remove-section=.comment --remove-section=.note --remove-section=.debug* $(TARGET).elf $(TARGET)
	$(STRIP) $(TARGET)

check-env:
ifndef BEAROS 
	$(error BEAROS is undefined)
endif

# Include all the dependency files into the dependency tree. '-' means not to complain if no
#   dependency files exist, which will be the case on the first build.
-include $(DEPS)

clean:
	rm -rf $(TARGET) build/ *.elf
	find . -name "*.d" -exec rm {} \;
	find . -name "*.o" -exec rm {} \;

stage:
	mkdir -p $(STAGE)/bin
	mkdir -p $(STAGE)/doc/$(TARGET)
	cp -p  $(TARGET) $(STAGE)/bin
	cp -p  doc/* $(STAGE)/doc/$(TARGET)/


