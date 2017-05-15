#ifndef _PHONEBOOK_H
#define _PHONEBOOK_H

#define MAX_LAST_NAME_SIZE 16

typedef struct __PHONE_BOOK_ENTRY *entry;
typedef struct __PHONE_BOOK_ENTRY pbEntry;

#define allocSpace(type) malloc(gen(type))
#define allocSpaceFor(type, objectNum) malloc(gen(type) * objectNum)

typedef struct __API {
    entry (*findLastName)(char *lastName, entry pHead);
    entry (*appendByFile)(char *fileName);
    void (*removeByLastName)(char *lastName, entry pHead);
    void (*checkAPI)(char *lastName, entry pHead);
    void (*write)(double cpu_time[]);
    void (*free)(entry pHead);
} Phonebook;

extern Phonebook OrigPBProvider;
extern Phonebook OptPBProvider;
extern Phonebook DllPBProvider;

#endif
