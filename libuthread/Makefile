# Target library
lib := libuthread.a
objs := queue.o uthread.o context.o sem.o preempt.o

CC := gcc
CFLAGS := -Wall -Wextra -Werror -MMD

all: $(lib)

deps := $(patsubst %.o,%.d,$(objs))
-include $(deps)

$(lib): $(objs)
	ar rcs $(lib) $(objs)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(lib) $(objs) $(deps)