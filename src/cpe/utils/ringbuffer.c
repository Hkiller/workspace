#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/ringbuffer.h"
#include "cpe/utils/stream_buffer.h"

#define ALIGN(s) (((s) + 3 ) & ~3)

struct ringbuffer {
	int size;
	int head;
};

INLINE int
block_offset(ringbuffer_t rb, ringbuffer_block_t blk) {
	char * start = (char *)(rb + 1);
	return (int)((char *)blk - start);
}

INLINE ringbuffer_block_t
block_ptr(ringbuffer_t rb, int offset) {
	char * start = (char *)(rb + 1);
	return (ringbuffer_block_t)(start + offset);
}

INLINE ringbuffer_block_t
block_next(ringbuffer_t rb, ringbuffer_block_t blk) {
	int align_length = ALIGN(blk->length);
	int head = block_offset(rb, blk);
	if (align_length + head == rb->size) {
		return NULL;
	}
	assert(align_length + head < rb->size);
	return block_ptr(rb, head + align_length);
}

ringbuffer_t
ringbuffer_new(int size) {
    ringbuffer_block_t blk;
	ringbuffer_t rb;
    
    rb = malloc(sizeof(*rb) + size);
	rb->size = size;
	rb->head = 0;

    blk = block_ptr(rb, 0);
	blk->length = size;
	blk->id = -1;
	return rb;
}

void
ringbuffer_delete(ringbuffer_t rb) {
	free(rb);
}

void
ringbuffer_link(ringbuffer_t rb , ringbuffer_block_t head, ringbuffer_block_t next) {
    assert(head);
	while (head->next >=0) {
		head = block_ptr(rb, head->next);
	}
	next->id = head->id;
	head->next = block_offset(rb, next);
}

ringbuffer_block_t
ringbuffer_unlink(ringbuffer_t rb , ringbuffer_block_t * head) {
    ringbuffer_block_t r = *head;

    if (r == NULL) return NULL;

    *head = r->next >= 0 ? block_ptr(rb, r->next) : NULL;
    r->next = -1;
    
    return r;
}

static ringbuffer_block_t
_alloc(ringbuffer_t rb, int total_size , int size) {
	ringbuffer_block_t blk = block_ptr(rb, rb->head);
    ringbuffer_block_t next;
	int align_length = ALIGN(sizeof(struct ringbuffer_block) + size);

	blk->length = sizeof(struct ringbuffer_block) + size;
	blk->offset = 0;
	blk->next = -1;
	blk->id = -1;
	next = block_next(rb, blk);
	if (next) {
		rb->head = block_offset(rb, next);
		if (align_length < total_size) {
			next->length = total_size - align_length;
			if (next->length >= sizeof(struct ringbuffer_block)) {
				next->id = -1;
			}
		}
	} else {
		rb->head = 0;
	}
	return blk;
}

ringbuffer_block_t
ringbuffer_alloc(ringbuffer_t rb, int size) {
	int align_length = ALIGN(sizeof(struct ringbuffer_block) + size);
	int i;
	for (i=0;i<2;i++) {
		int free_size = 0;
		ringbuffer_block_t blk = block_ptr(rb, rb->head);
		do {
			if (blk->length >= sizeof(struct ringbuffer_block) && blk->id >= 0)
				return NULL;
			free_size += ALIGN(blk->length);
			if (free_size >= align_length) {
				return _alloc(rb, free_size , size);
			}
			blk = block_next(rb, blk);
		} while(blk);
		rb->head = 0;
	}
	return NULL;
}

static int
_last_id(ringbuffer_t rb) {
	int i;
	for (i=0;i<2;i++) {
		ringbuffer_block_t blk = block_ptr(rb, rb->head);
		do {
			if (blk->length >= sizeof(struct ringbuffer_block) && blk->id >= 0)
				return blk->id;
			blk = block_next(rb, blk);
		} while(blk);
		rb->head = 0;
	}
	return -1;
}

int
ringbuffer_collect(ringbuffer_t rb) {
	int id = _last_id(rb);
	ringbuffer_block_t blk = block_ptr(rb, 0);

    if (id < 0) return -1;

	do {
		if (blk->length >= sizeof(struct ringbuffer_block) && blk->id == id) {
			blk->id = -1;
		}
		blk = block_next(rb, blk);
	} while(blk);
	return id;
}

void
ringbuffer_shrink(ringbuffer_t rb, ringbuffer_block_t blk, int size) {
    int align_length;
    int old_length;

	if (size == 0) {
		rb->head = block_offset(rb, blk);
		return;
	}
	align_length = ALIGN(sizeof(struct ringbuffer_block) + size);
	old_length = ALIGN(blk->length);
	assert(align_length <= old_length);
	blk->length = size + sizeof(struct ringbuffer_block);
	if (align_length == old_length) {
		return;
	}
	blk = block_next(rb, blk);
	blk->length = old_length - align_length;
	if (blk->length >= sizeof(struct ringbuffer_block)) {
		blk->id = -1;
	}
	rb->head = block_offset(rb, blk);
}

static int
_block_id(ringbuffer_block_t blk) {
    int id;

	assert(blk->length >= sizeof(struct ringbuffer_block));
    id = blk->id;
	assert(id>=0);
	return id;
}

