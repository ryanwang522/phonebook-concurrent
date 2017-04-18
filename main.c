#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "phonebook.h"

#ifndef THREAD_NUM
#define THREAD_NUM 4
#endif

#define DICT_FILE "./dictionary/words.txt"

Phonebook *PBProvider[] = {
    &OrigPBProvider,
    &OptPBProvider,
};

static double diff_in_second(struct timespec t1, struct timespec t2)
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

int main(int argc, char *argv[])
{
    Phonebook *pb = PBProvider[SELECTOR];

    struct timespec start, end;
    double cpu_time[2];
    entry pHead;

    /* Compute execution time */
    clock_gettime(CLOCK_REALTIME, &start);
    pHead =  pb->appendByFile(DICT_FILE);
    clock_gettime(CLOCK_REALTIME, &end);

    cpu_time[0] = diff_in_second(start, end);

    /* Find the given entry */
    /* the givn last name to find */
    char input[MAX_LAST_NAME_SIZE] = "zyxel";

    /* check findLastName */
    pb->checkAPI(input, pHead);

#if defined(__GNUC__)
    __builtin___clear_cache((char *) pHead, (char *) pHead + sizeof(entry));
#endif

    /* Compute the execution time */
    clock_gettime(CLOCK_REALTIME, &start);
    pb->findLastName(input, pHead);
    clock_gettime(CLOCK_REALTIME, &end);
    cpu_time[1] = diff_in_second(start, end);

    /* Write the execution time to file. */
    pb->write(cpu_time);

    printf("execution time of append() : %lf sec\n", cpu_time[0]);
    printf("execution time of findName() : %lf sec\n", cpu_time[1]);

    /* Test remove */
    pb->remove("zyoba", pHead);
    if (pb->findLastName("zyoba", pHead) == NULL)
        printf("Remove success\n");

    /* Release memory */
    pb->free(pHead);

    return 0;
}
