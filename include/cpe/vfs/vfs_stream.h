#ifndef CPE_VFS_STREAM_H
#define CPE_VFS_STREAM_H
#include "cpe/utils/stream.h"
#include "vfs_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*write*/
struct vfs_write_stream {
    struct write_stream m_stream;
    vfs_file_t m_file;
};

int vfs_write_stream_write(struct write_stream * stream, const void * buf, size_t size);
int vfs_write_stream_flush(struct write_stream * stream);
void vfs_write_stream_init(struct vfs_write_stream * stream, vfs_file_t f);

#define VFS_WRITE_STREAM_INITIALIZER(__file)                  \
    { CPE_WRITE_STREAM_INITIALIZER(vfs_write_stream_write, vfs_write_stream_flush), __file }

/*read*/
struct vfs_read_stream {
    struct read_stream m_stream;
    vfs_file_t m_file;
};

int vfs_read_stream_read(struct read_stream * stream, void * buf, size_t size);
void vfs_read_stream_init(struct vfs_read_stream * stream, vfs_file_t f);

#define VFS_READ_STREAM_INITIALIZER(__file)                        \
    { CPE_READ_STREAM_INITIALIZER(vfs_stream_read), __fp }


#ifdef __cplusplus
}
#endif

#endif
