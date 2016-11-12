    #include "cpe/vfs/vfs_stream.h"
#include "vfs_file_i.h"
#include "vfs_backend_i.h"

int vfs_write_stream_write(struct write_stream * stream, const void * buf, size_t size) {
    struct vfs_write_stream * fs = (struct vfs_write_stream *)stream;
    return (int)vfs_file_write(fs->m_file, buf, size);
}

int vfs_write_stream_flush(struct write_stream * stream) {
    struct vfs_write_stream * fs = (struct vfs_write_stream *)stream;
    return vfs_file_flush(fs->m_file);
}

void vfs_write_stream_init(struct vfs_write_stream * stream, vfs_file_t f) {
    stream->m_stream.write = vfs_write_stream_write;
    stream->m_stream.flush = vfs_write_stream_flush;
    stream->m_file = f;
}

int vfs_read_stream_read(struct read_stream * stream, void * buf, size_t size) {
    struct vfs_read_stream * fs = (struct vfs_read_stream *)stream;
    return (int)vfs_file_read(fs->m_file, buf, size);
}

void vfs_read_stream_init(struct vfs_read_stream * stream, vfs_file_t f) {
    stream->m_stream.read = vfs_read_stream_read;
    stream->m_file = f;
}
