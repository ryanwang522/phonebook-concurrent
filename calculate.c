#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    FILE *fp = fopen("orig.txt", "r");
    FILE *output = fopen("output.txt", "w");
    if (!fp) {
        printf("ERROR opening input file orig.txt\n");
        exit(0);
    }
    char append[50], find[50];
    double orig_sum_a = 0.0, orig_sum_f = 0.0, orig_a, orig_f;
    for (int i = 0; i < 100; i++) {
        if (feof(fp)) {
            printf("ERROR: You need 100 datum instead of %d\n", i);
            printf("run 'make run' longer to get enough information\n\n");
            exit(0);
        }
        fscanf(fp, "%s %s %lf %lf\n", append, find,&orig_a, &orig_f);
        orig_sum_a += orig_a;
        orig_sum_f += orig_f;
    }
    fclose(fp);

    fp = fopen("thread.txt", "r");
    if (!fp) {
        fp = fopen("orig.txt", "r");
        if (!fp) {
            printf("ERROR opening input file opt.txt\n");
            exit(0);
        }
    }
    double thread_sum_a = 0.0, thread_sum_f = 0.0, thread_a, thread_f;
    for (int i = 0; i < 100; i++) {
        if (feof(fp)) {
            printf("ERROR: You need 100 datum instead of %d\n", i);
            printf("run 'make run' longer to get enough information\n\n");
            exit(0);
        }
        fscanf(fp, "%s %s %lf %lf\n", append, find,&thread_a, &thread_f);
        thread_sum_a += thread_a;
        thread_sum_f += thread_f;
    }
    fclose(fp);

    fp = fopen("dll.txt", "r");
    if (!fp) {
        fp = fopen("orig.txt", "r");
        if (!fp) {
            printf("ERROR opening input file opt.txt\n");
            exit(0);
        }
    }
    double dll_sum_a = 0.0, dll_sum_f = 0.0, dll_a, dll_f;
    for (int i = 0; i < 100; i++) {
        if (feof(fp)) {
            printf("ERROR: You need 100 datum instead of %d\n", i);
            printf("run 'make run' longer to get enough information\n\n");
            exit(0);
        }
        fscanf(fp, "%s %s %lf %lf\n", append, find,&dll_a, &dll_f);
        dll_sum_a += dll_a;
        dll_sum_f += dll_f;
    }

    fprintf(output, "append() %lf %lf %lf\n", orig_sum_a / 100.0,
            thread_sum_a / 100.0, dll_sum_a / 100.0);
    fprintf(output, "findLastName() %lf %lf %lf",
            orig_sum_f / 100.0, thread_sum_f / 100.0,
            dll_sum_f / 100.0);
    fclose(output);
    fclose(fp);
    return 0;
}
