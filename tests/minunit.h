#ifndef minunit_h
#define minunit_h

#define mu_assert(message, test)                \
    do {                                        \
        if (!(test))                            \
            return message;                     \
    } while (0)

#define mu_run_test(test)                       \
    do {                                        \
        printf("RUNNING TEST %-85s", #test);    \
        char *message = test();                 \
        tests_run++;                            \
        if (message) {                          \
            teardown();                         \
            printf(" [ FAIL ]\n");              \
            return message;                     \
        } else {                                \
            teardown();                         \
            printf(" [ OK ]\n");                \
        }                                       \
    } while (0)

extern int tests_run;

#endif
