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

EXEC = phonebook_orig phonebook_opt phonebook_dll
GIT_HOOKS := .git/hooks/applied
.PHONY: all
all: $(GIT_HOOKS) $(EXEC)

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

SRCS_common = main.c
SRCS = \
    phonebook_orig.c \
    phonebook_opt.c \
	phonebook_dll.c \
    text_align.c

tools/text_align: text_align.c tools/tool-text_align.c
	$(CC) $(CFLAGS_common) $^ -o $@

phonebook_orig: $(SRCS_common) $(SRCS)
	$(CC) $(CFLAGS) -DSELECTOR=0 -o $@ $^

phonebook_opt: $(SRCS_common) $(SRCS)
	$(CC) $(CFLAGS) -DSELECTOR=1 -o $@ $^

phonebook_dll: $(SRCS_common) $(SRCS)
	$(CC) $(CFLAGS) -DSELECTOR=2 -o $@ $^

run: $(EXEC)
	echo 3 | sudo tee /proc/sys/vm/drop_caches
	watch -d -t "./phonebook_orig && echo 3 | sudo tee /proc/sys/vm/drop_caches"

cache-test: $(EXEC)
	perf stat --repeat 100 \
		-e cache-misses,cache-references,instructions,cycles \
		./phonebook_orig
	perf stat --repeat 100 \
		-e cache-misses,cache-references,instructions,cycles \
		./phonebook_opt

output.txt: cache-test calculate
	./calculate

plot: output.txt
	gnuplot scripts/runtime.gp

calculate: calculate.c
	$(CC) $(CFLAGS_common) $^ -o $@

.PHONY: clean
clean:
	$(RM) $(EXEC) *.o perf.* \
	      	calculate orig.txt opt.txt output.txt runtime.png align.txt
