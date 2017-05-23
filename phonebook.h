#ifndef _PHONEBOOK_H
#define _PHONEBOOK_H

#define MAX_LAST_NAME_SIZE 16

typedef struct __PHONE_BOOK_ENTRY *entry;
typedef struct __PHONE_BOOK_ENTRY pbEntry;
typedef struct __PHONE_BOOK_INFO *info;
typedef struct __PHONE_BOOK_INFO pbInfo;

#define allocSpace(type) malloc(gen(type))
#define allocSpaceFor(type, objectNum) malloc(gen(type) * objectNum)

struct __PHONE_BOOK_INFO {
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
};

typedef struct __API {
    entry (*find)(char *lastName, entry pHead);
    entry (*import)(char *fileName);
    void (*remove)(char *lastName, entry pHead);
    void (*write)(double cpu_time[]);
    void (*free)(entry pHead);
    info (*getInfo)(entry e);
} Phonebook;

extern Phonebook OrigPBProvider;
extern Phonebook ThreadPBProvider;
extern Phonebook DllPBProvider;

#endif
