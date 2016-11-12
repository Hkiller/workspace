#include "cpe/pal/pal_shm.h"

#if defined _WIN32 /*windows*/

#elif defined ANDROID /*android*/

#else /*else windows*/
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int cpe_shm_key_gen(const char * name, char app_id) {
    struct stat state_buf;
    int f_status;

    f_status = stat(name, &state_buf);
    if (f_status == -1) {
        if (errno == ENOENT) {
            int fd = creat(name, 0600);
            if (fd == -1) return -1;
            close(fd);
        }
        else {
            return -1;
        }
    }
    
    return ftok(name, app_id);
}

int cpe_shm_key_get(const char * name, char app_id) {
    struct stat state_buf;
    int f_status;

    f_status = stat(name, &state_buf);
    if (f_status == -1) return -1;

    return ftok(name, app_id);
}

#endif
