#ifndef _PHONEBOOK_H
#define _PHONEBOOK_H

#define MAX_LAST_NAME_SIZE 16

typedef struct __PHONE_BOOK_ENTRY *entry;

typedef struct __API {
    void (*initialize)();
    entry (*findLastName)(char *lastName, entry pHead);
    entry (*appendByFile)(char *fileName);
    void (*remove)(char *lastName, entry pHead);
    void (*checkAPI)(char *lastName, entry pHead);
    void (*write)(double cpu_time[]);
    void (*free)(entry pHead);
} Phonebook;

extern Phonebook OrigPBProvider;
extern Phonebook OptPBProvider;

#endif
