#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

struct queue_node {
    struct queue_node *prev;
    struct queue_node *next;
    void *item;
};

struct queue {
    struct queue_node *head;
    struct queue_node *tail;
    size_t size;
};

queue_t queue_create(void)
{
    queue_t q = malloc(sizeof(struct queue));
    if (q == NULL) {
        return NULL;
    }
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    return q;
}

int queue_destroy(queue_t queue)
{
    if (queue == NULL || queue->size != 0) {
        return -1;
    }
    free(queue);
    return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
    if (queue == NULL || data == NULL) {
        return -1;
    }
    struct queue_node *new_node = malloc(sizeof(struct queue_node));
    if (new_node == NULL) {
        return -1;
    }
    new_node->item = data;
    if (queue->size == 0) {
        new_node->prev = NULL;
        new_node->next = NULL;
        queue->head = new_node;
        queue->tail = new_node;
    } else {
        new_node->prev = queue->tail;
        queue->tail->next = new_node;
        queue->tail = new_node;
        new_node->next = NULL;
    }
    queue->size++;
    return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
    if (queue == NULL || data == NULL) {
        return -1;
    }
    if (queue->size == 0) {
        return -1;
    }

    *data = queue->head->item;
    struct queue_node *temp = queue->head;

    if (queue->size == 1) {
        queue->head = NULL;
        queue->tail = NULL;
        free(temp);
    } else {
        queue->head = queue->head->next;
        queue->head->prev = NULL;
        free(temp);
    }
    queue->size--;
    return 0;
}

int queue_delete(queue_t queue, void *data)
{
    if (queue == NULL || data == NULL) {
        return -1;
    }
    struct queue_node *current = queue->head;
    while (current != NULL) {
        if (current->item == data) {
            if (current == queue->head) {
                queue->head = current->next;
            } else {
                current->prev->next = current->next;
            }
            if (current == queue->tail) {
                queue->tail = current->prev;
            } else {
                current->next->prev = current->prev;
            }
            free(current);
            queue->size--;
            return 0;
        }
        current = current->next;
    }
    return -1;
}

int queue_iterate(queue_t queue, queue_func_t func)
{
    if (queue == NULL || func == NULL) {
        return -1;
    }
    struct queue_node *current = queue->head;
    while (current != NULL) {
        struct queue_node *next = current->next;
        func(queue, current->item);
        current = next;
    }
    return 0;
}

int queue_length(queue_t queue)
{
    if (queue == NULL) {
        return -1;
    }
    return queue->size;
}
