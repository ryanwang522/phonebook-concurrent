CC ?= gcc
CFLAGS ?= -Wall -std=gnu99 -O0 -pthread -g -pg

ifdef CHECK_LEAK
CFLAGS += -fsanitize=address -fno-omit-frame-pointer
endif

ifdef THREAD
CFLAGS  += -D THREAD_NUM=${THREAD}
endif

ifeq ($(strip $(DEBUG)),1)
CFLAGS += -DDEBUG -g
endif

EXEC = phonebook
GIT_HOOKS := .git/hooks/applied
.PHONY: all
all: $(GIT_HOOKS) $(EXEC)

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

SRCS_common = \
    main.c \
    text_align.c

tools/text_align: text_align.c tools/tool-text_align.c
	$(CC) $(CFLAGS_common) $^ -o $@

phonebook: $(SRCS_common) phonebook_orig.c phonebook_thread.c phonebook_dll.c
	$(CC) $(CFLAGS) -o $@ $^

run: $(EXEC)
	echo 3 | sudo tee /proc/sys/vm/drop_caches
	watch -d -t "./phonebook 0 && echo 3 | sudo tee /proc/sys/vm/drop_caches"

cache-test: $(EXEC)
	perf stat --repeat 100 \
		-e cache-misses,cache-references,instructions,cycles \
		./phonebook 0
	perf stat --repeat 100 \
		-e cache-misses,cache-references,instructions,cycles \
		./phonebook 1
	perf stat --repeat 100 \
		-e cache-misses,cache-references,instructions,cycles \
		./phonebook 2

output.txt: cache-test calculate
	./calculate

plot: output.txt
	gnuplot scripts/runtime.gp

calculate: calculate.c
	$(CC) $(CFLAGS_common) $^ -o $@

.PHONY: clean
clean:
	$(RM) $(EXEC) *.o perf.* \
	      	calculate orig.txt thread.txt dll.txt output.txt runtime.png align.txt
