#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "phonebook_orig.h"

static entry *headPtr;
static entry *prev;

/* original version */
static entry *findName(char lastname[], entry *pHead)
{
    while (pHead) {
        if (strcasecmp(lastname, pHead->lastName) == 0)
            return pHead;
        prev = pHead;
        pHead = pHead->pNext;
    }
    return NULL;
}

static entry *append(char lastName[], entry *e)
{
    /* allocate memory for the new entry and put lastName */
    e->pNext = (entry *) malloc(sizeof(entry));
    e = e->pNext;
    strcpy(e->lastName, lastName);
    e->pNext = NULL;

    return e;
}

static void phonebook_init()
{
    headPtr = (entry *) malloc(sizeof(entry));
    headPtr->pNext = NULL;
}

static entry *phonebook_findName(char lastName[])
{
    return findName(lastName, headPtr);
}

static entry *phonebook_append(char *fileName)
{
    FILE *fp = fopen(fileName, "r");
    if (!fp) {
        printf("cannot open the file!\n");
        return NULL;
    }

    int i = 0;
    char line[MAX_LAST_NAME_SIZE];
    entry *e = headPtr;

    while (fgets(line, sizeof(line), fp)) {
        while (line[i] != '\0') i++;

        line[i - 1] = '\0';
        i = 0;
        e = append(line, e);
    }

    fclose(fp);
    return headPtr;
}

static void phonebook_remove(char lastName[])
{
    entry *e = findName(lastName, headPtr);
    assert(e && "remove error");

    if (e == headPtr)
        headPtr = e->pNext;
    else
        prev->pNext = e->pNext;
    free(e);
}


static void phonebook_free()
{
    entry *e;
    while (headPtr) {
        e = headPtr;
        headPtr = headPtr -> pNext;
        free(e);
    }
}

struct __API Phonebook = {
    .initialize = phonebook_init,
    .findName = phonebook_findName,
    .append = phonebook_append,
    .remove = phonebook_remove,
    .free = phonebook_free,
};
