#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include IMPL

#ifndef OPT
#define OUTPUT_FILE "orig.txt"

#else
#define OUTPUT_FILE "opt.txt"
#endif

#ifndef THREAD_NUM
#define THREAD_NUM 4
#endif

#define DICT_FILE "./dictionary/words.txt"

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
    struct timespec start, end;
    double cpu_time1,cpu_time2;

    Phonebook.initialize();

    /* Build the entry */
    entry *pHead;
    printf("size of entry : %lu bytes\n", sizeof(entry));

    /* Compute execution time */
    clock_gettime(CLOCK_REALTIME, &start);
    pHead =  Phonebook.append(DICT_FILE);
    clock_gettime(CLOCK_REALTIME, &end);

    cpu_time1 = diff_in_second(start, end);

    /* Find the given entry */
    /* the givn last name to find */
    char input[MAX_LAST_NAME_SIZE] = "zyxel";

    assert(Phonebook.findName(input) &&
           "Did you implement findName() in " IMPL "?");
    assert(0 == strcmp(Phonebook.findName(input)->lastName, "zyxel"));

#if defined(__GNUC__)
    __builtin___clear_cache((char *) pHead, (char *) pHead + sizeof(entry));
#endif

    /* Compute the execution time */
    clock_gettime(CLOCK_REALTIME, &start);
    Phonebook.findName(input);
    clock_gettime(CLOCK_REALTIME, &end);
    cpu_time2 = diff_in_second(start, end);

    /* Write the execution time to file. */
    FILE *output;
    output = fopen(OUTPUT_FILE, "a");
    fprintf(output, "append() findName() %lf %lf\n", cpu_time1, cpu_time2);
    fclose(output);

    printf("execution time of append() : %lf sec\n", cpu_time1);
    printf("execution time of findName() : %lf sec\n", cpu_time2);

    /* Test remove */
    Phonebook.remove("zyoba");

    /* Release memory */
    Phonebook.free();

    return 0;
}
