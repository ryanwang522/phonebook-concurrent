#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>

#include "text_align.h"
#include "phonebook.h"
#include "debug.h"
#include "list.h"

#define ALIGN_FILE "align.txt"

#ifndef THREAD_NUM
#define THREAD_NUM 4
#endif

#define gen(X) _Generic((X), \
                int *: sizeof(int), \
                char *: sizeof(char), \
                entry: sizeof(pbEntry), \
                detail *: sizeof(detail), \
                thread_arg *: sizeof(thread_arg), \
                info: sizeof(pbInfo))

typedef struct __DETAIL {
    char firstName[16];
    char email[16];
    char phone[10];
    char cell[10];
    char addr1[16];
    char addr2[16];
    char city[16];
    char state[2];
    char zip[5];
} detail;

typedef detail *pdetail;

struct __PHONE_BOOK_ENTRY {
    char *lastName;
    pdetail dtl;
    list_t list;
};

typedef struct __THREAD_ARGUMENT {
    char *data_begin;
    char *data_end;
    int threadID;
    int numOfThread;
    entry lEntryPool_begin;     /* local entry pool */
    list_t *localListHead;      /* local list head */
} thread_arg;

static entry entry_pool;
static thread_arg *thread_args[THREAD_NUM];
static char *map;
static off_t file_size;

static entry findLastName(char *lastName, entry pHead)
{
    size_t len = strlen(lastName);
    entry curr = pHead;

    list_for_each_entry(curr, &(pHead->list), list) {
        if (!strncasecmp(lastName, curr->lastName, len)
                && (curr->lastName[len] == '\n' ||
                    curr->lastName[len] == '\0')) {
            curr->lastName[len] = '\0';
            if (!curr->dtl) {
                curr->dtl = (pdetail) allocSpace(curr->dtl);
                assert(curr->dtl && "malloc for curr->dtl error");
            }
            return curr;
        }
    }
    return NULL;
}

static thread_arg *createThread_arg(char *data_begin, char *data_end,
                                    int threadID, int numOfThread,
                                    entry entryPool)
{
    thread_arg *new_arg = (thread_arg *) allocSpace(new_arg);
    assert(new_arg && "malloc for new_arg error\n");

    new_arg->data_begin = data_begin;
    new_arg->data_end = data_end;
    new_arg->threadID = threadID;
    new_arg->numOfThread = numOfThread;
    new_arg->lEntryPool_begin = entryPool;
    INIT_LIST_HEAD(&(entryPool->list));
    new_arg->localListHead = &(entryPool->list);
    return new_arg;
}

/**
 * Generate a local linked list in thread.
 */
static void append(void *arg)
{
    // Remove previous debug code

    thread_arg *t_arg = (thread_arg *) arg;

    char *data = t_arg->data_begin;
    entry localEntry = t_arg->lEntryPool_begin;

    while (data < t_arg->data_end) {
        localEntry->lastName = data;
        localEntry->dtl = NULL;
        list_add_tail(&(localEntry->list),
                      &(t_arg->lEntryPool_begin->list));

        data += MAX_LAST_NAME_SIZE * t_arg->numOfThread;
        localEntry += t_arg->numOfThread;
    }

    pthread_exit(NULL);
}

void show_list_entry(entry pHead)
{
    list_for_each_entry(pHead, &(pHead->list), list)
    printf("%s", pHead->lastName);
}

static entry import(char *fileName)
{
    int i;
    /* File preprocessing */
    text_align(fileName, ALIGN_FILE, MAX_LAST_NAME_SIZE);
    int fd = open(ALIGN_FILE, O_RDONLY | O_NONBLOCK);
    assert(fd && "open ALIGN_FILE error");

    file_size = fsize(ALIGN_FILE);

    /* Build the entry */
    entry pHead;
    printf("size of entry : %lu bytes\n", sizeof(pbEntry));

    pthread_t threads[THREAD_NUM];

    /* Allocate the resource at first */
    map = mmap(NULL, file_size,
               PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    assert(map && "mmap error");

    entry_pool = allocSpaceFor(entry_pool, file_size / MAX_LAST_NAME_SIZE);
    assert(entry_pool && "entry_pool error");

    /* Prepare for multi-threading & Deliver jobs to threads */
    for (i = 0; i < THREAD_NUM; i++) {
        // Created by malloc, remeber to free them.
        thread_args[i] = createThread_arg(map + MAX_LAST_NAME_SIZE * i,
                                          map + file_size, i, THREAD_NUM, entry_pool + i);
        pthread_create(&threads[i], NULL, (void *)&append,
                       (void *)thread_args[i]);
    }

    for (i = 0; i < THREAD_NUM; i++)
        pthread_join(threads[i], NULL);

    /* Connect the linked list of each thread */
    pHead = thread_args[0]->lEntryPool_begin;

    for (i = 1; i < THREAD_NUM; i++) {
        list_splice_tail(thread_args[i]->localListHead, pHead->list.prev);
    }

    close(fd);
    return pHead;
}

static void removeByLastName(char *lastName, entry pHead)
{
    entry e = findLastName(lastName, pHead);

    if (!e) {
        printf("Target not exist.\n");
        return;
    } else {
        (e->list.next)->prev = e->list.prev;
        (e->list.prev)->next = e->list.next;
    }

    free(e->dtl);
}

static void writeFile(double cpu_time[])
{
    FILE *output;
    output = fopen("dll.txt", "a");
    assert(output && "fopen opt.txt error");
    fprintf(output, "append() findLastName() %lf %lf\n", cpu_time[0], cpu_time[1]);
    fclose(output);
}

static void freeSpace(entry pHead)
{
    entry e = pHead;

    list_for_each_entry(e, &(pHead->list), list)
    free(e->dtl);

    free(entry_pool);
    for (int i = 0; i < THREAD_NUM; i++)
        free(thread_args[i]);

    munmap(map, file_size);
}

static info getInfo(entry e)
{
    info f = allocSpace(f);
    assert(f && "malloc for f error");
    strcpy(f->lastName, e->lastName);
    strcpy(f->firstName, e->dtl->firstName);
    strcpy(f->email, e->dtl->email);
    strcpy(f->phone, e->dtl->phone);
    strcpy(f->cell, e->dtl->cell);
    strcpy(f->addr1, e->dtl->addr1);
    strcpy(f->addr2, e->dtl->addr2);
    strcpy(f->city, e->dtl->city);
    strcpy(f->state, e->dtl->state);
    strcpy(f->zip, e->dtl->zip);
    return f;
}

Phonebook DllPBProvider= {
    .find = findLastName,
    .import = import,
    .remove = removeByLastName,
    .write = writeFile,
    .free = freeSpace,
    .getInfo = getInfo,
};
