#include <assert.h>
#include "vorbis/vorbisfile.h"
#include "cpe/vfs/vfs_file.h"
#include "ui_cache_sound_buf_i.h"

static size_t ui_cache_sound_load_ogg_read_fun(void *ptr, size_t size, size_t nmemb, void *datasource) {
    vfs_file_t fp = datasource;
    int r;
    size_t readed_size = 0;
    
    while(readed_size < size) {
        r = vfs_file_read(fp, ((char*)ptr) + readed_size, size - readed_size);
        assert(r > 0);
        readed_size += r;
    }

    return readed_size;
}

static int ui_cache_sound_load_ogg_seek_fun(void * datasource, ogg_int64_t off, int whence) {
    vfs_file_t fp = datasource;
    vfs_file_seek_op_t op;
    
    switch(whence) {
    case SEEK_SET:
        op = vfs_file_seek_set;
        break;
    case SEEK_CUR:
        op = vfs_file_seek_cur;
        break;
    case SEEK_END:
        op = vfs_file_seek_end;
        break;
    default:
        assert(0);
        return -1;
    }

	return vfs_file_seek(fp, (ssize_t)off, op);
}

static long ui_cache_sound_load_ogg_tell_fun(void * datasource) {
    vfs_file_t fp = datasource;
	return (long)vfs_file_tell(fp);
}

static ov_callbacks s_ov_callbacks = {
    ui_cache_sound_load_ogg_read_fun,
    ui_cache_sound_load_ogg_seek_fun,
    NULL,
    ui_cache_sound_load_ogg_tell_fun
};

int ui_cache_sound_load_ogg(ui_cache_sound_buf_t buf, vfs_file_t fp, error_monitor_t em, mem_allocrator_t tmp_alloc) {
	OggVorbis_File vf;
    vorbis_info * pvi;
    size_t readed_size;
    int logical_stream;

    assert(buf->m_data == NULL);

    if (ov_open_callbacks((void*)fp, &vf, NULL, 0, s_ov_callbacks) != 0) {
        CPE_ERROR(buf->m_mgr->m_em, "ov_open_callbacks: fail!");
        return -1;
    }

	buf->m_format = ui_cache_sound_format_ogg;
	pvi = ov_info(&vf, 0);

	buf->m_data_size = (uint32_t)ov_pcm_total(&vf, 0);
    buf->m_freq = pvi->rate;
    buf->m_channel = pvi->channels;
    buf->m_bits_per_sample = 16;
    buf->m_data_size *= (buf->m_channel == 1 ? 2 : 4);
	buf->m_data_format = buf->m_channel == 1 ? ui_cache_sound_data_format_mono16 : ui_cache_sound_data_format_stereo16;
	buf->m_bytes_per_sec = buf->m_freq * buf->m_channel * 2;
    buf->m_data = mem_alloc(buf->m_mgr->m_alloc, buf->m_data_size);
    if (buf->m_data == NULL) {
        CPE_ERROR(buf->m_mgr->m_em, "ov_open_callbacks: create data buf fail, size=%u!", buf->m_data_size);
        ov_clear(&vf);
        return -1;
    }

    readed_size = 0;
    while(readed_size < buf->m_data_size) {
        int r = ov_read(&vf, ((char*)buf->m_data) + readed_size, buf->m_data_size - readed_size, 0, 2, 1, &logical_stream);
        if (r <= 0) {
            CPE_ERROR(buf->m_mgr->m_em, "ov_open_callbacks: read data fail!");
            ov_clear(&vf);
            return -1;
        }
        readed_size += r;
    }

    ov_clear(&vf);

    return 0;
}
