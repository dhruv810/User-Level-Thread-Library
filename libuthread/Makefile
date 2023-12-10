# Target library
lib := libuthread.a


targets := $(lib)
objs    := queue.o uthread.o context.o preempt.o sem.o
deps    := $(patsubst %.o,%.d,$(objs))

all: $(targets)
-include $(deps)

libuthread.a: $(objs)
	@echo "AR $@"
	$(Q)ar -rcs $@ $^

#backwards compatibility to allow command "make debug"
debug: D = 1
debug: all

ifneq ($(V),1)
Q = @
endif

#allow deprecated ucontext API on MacOS
#checking for MacOS example from here: https://git.kernel.org/pub/scm/git/git.git/plain/Makefile?id=c965c029330b1f81cc107c5d829e7fd79c61d8ea
uname := $(shell sh -c 'uname -s 2>/dev/null')
ifeq ($(uname),Darwin)
CFLAGS += -D_XOPEN_SOURCE
CFLAGS += -Wno-deprecated-declarations
endif

CFLAGS += -Wall -Wextra

CFLAGS += -Werror

CFLAGS += -MMD

## Debug flag
ifneq ($(D),1)
CFLAGS	+= -O2
else
CFLAGS	+= -g
endif

%.o: %.c
	@echo "CC $@"
	$(Q)gcc $(CFLAGS) -c -o $@ $<

clean:
	@echo "clean"
	$(Q)rm $(targets) $(objs) $(deps)
