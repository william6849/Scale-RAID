#ifndef CHUNK_H
#define CHUNK_H
#include <stdint.h>

typedef struct chunk_set chunk_set;
typedef struct chunk_set *chunk_set_p;
struct chunk_set {
    uint32_t col;
    uint32_t row;
    chunk_set_p next;
};
#define CHUNK_HEAD_INIT() \
    {                         \
        0, 0, NULL            \
    }
#define CHUNK_HEAD(name) struct chunk_set name = CHUNK_HEAD_INIT();

#define CHUNK_INIT(node)     \
    do {                     \
        (node)->next = NULL; \
        (node)->row = 0;     \
        (node)->col = 0;     \
    } while (0);

static inline void __chunk_set(struct chunk_set *chunk, uint32_t c, uint32_t r)
{
    chunk->col = c;
    chunk->row = r;
}
#define CHUNK_SET(node, c, r)    \
    do {                         \
        __chunk_set(node, c, r); \
    } while (0);



#define CHUNK_ADD(list, node) \
    do {                      \
        (node)->next = list;  \
        list = node;          \
    } while (0);


#define CHUNK_CLEAN(list)         \
    while (list) {                \
        typeof(list) prev = list; \
        list = (list)->next;      \
        free(prev);               \
    };

#endif