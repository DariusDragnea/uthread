#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <queue.h>

#define TEST_ASSERT(assert)                                                    \
    do {                                                                       \
        printf("ASSERT: " #assert " ... ");                                    \
        if (assert) {                                                          \
            printf("PASS\n");                                                  \
        } else {                                                               \
            printf("FAIL\n");                                                  \
            exit(1);                                                           \
        }                                                                      \
    } while (0)

int test_count = 0;

/* Testing Functions */
void print_data(queue_t queue, void *data)
{
    (void)queue;
    printf("%d\n", *(int *)data);
    test_count++;
}

void delete_data(queue_t queue, void *data)
{
    printf("deleting: %c\n", *(char *)data);
    queue_delete(queue, data);
    printf("deleted: %c\n", *(char *)data);
    test_count++;
}
//

/* Create */
void test_create(void)
{
    queue_t q;

    fprintf(stderr, "*** TEST create ***\n");

    TEST_ASSERT((q = queue_create()) != NULL);
    queue_destroy(q);
}

/* Enqueue */
void test_enqueue(void)
{
    queue_t q = queue_create();
    int data = 3;
    char data2 = 'a';
    float data3 = 5.5;

    fprintf(stderr, "*** TEST enqueue ***\n");

    queue_enqueue(q, &data);
    queue_enqueue(q, &data2);
    queue_enqueue(q, &data3);
    TEST_ASSERT(queue_length(q) == 3);
    queue_destroy(q);
}

/* Dequeue */
void test_dequeue(void)
{
    queue_t q = queue_create();
    int data = 3;
    char data2 = 'a';
    float data3 = 5.5;

    fprintf(stderr, "*** TEST dequeue ***\n");

    queue_enqueue(q, &data);
    queue_enqueue(q, &data2);
    queue_enqueue(q, &data3);
    void *ptr;
    queue_dequeue(q, &ptr);
    queue_dequeue(q, &ptr);
    queue_dequeue(q, &ptr);
    TEST_ASSERT(queue_length(q) == 0);
    queue_destroy(q);
}

/* Delete */
void test_delete(void)
{
    queue_t q = queue_create();
    int data = 3;
    char data2 = 'a';
    float data3 = 5.5;

    fprintf(stderr, "*** TEST delete ***\n");

    queue_enqueue(q, &data);
    queue_enqueue(q, &data2);
    queue_enqueue(q, &data3);
    queue_delete(q, &data3);
    TEST_ASSERT(queue_length(q) == 2);
    queue_delete(q, &data2);
    TEST_ASSERT(queue_length(q) == 1);
    queue_delete(q, &data);
    TEST_ASSERT(queue_length(q) == 0);
    queue_destroy(q);
}

/* Iterate */
void test_iterate(void)
{
    queue_t q = queue_create();
    int data = 3;
    char data2 = 'a';
    float data3 = 5.5;

    fprintf(stderr, "*** TEST iterate ***\n");
    test_count = 0;
    queue_enqueue(q, &data);
    queue_enqueue(q, &data2);
    queue_enqueue(q, &data3);
    queue_iterate(q, print_data);
    TEST_ASSERT(test_count == 3);

    test_count = 0;
    queue_iterate(q, delete_data);
    TEST_ASSERT(test_count == 3);
    TEST_ASSERT(queue_length(q) == 0);
    queue_destroy(q);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void)
{
    int data = 3, *ptr;
    queue_t q;

    fprintf(stderr, "*** TEST queue_simple ***\n");

    q = queue_create();
    queue_enqueue(q, &data);
    queue_dequeue(q, (void **)&ptr);
    TEST_ASSERT(ptr == &data);
    queue_destroy(q);
}

int main(void)
{
    test_create();
    test_queue_simple();
    test_enqueue();
    test_dequeue();
    test_delete();
    test_iterate();
    return 0;
}