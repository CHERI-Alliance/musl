#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void *run_thread(void *arg) {
    sleep(100);
    return NULL;
}

int main() {
    void *thread_ret;

    pthread_t thread;
    pthread_create(&thread, NULL, run_thread, NULL);
    pthread_cancel(thread);
    pthread_join(thread, &thread_ret);

    printf("thread returned %#p\n", thread_ret);
    return thread_ret != PTHREAD_CANCELED;
}
