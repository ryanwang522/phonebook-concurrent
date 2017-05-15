#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "phonebook.h"

#define gen(X) _Generic((X), \
                int *: sizeof(int), \
                char *: sizeof(char), \
                entry: sizeof(pbEntry))

struct __PHONE_BOOK_ENTRY {
    char lastName[MAX_LAST_NAME_SIZE];
    char firstName[16];
    char email[16];
    char phone[10];
    char cell[10];
    char addr1[16];
    char addr2[16];
    char city[16];
    char state[2];
    char zip[5];
    struct __PHONE_BOOK_ENTRY *pNext;
};

static entry prev;

/* original version */
static entry findLastName(char *lastName, entry pHead)
{
    while (pHead) {
        if (strcasecmp(lastName, pHead->lastName) == 0)
            return pHead;
        prev = pHead;
        pHead = pHead->pNext;
    }
    return NULL;
}

static entry append(char *lastName, entry e)
{
    /* allocate memory for the new entry and put lastName */
    e->pNext = allocSpace(e->pNext);
    assert(e->pNext && "malloc for e->pNext error");

    e = e->pNext;
    strcpy(e->lastName, lastName);
    e->pNext = NULL;

    return e;
}

static entry appendByFile(char *fileName)
{
    FILE *fp = fopen(fileName, "r");
    assert(fp && "fopen fileName error");

    int i = 0;
    char line[MAX_LAST_NAME_SIZE];
    entry pHead, e;
    printf("size of entry : %lu bytes\n", sizeof(pbEntry));

    pHead = allocSpace(pHead);
    assert(pHead && "malloc for pHead error");

    e = pHead;
    e->pNext = NULL;

    while (fgets(line, sizeof(line), fp)) {
        while (line[i] != '\0')
            i++;
        line[i - 1] = '\0';
        i = 0;
        e = append(line, e);
    }

    fclose(fp);

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
    free(e);
}

static void writeFile(double cpu_time[])
{
    FILE *output;
    output = fopen("orig.txt", "a");
    assert(output && "fopen orig.txt error");

    fprintf(output, "append() findLastName() %lf %lf\n", cpu_time[0], cpu_time[1]);
    fclose(output);
}

static void freeSpace(entry pHead)
{
    entry e;
    while (pHead) {
        e = pHead;
        pHead = pHead -> pNext;
        free(e);
    }
}

static char *getLastName(entry e)
{
    return e->lastName;
}

Phonebook OrigPBProvider= {
    .find = findLastName,
    .appendByFile = appendByFile,
    .remove = removeByLastName,
    .write = writeFile,
    .free = freeSpace,
    .getLastName = getLastName,
};
