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
#define OPT 1

#ifndef THREAD_NUM
#define THREAD_NUM 4
#endif

typedef struct _detail {
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

typedef struct _thread_argument {
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

#ifdef DEBUG
static double diff_in_second(struct timespec t1,
                             struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec-t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}
#endif

static entry findLastName(char *lastName, entry pHead)
{
    size_t len = strlen(lastName);
    while (pHead) {
        if (strncasecmp(lastName, pHead->lastName, len) == 0
                && (pHead->lastName[len] == '\n' ||
                    pHead->lastName[len] == '\0')) {
            pHead->lastName[len] = '\0';
            if (!pHead->dtl)
                pHead->dtl = (pdetail) malloc(sizeof(detail));
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
    thread_arg *new_arg = (thread_arg *) malloc(sizeof(thread_arg));

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
#ifdef DEBUG
    struct timespec start, end;
    double cpu_time;

    clock_gettime(CLOCK_REALTIME, &start);
#endif

    thread_arg *t_arg = (thread_arg *) arg;

    int count = 0;
    entry j = t_arg->lEntryPool_begin;
    for (char *i = t_arg->data_begin; i < t_arg->data_end;
            i += MAX_LAST_NAME_SIZE * t_arg->numOfThread,
            j += t_arg->numOfThread, count++) {
        /* Append the new at the end of the local linked list */
        t_arg->lEntry_tail->pNext = j;
        t_arg->lEntry_tail = t_arg->lEntry_tail->pNext;
        t_arg->lEntry_tail->lastName = i;
        t_arg->lEntry_tail->pNext = NULL;
        t_arg->lEntry_tail->dtl = NULL;
        DEBUG_LOG("thread %d t_argend string = %s\n",
                  t_arg->threadID, t_arg->lEntry_tail->lastName);
    }
#ifdef DEBUG
    clock_gettime(CLOCK_REALTIME, &end);
    cpu_time = diff_in_second(start, end);
#endif

    DEBUG_LOG("thread take %lf sec, count %d\n", cpu_time, count);

    pthread_exit(NULL);
}

void show_entry(entry pHead)
{
    while (pHead) {
        printf("%s", pHead->lastName);
        pHead = pHead->pNext;
    }
}

static entry appendByFile(char *fileName)
{
    int i = 0;
    /* File preprocessing */
    text_align(fileName, ALIGN_FILE, MAX_LAST_NAME_SIZE);
    int fd = open(ALIGN_FILE, O_RDONLY | O_NONBLOCK);
    file_size = fsize(ALIGN_FILE);

    /* Build the entry */
    entry pHead, e;
    printf("size of entry : %lu bytes\n", sizeof(entry));

    pthread_t threads[THREAD_NUM];

    /* Allocate the resource at first */
    map = mmap(NULL, file_size,
               PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    assert(map && "mmap error");
    entry_pool = malloc(sizeof(*entry_pool) *
                        file_size / MAX_LAST_NAME_SIZE);
    assert(entry_pool && "entry_pool error");

    /* Prepare for multi-threading */
    for (i = 0; i < THREAD_NUM; i++)
        // Created by malloc, remeber to free them.
        thread_args[i] = createThread_arg(map + MAX_LAST_NAME_SIZE * i, map + file_size, i, THREAD_NUM, entry_pool + i);

    /* Deliver the jobs to all threads and wait for completing */
    for (i = 0; i < THREAD_NUM; i++)
        pthread_create(&threads[i], NULL, (void *)&append,
                       (void *)thread_args[i]);

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

static void checkAPI(char *lastName, entry pHead)
{
    assert(findLastName(lastName, pHead) &&
           "Did you implement findLastName() in phonebook_opt ?");
    assert(0 == strcmp(findLastName(lastName, pHead)->lastName, lastName));
}

static void writeFile(double cpu_time[])
{
    FILE *output;
    output = fopen("opt.txt", "a");
    fprintf(output, "append() findLastName() %lf %lf\n", cpu_time[0], cpu_time[1]);
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

Phonebook OptPBProvider= {
    .findLastName = findLastName,
    .appendByFile = appendByFile,
    .remove = removeByLastName,
    .checkAPI = checkAPI,
    .write = writeFile,
    .free = freeSpace,
};
