TARGET = shortbread

UNAME := $(shell uname -s)

CC = gcc
LD = ld

MY_CFLAGS += -std=gnu11 -Wpedantic -Wall -Wextra -Werror -Wgnu
MY_CFLAGS += -pthread
override CFLAGS += $(MY_CFLAGS)

LDFLAGS += -ldl
ifeq ($(UNAME),Darwin)
	ARCH    := $(shell uname -m)
	LDFLAGS += -arch $(ARCH) -macosx_version_min 10.9.0 -lSystem
endif

SOURCE  = $(wildcard *.c)
OBJECTS = $(SOURCE:.c=.o)

all: debug

release: clean
	@make $(TARGET) CFLAGS="-Ofast"
	strip $(TARGET)

debug:
	@make $(TARGET) CFLAGS="-O0 -g3 -DDEBUG"

$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) $+ -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $^

clean:
	rm -f $(TARGET)
	rm -f $(OBJECTS)

.PHONY: all release debug clean
