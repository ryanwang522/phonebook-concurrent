#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "phonebook.h"

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
    e->pNext = malloc(sizeof(*e->pNext));
    e = e->pNext;
    strcpy(e->lastName, lastName);
    e->pNext = NULL;

    return e;
}

static entry appendByFile(char *fileName)
{
    FILE *fp = fopen(fileName, "r");
    if (!fp) {
        printf("cannot open the file!\n");
        return NULL;
    }

    int i = 0;
    char line[MAX_LAST_NAME_SIZE];
    entry pHead, e;
    printf("size of entry : %lu bytes\n", sizeof(entry));

    pHead = malloc(sizeof(*pHead));
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

static void checkAPI(char *lastName, entry pHead)
{
    assert(findLastName(lastName, pHead) &&
           "Did you implement findLastName() in phonebook_orig ?");
    assert(0 == strcmp(findLastName(lastName, pHead)->lastName, lastName));
}

static void writeFile(double cpu_time[])
{
    FILE *output;
    output = fopen("orig.txt", "a");
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

Phonebook OrigPBProvider= {
    .findLastName = findLastName,
    .appendByFile = appendByFile,
    .removeByLastName = removeByLastName,
    .checkAPI = checkAPI,
    .write = writeFile,
    .free = freeSpace,
};
