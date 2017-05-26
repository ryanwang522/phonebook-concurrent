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
    struct find_table_ *ftable_;
    struct import_table_ *itable_;
    void (*remove)(char *lastName, entry pHead);
    void (*write)(double cpu_time[]);
    void (*free)(entry pHead);
    info (*getInfo)(entry e);
} Phonebook;

struct find_table_ {
    entry (*find)(char *lastName, entry pHead);
};

struct import_table_ {
    entry (*import)(char *fileName);
};

extern Phonebook OrigPBProvider;
extern Phonebook ThreadPBProvider;
extern Phonebook DllPBProvider;

extern struct find_table_ f_t_orig[], f_t_thread[], f_t_dll[];
extern struct import_table_ i_t_orig[], i_t_thread[], i_t_dll[];
#endif
