#include <assert.h>
#include "cpe/utils/service.h"
#include "cpe/utils/file.h"

#if defined _WIN32
#elif defined ANDROID
/*not impl for android!*/
#else
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/file.h>

void cpe_daemonize(error_monitor_t em) {
    int i, fd0, fd1, fd2;  
    pid_t pid;  
    struct rlimit rl;  
    struct sigaction sa;  
    int temp;  
  
    /*首先要调用umask将文件模式创建屏蔽字设置为0.由继承得来的文件模式创建屏蔽字可能会拒绝设置某些权限 */
    umask(0);  
  
    /*获取最大的文件描述号  */
    temp = getrlimit(RLIMIT_NOFILE, &rl);  
    if(temp < 0) {
        CPE_ERROR(em, "daemonize: can't get file limit, exit");
        exit(-1);
    }
  
    /* 调用fork，然后使父进程退出（exit）。
       这样做实现了两点：第一，如果该守护进程是作为一条简单shell命令启动的，那么父进程终止使得shell认为这条命令已经执行完毕；
       第二，子进程继承了父进程的进程组ID，但具有一个新的进程ID，这就保证了子进程不是一个进程组的组长进程。这是setsid调用必要前提条件。
     */
    pid = fork();  
    if (pid < 0) {
        CPE_ERROR(em, "daemonize: can't fork(1), exit");
        exit(-1);
    }
    else if (pid != 0) {
        exit(0);
    }
  
    /* 调用setsid以创建一个新会话。执行三个操作，（a）成为新会话的首进程，（b）成为一个新进程的组长进程，（c）没有控制终端。 */
    setsid();
    sa.sa_handler = SIG_IGN;  
    sigemptyset(&sa.sa_mask);  
    sa.sa_flags = 0;  
    temp = sigaction(SIGHUP, &sa, NULL);  
    if(temp < 0) {
        CPE_ERROR(em, "daemonize: can't ignore SIGHUP, exit");
        exit(-1);
    }
  
    /* 确保子进程不会有机会分配到一个控制终端 */
    pid = fork();  
    if (pid < 0) {
        CPE_ERROR(em, "daemonize: can't fork(2), exit");
        assert(0);
        exit(-1);
    }
    else if (pid != 0) {
        exit(0);
    }

    /* 将当前工作目录更改为根目录。进程活动时，其工作目录所在的文件系统不能卸下。一般需要将工作目录改变到根目录。对于需要转储核心，写运行日志的进程将工作目录改变到特定目录 */
    temp = chdir("/");  
    if(temp < 0) {
        CPE_ERROR(em, "daemonize: can't change directoy to '/', exit");
        assert(0);
        exit(-1);
    }

    /* 关闭不再需要的文件描述符。进程从创建它的父进程那里继承了打开的文件描述符。如不关闭，将会浪费系统资源，造成进程所在的文件系统无法卸下以及引起无法预料的错误。 */
    if(rl.rlim_max == RLIM_INFINITY) {
        rl.rlim_max = 1024;  
    }

    for(i=0; i<rl.rlim_max; i++) {
        close(i);
    }
  
    /* 重定向0,1,2到/dev/null，使任何一个试图读标准输入，写标准输出和标准出错的程序库都不会产生任何效果。 */
    fd0 = open("/dev/null", O_RDWR);  
    fd1 = dup(0);  
    fd2 = dup(0);  

    assert(fd0 == 0);
    assert(fd1 == 1);
    assert(fd2 == 2);
    
    if(fd0 != 0 || fd1 != 1 || fd2 != 2) {  
        CPE_ERROR(em, "daemonize: unexpected file descriptors %d %d %d, exit", fd0, fd1, fd2);  
        assert(0);
        exit(-1);
    }
}

int cpe_check_and_write_pid(const char * pidfile, error_monitor_t em) {
    int fd;  
    struct flock fk;  
    char buf[16];  
  
    /* 打开放置记录锁的文件 */
    fd = open(pidfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);  
    if (fd < 0) {  
        CPE_ERROR(em, "write_pid_file %s: can't open, %s, exit", pidfile, strerror(errno));  
        exit(-1);
    }  

    /*试图对文件fd加锁，*/
    fk.l_type = F_WRLCK;  
    fk.l_start = 0;  
    fk.l_whence = SEEK_SET;  
    fk.l_len = 0;  

    if (fcntl(fd, F_SETLK, &fk) < 0) {      /*如果加锁失败的话 */
        /* 如果是因为权限不够或资源暂时不可用，则返回1  */
        if (EACCES == errno || EAGAIN == errno) {  
            close(fd);  
            return 1;
        }

        /* 否则，程序出错，写入一条错误记录后直接退出 */
        CPE_ERROR(em, "write_pid_file %s: can't lock, %s, exit", pidfile, strerror(errno));  
        exit(-1);
    }  
  
    /* 先将文件fd清空，然后再向其中写入当前的进程号 */
    ftruncate(fd, 0);
    snprintf(buf, sizeof(buf), "%d", (int)getpid());
    write(fd, buf, strlen(buf));

    return 0;  
}

int cpe_check_and_remove_pid(const char * pidfile, error_monitor_t em) {
    int fd;  
    struct flock fk;  
  
    /* 打开放置记录锁的文件 */
    fd = open(pidfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);  
    if (fd < 0) {  
        CPE_ERROR(em, "write_pid_file %s: can't open, %s, exit", pidfile, strerror(errno));  
        exit(-1);
    }  

    /*试图对文件fd加锁，*/
    fk.l_type = F_WRLCK;  
    fk.l_start = 0;  
    fk.l_whence = SEEK_SET;  
    fk.l_len = 0;  

    if (fcntl(fd, F_SETLK, &fk) < 0) {      /*如果加锁失败的话 */
        /* 如果是因为权限不够或资源暂时不可用，则返回1  */
        if (EACCES == errno || EAGAIN == errno) {  
            close(fd);  
            return 1;
        }

        /* 否则，程序出错，写入一条错误记录后直接退出 */
        CPE_ERROR(em, "write_pid_file %s: can't lock, %s, exit", pidfile, strerror(errno));  
        exit(-1);
    }  
  
    /* 先将文件fd清空，然后再向其中写入当前的进程号 */
    close(fd);

    if (file_rm(pidfile, em) != 0) {
        CPE_ERROR(em, "write_pid_file %s: remove fail, errno=%d (%s)", pidfile, errno, strerror(errno));
        return -1;
    }

    return 0;  
}

int cpe_kill_by_pidfile(const char * pidfile, int sig, error_monitor_t em) {
    int pid;
    FILE * f;

    if (!file_exist(pidfile, NULL)) return 0;
    
    f = fopen(pidfile, "r");
    if (f == NULL) {
        CPE_ERROR(em, "cpe_kill_by_pidfile %s: open file fail!", pidfile);
        return -1;
    }

    fscanf(f, "%d", &pid);
    if (kill((pid_t)pid, sig) != 0) {
        if (errno != ESRCH) {
            CPE_ERROR(em, "cpe_kill_by_pidfile %s: send sig %d to pid %d fail, %s!", pidfile, sig, pid, strerror(errno));
            return -1;
        }
    }

    fclose(f);
    
    return 0;
}

#endif
