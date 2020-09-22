/*
 * The Shadow Simulator
 * See LICENSE for licensing information
 */

#include <errno.h>
#include <linux/futex.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

// Note: this test roughly follows the example at the end of `man 2 futex`

#define PTR_TO_INT(p) ((int)(long)(p))
#define INT_TO_PTR(i) ((void*)(long)(i))
#define NUM_LOOPS 100
#define UNAVAILABLE 0
#define AVAILABLE 1

// The futex word used to synchronize threads
volatile int futex_word1 = UNAVAILABLE; // initial state is unavailable
volatile int futex_word2 = AVAILABLE; // initial state is available

// Acquire: wait for the futex pointed to by `word` to become 1, then set to 0
static int _futex_wait(volatile int* word) {
    while (1) {
        // Args are: ptr, expected old val, desired new val
        bool is_available = AVAILABLE == __sync_val_compare_and_swap(word, AVAILABLE, UNAVAILABLE);

        if (is_available) {
            break;
        } else {
            int res = syscall(SYS_futex, word, FUTEX_WAIT, UNAVAILABLE, NULL, NULL, 0);
            if (res == -1 && errno != EAGAIN) {
                char errbuf[32] = {0};
                strerror_r(errno, errbuf, 32);
                printf("FUTEX_WAIT syscall failed: error %i: %s", errno, errbuf);
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}

// Release: if the futex pointed to by `word` is 0, set to 1 and wake blocked waiters
static int _futex_post(volatile int* word) {
    bool is_posted = UNAVAILABLE == __sync_val_compare_and_swap(word, UNAVAILABLE, AVAILABLE);

    if (is_posted) {
        int res = syscall(SYS_futex, word, FUTEX_WAKE, AVAILABLE, NULL, NULL, 0);
        if (res == -1) {
            char errbuf[32] = {0};
            strerror_r(errno, errbuf, 32);
            printf("FUTEX_WAKE syscall failed: error %i: %s", errno, errbuf);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

static int _run_futex_loop(volatile int* word1, volatile int* word2, int slow) {
    int threadID = 0;
#ifdef SYS_gettid
    threadID = syscall(SYS_gettid);
#endif

    for (int j = 1; j <= NUM_LOOPS; j++) {
        // Slow down one thread to increase the chance that we'll need a FUTEX_WAIT syscall
        if (slow) {
            usleep(1000);
        }

        if (_futex_wait(word1) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        printf("thread %i loop %i/%i\n", threadID, j, NUM_LOOPS);

        if (_futex_post(word2) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

static void* _run_main_thread(void* arg) {
    return INT_TO_PTR(_run_futex_loop(&futex_word1, &futex_word2, 0));
}

static void* _run_aux_thread(void* arg) {
    return INT_TO_PTR(_run_futex_loop(&futex_word2, &futex_word1, 1));
}

int run() {
    pthread_t aux_thread = {0};

    if (pthread_create(&aux_thread, NULL, _run_aux_thread, NULL) < 0) {
        printf("thread creation failed: error %i: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    void* main_result = _run_main_thread(NULL);

    void* aux_result = NULL;
    if (pthread_join(aux_thread, &aux_result) < 0) {
        printf("thread join failed: error %i: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    bool main_success = PTR_TO_INT(main_result) == 0;
    bool aux_success = PTR_TO_INT(aux_result) == 0;

    if (main_success && aux_success) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}

int main() {
    printf("########## futex test starting ##########\n");

    if (run() == EXIT_SUCCESS) {
        printf("########## futex test passed ##########\n");
        return EXIT_SUCCESS;
    } else {
        printf("########## futex test failed ##########\n");
        return EXIT_FAILURE;
    }
}
