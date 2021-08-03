#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define MAGIC 0xb0bacafe

static void *test(void *args);

int main() {
    pthread_attr_t attr;
    if(pthread_attr_init(&attr)) return 1;

    pthread_t thread;
    char *arg_str = "hello world from args!";
    if(pthread_create(&thread, &attr, test, arg_str)) return 2;

    void *thread_ret;
    if(pthread_join(thread, &thread_ret)) return 3;

    if ((uintptr_t) thread_ret != MAGIC) return 4;

    return 0;
}

static void *test(void *args) {
    char *str = (char *) args;
    printf("in thread: [%s]\n", str);

    if (strcmp(str, "hello world from args!")) {
        return (void *) NULL;
    }

    return (void *) MAGIC;
}