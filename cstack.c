#include "cstack.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(VAR) (void)(VAR)

struct node
{
    const struct node* prev;
    unsigned int size;
    char data[];
};

struct stack_entry
{
    int reserved;
    struct node* stack;
};

struct stack_entries_table
{
    unsigned int size;
    struct stack_entry* entries;
};

static struct stack_entries_table g_table = {0u, NULL};

hstack_t stack_new(void)
{
    for (unsigned int i = 0; i < g_table.size; ++i)
    {
        if (g_table.entries[i].reserved == 0)
        {
            g_table.entries[i].reserved = 1;
            g_table.entries[i].stack = NULL;
            return (hstack_t)i;
        }
    }

    unsigned int new_size;
    if (g_table.size == 0)
    {
        new_size = 10;
    }
    else
    {
        new_size = g_table.size * 2;
    }

    struct stack_entry* new_entries =
        realloc(g_table.entries, new_size * sizeof(struct stack_entry));
    if (new_entries == NULL)
    {
        return -1;
    }

    g_table.entries = new_entries;

    for (unsigned int i = g_table.size; i < new_size; ++i)
    {
        g_table.entries[i].reserved = 0;
        g_table.entries[i].stack = NULL;
    }

    g_table.size = new_size;

    g_table.entries[0].reserved = 1;
    g_table.entries[0].stack = NULL;
    return 0;
}

void stack_free(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack))
    {
        return;
    }

    struct node* current = g_table.entries[hstack].stack;
    while (current != NULL)
    {
        struct node* prev = (struct node*)current->prev;
        free(current);
        current = prev;
    }

    g_table.entries[hstack].reserved = 0;
    g_table.entries[hstack].stack = NULL;
}

void stack_free_all(void)
{
    for (unsigned int i = 0; i < g_table.size; ++i)
    {
        if (g_table.entries[i].reserved == 1)
        {
            stack_free((hstack_t)i);
        }
    }

    free(g_table.entries);
    g_table.entries = NULL;
    g_table.size = 0;
}

int stack_valid_handler(const hstack_t hstack)
{
    if (hstack < 0)
    {
        return 0;
    }
    if ((unsigned int)hstack >= g_table.size)
    {
        return 0;
    }
    if (g_table.entries[hstack].reserved == 0)
    {
        return 0;
    }
    return 1;
}

unsigned int stack_size(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack))
    {
        return 0;
    }

    unsigned int count = 0;
    struct node* current = g_table.entries[hstack].stack;
    while (current != NULL)
    {
        ++count;
        current = (struct node*)current->prev;
    }
    return count;
}

void
stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    if (!stack_valid_handler(hstack))
    {
        return;
    }
    if (data_in == NULL || size == 0)
    {
        return;
    }

    unsigned int total_size = sizeof(struct node) + size;
    struct node* new_node = malloc(total_size);
    if (new_node == NULL)
    {
        return;
    }

    new_node->prev = g_table.entries[hstack].stack;
    new_node->size = size;
    memcpy(new_node->data, data_in, size);

    g_table.entries[hstack].stack = new_node;
}

unsigned int
stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    if (!stack_valid_handler(hstack))
    {
        return 0;
    }
    if (data_out == NULL || size == 0)
    {
        return 0;
    }

    struct node* top_node = g_table.entries[hstack].stack;
    if (top_node == NULL)
    {
        return 0;
    }

    if (size < top_node->size)
    {
        return 0;
    }

    memcpy(data_out, top_node->data, top_node->size);
    unsigned int copied_size = top_node->size;

    g_table.entries[hstack].stack = (struct node*)top_node->prev;
    free(top_node);

    return copied_size;
}

unsigned stack_top_size(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack))
    {
        return 0;
    }

    struct node* top_node = g_table.entries[hstack].stack;
    if (top_node == NULL)
    {
        return 0;
    }

    return top_node->size;
}

hstack_t stack_get_all(void)
{
    hstack_t new_stack = stack_new();
    if (new_stack == -1)
    {
        return -1;
    }

    for (unsigned int i = 0; i < g_table.size; ++i)
    {
        if ((hstack_t)i == new_stack)
        {
            continue;
        }
        if (g_table.entries[i].reserved == 1)
        {
            stack_push(new_stack, &i, sizeof(hstack_t));
        }
    }

    return new_stack;
}
