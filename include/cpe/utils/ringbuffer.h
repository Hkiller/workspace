#ifndef CPE_RINGBUFFER_H
#define CPE_RINGBUFFER_H
#include "stream.h"
#include "buffer.h"

/* 
  这段代码是从云凤的代码中拿来的,但是为了和这里的代码风格一致，做了部分修改。
  原始代码参见https://github.com/cloudwu/mread
 */

typedef struct ringbuffer * ringbuffer_t;
typedef struct ringbuffer_block * ringbuffer_block_t;

struct ringbuffer_block {
	int length;
	int offset;
	int id;
	int next;
};

ringbuffer_t ringbuffer_new(int size);
void ringbuffer_delete(ringbuffer_t rb);

ringbuffer_block_t ringbuffer_alloc(ringbuffer_t rb, int size);
void ringbuffer_link(ringbuffer_t rb , ringbuffer_block_t prev, ringbuffer_block_t next);
ringbuffer_block_t ringbuffer_unlink(ringbuffer_t rb , ringbuffer_block_t * head);

int ringbuffer_collect(ringbuffer_t rb);
void ringbuffer_shrink(ringbuffer_t rb, ringbuffer_block_t blk, int size);
void ringbuffer_free(ringbuffer_t rb, ringbuffer_block_t blk);
int ringbuffer_block_data(ringbuffer_t rb, ringbuffer_block_t blk, int skip, void **ptr);
int ringbuffer_block_len(ringbuffer_t rb, ringbuffer_block_t blk, int skip);
int ringbuffer_block_total_len(ringbuffer_t rb, ringbuffer_block_t blk);
int ringbuffer_data(ringbuffer_t rb, ringbuffer_block_t blk, int size, int skip, void **ptr);
void * ringbuffer_copy(ringbuffer_t rb, ringbuffer_block_t from, int skip, ringbuffer_block_t to);
ringbuffer_block_t ringbuffer_yield(ringbuffer_t rb, ringbuffer_block_t blk, int skip);

const char * ringbuffer_dump(mem_buffer_t buff, ringbuffer_t rb);

#endif

