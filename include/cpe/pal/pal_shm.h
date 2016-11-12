#ifndef CPE_PAL_SHM_H
#define CPE_PAL_SHM_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined _WIN32 /*windows*/
#include <windows.h>
#include <errno.h>

#elif defined ANDROID /*android*/
#else
/*posix */

#include <sys/shm.h> 
#include <sys/ipc.h> 
#include <errno.h>
#include <string.h>

typedef int cpe_shm_id_t;

#define CPE_SHM_INVALID_KEY -1

typedef struct shmid_ds cpe_shmid_ds;

int cpe_shm_key_gen(const char * name, char app_id);
int cpe_shm_key_get(const char * name, char app_id);

#define cpe_shm_create(__key, __size, __flags) shmget(__key, __size, IPC_CREAT | IPC_EXCL | __flags)
#define cpe_shm_get(__key) shmget(__key, 0, 066)
#define cpe_shm_rm(__id) shmctl(__id, IPC_RMID, NULL)
#define cpe_shm_attach(__id, __start, __flag) shmat(__id, __start, __flag)
#define cpe_shm_detach(__start) shmdt(__start)
#define cpe_shm_ds_get(__id, __buf) shmctl(__id, IPC_STAT, __buf)

#define cpe_shm_errno() errno
#define cpe_shm_errstr(n) strerror(n)

#endif

#ifdef __cplusplus
}
#endif

#endif

