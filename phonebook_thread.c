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
    struct __PHONE_BOOK_ENTRY *pNext;
    pdetail dtl;
};

typedef struct __THREAD_ARGUMENT {
    char *data_begin;
    char *data_end;
    int threadID;
    int numOfThread;
    entry lEntryPool_begin;    /* The local entry pool */
    entry lEntry_head; /* local entry linked list */
    entry lEntry_tail; /* local entry linked list */
} thread_arg;

static entry prev;
static entry entry_pool;
static thread_arg *thread_args[THREAD_NUM];
static char *map;
static off_t file_size;

static entry findLastName(char *lastName, entry pHead)
{
    size_t len = strlen(lastName);
    while (pHead) {
        if (!strncasecmp(lastName, pHead->lastName, len)
                && (pHead->lastName[len] == '\n' ||
                    pHead->lastName[len] == '\0')) {
            pHead->lastName[len] = '\0';
            if (!pHead->dtl) {
                pHead->dtl = (pdetail) allocSpace(pHead->dtl);
                assert(pHead->dtl && "malloc for pHead->dtl error");
            }
            return pHead;
        }
        DEBUG_LOG("find string = %s\n", pHead->lastName);
        prev = pHead;
        pHead = pHead->pNext;
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
    new_arg->lEntry_head = new_arg->lEntry_tail = entryPool;
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
    entry localPtr = t_arg->lEntryPool_begin;

    while (data < t_arg->data_end) {
        t_arg->lEntry_tail->pNext = localPtr;
        t_arg->lEntry_tail = t_arg->lEntry_tail->pNext;
        t_arg->lEntry_tail->lastName = data;
        t_arg->lEntry_tail->pNext = NULL;
        t_arg->lEntry_tail->dtl = NULL;

        data += MAX_LAST_NAME_SIZE * t_arg->numOfThread;
        localPtr += t_arg->numOfThread;
    }

    pthread_exit(NULL);
}

void show_entry(entry pHead)
{
    while (pHead) {
        printf("%s", pHead->lastName);
        pHead = pHead->pNext;
    }
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
    entry pHead, e;
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
    pHead = thread_args[0]->lEntry_head;
    DEBUG_LOG("Connect 0 head string %s %p\n",
              pHead->lastName, thread_args[0]->data_begin);
    e = thread_args[0]->lEntry_tail;
    DEBUG_LOG("Connect 0 tail string %s %p\n",
              e->lastName, thread_args[0]->data_begin);
    DEBUG_LOG("round 0\n");

    for (i = 1; i < THREAD_NUM; i++) {
        e->pNext = thread_args[i]->lEntry_head;
        DEBUG_LOG("Connect %d head string %s %p\n", i,
                  e->pNext->lastName, thread_args[i]->data_begin);

        e = thread_args[i]->lEntry_tail;
        DEBUG_LOG("Connect %d tail string %s %p\n", i,
                  e->lastName, thread_args[i]->data_begin);
        DEBUG_LOG("round %d\n", i);
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
    } else if (e == pHead)
        pHead = e->pNext;
    else
        prev->pNext = e->pNext;

    free(e->dtl);
}

static void writeFile(double cpu_time[])
{
    FILE *output;
    output = fopen("thread.txt", "a");
    assert(output && "fopen thread.txt error");
    fprintf(output, "import() findLastName() %lf %lf\n", cpu_time[0], cpu_time[1]);
    fclose(output);
}

static void freeSpace(entry pHead)
{
    entry e = pHead;

    while (e) {
        free(e->dtl);
        e = e -> pNext;
    }

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

struct find_table_ f_t_thread[] = {{findLastName}};
struct import_table_ i_t_thread[] = {{import}};

Phonebook ThreadPBProvider= {
    .ftable_ = f_t_thread,
    .itable_ = i_t_thread,
    .remove = removeByLastName,
    .write = writeFile,
    .free = freeSpace,
    .getInfo = getInfo,
};