void
ringbuffer_free(ringbuffer_t rb, ringbuffer_block_t blk) {
    int id;

	if (blk == NULL)
		return;
	id = _block_id(blk);
	blk->id = -1;
	while (blk->next >= 0) {
		blk = block_ptr(rb, blk->next);
		assert(_block_id(blk) == id);
		blk->id = -1;
	}
}

int
ringbuffer_block_data(ringbuffer_t rb, ringbuffer_block_t blk, int skip, void **ptr) {
	int length = blk->length - sizeof(struct ringbuffer_block) - blk->offset;

    if (length >= skip) {
        char * start = (char *)(blk + 1);
        *ptr = (start + blk->offset + skip);
        return length - skip;
    }
    else {
        *ptr = NULL;
        return -1;
    }
}

int ringbuffer_block_len(ringbuffer_t rb, ringbuffer_block_t blk, int skip) {
    return blk->length - sizeof(struct ringbuffer_block) - blk->offset;
}

int ringbuffer_block_total_len(ringbuffer_t rb, ringbuffer_block_t blk) {
	int length;
    length = blk->length - sizeof(struct ringbuffer_block) - blk->offset;
    while (blk->next >= 0) {
        blk = block_ptr(rb, blk->next);
        assert(blk->offset == 0);
        length += blk->length - sizeof(struct ringbuffer_block);
    }

    return length;
}

int
ringbuffer_data(ringbuffer_t rb, ringbuffer_block_t blk, int size, int skip, void **ptr) {
	int length = blk->length - sizeof(struct ringbuffer_block) - blk->offset;
	for (;;) {
		if (length > skip) {
            int ret;

			if (length - skip >= size) {
				char * start = (char *)(blk + 1);
				*ptr = (start + blk->offset + skip);
				return size;
			}
			*ptr = NULL;
			ret = length - skip;
			while (blk->next >= 0) {
				blk = block_ptr(rb, blk->next);
				ret += blk->length - sizeof(struct ringbuffer_block);
				if (ret >= size)
					return ret;
			}
			return ret;
		}
		if (blk->next < 0) {
			assert(length == skip);
			*ptr = NULL;
			return 0;
		}
		blk = block_ptr(rb, blk->next);
		assert(blk->offset == 0);
		skip -= length;
		length = blk->length - sizeof(struct ringbuffer_block);
	}
}

void *
ringbuffer_copy(ringbuffer_t rb, ringbuffer_block_t from, int skip, ringbuffer_block_t to) {
	int size = to->length - sizeof(struct ringbuffer_block);
	int length = from->length - sizeof(struct ringbuffer_block) - from->offset;
	char * ptr = (char *)(to+1);
	for (;;) {
		if (length > skip) {
			char * src = (char *)(from + 1);
			src += from->offset + skip;
			length -= skip;
			while (length < size) {
				memcpy(ptr, src, length);
				ptr += length;
				size -= length;

                if (from->next >= 0) {
                    from = block_ptr(rb , from->next);
                    assert(from->offset == 0);
                    length = from->length - sizeof(struct ringbuffer_block);
                    src =  (char *)(from + 1);
                }
                else {
                    to->id = from->id;
                    return (char *)(to + 1);
                }
			}
			memcpy(ptr, src , size);
			to->id = from->id;
			return (char *)(to + 1);
		}

		if(from->next >= 0) {
            from = block_ptr(rb, from->next);
            assert(from->offset == 0);
            skip -= length;
            length = from->length - sizeof(struct ringbuffer_block);
        }
        else {
			to->id = from->id;
			return (char *)(to + 1);
        }
	}
}

ringbuffer_block_t
ringbuffer_yield(ringbuffer_t rb, ringbuffer_block_t blk, int skip) {
	int length = blk->length - sizeof(struct ringbuffer_block) - blk->offset;
	for (;;) {
		if (length > skip) {
			blk->offset += skip;
			return blk;
		}
		blk->id = -1;
		if (blk->next < 0) {
			return NULL;
		}
		blk = block_ptr(rb, blk->next);
		assert(blk->offset == 0);
		skip -= length;
		length = blk->length - sizeof(struct ringbuffer_block);
	}
}

void ringbuffer_dump_i(write_stream_t s, ringbuffer_t rb) {
    if (rb) {
        ringbuffer_block_t blk = block_ptr(rb,0);
        int i = 0;

        stream_printf(s, "total size= %d\n",rb->size);
        while (blk) {
            ++i;
            if (i > 1024) break;

            if (blk->length >= sizeof(*blk)) {
                stream_printf(s, "[%u : %d]", (unsigned)(blk->length - sizeof(*blk)), block_offset(rb,blk));
                stream_printf(s, " id=%d",blk->id);
                if (blk->id >=0) {
                    stream_printf(s, " offset=%d next=%d",blk->offset, blk->next);
                }
            }
            else {
                stream_printf(s, "<%u : %d>", blk->length, block_offset(rb,blk));
            }

            stream_printf(s, " ");

            blk = block_next(rb, blk);
        }
    }
    else {
        stream_printf(s, "rb null");
    }
}

const char * ringbuffer_dump(mem_buffer_t buffer, ringbuffer_t rb) {
    struct write_stream_buffer s = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);
    mem_buffer_clear_data(buffer);

    ringbuffer_dump_i((write_stream_t)&s, rb);

    mem_buffer_append_char(buffer, 0);

    return mem_buffer_make_continuous(buffer, 0);
}
