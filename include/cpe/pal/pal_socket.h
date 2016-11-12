#ifndef CPE_PAL_SOCKET_H
#define CPE_PAL_SOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32

#include <WinSock.h>
#include <errno.h>
#include <io.h>

#ifdef EINPROGRESS
#undef EINPROGRESS
#endif
#define EINPROGRESS WSAEINPROGRESS

#ifdef EWOULDBLOCK
#undef EWOULDBLOCK
#endif
#define EWOULDBLOCK WSAEWOULDBLOCK

//extern int cpe_socket_open(int af, int type, int protocol);
#define cpe_sock_open(_af, _type, _protocol) (_open_osfhandle(socket((_af), (_type), (_protocol)), 0))
#define cpe_connect(_fd, _name, _namelen) (connect(_get_osfhandle(_fd), (_name), (_namelen)))
#define cpe_getsockopt(_fd, _level, _optname, _optval, _optlen) (getsockopt(_get_osfhandle(_fd), _level, _optname, _optval, _optlen))
#define cpe_getsockname(_fd, _name, _namelen) (getsockname (_get_osfhandle(_fd), _name, _namelen))
#define cpe_getpeername(_fd, _name, _namelen) (getpeername (_get_osfhandle(_fd), _name, _namelen))
#define cpe_bind(_fd, _addr, _namelen) (bind(_get_osfhandle(_fd), _addr,_namelen))
#define cpe_listen(_fd, _backlog) (listen(_get_osfhandle(_fd), _backlog))
#define cpe_accept(_fd, _addr, _addrlen) (_open_osfhandle(accept(_get_osfhandle(_fd), _addr,_addrlen), 0))
#define cpe_recv(_fd, _buf, _len, _flags) (recv(_get_osfhandle(_fd), _buf, _len, _flags))
#define cpe_recvfrom(_fd, _buf, _len,_flags, _from, _fromlen) (recvfrom(_get_osfhandle(_fd), _buf, _len,_flags, _from, _fromlen))
#define cpe_send(_fd, _buf, _len, _flags) (send(_get_osfhandle(_fd), _buf, _len, _flags))
#define cpe_sendto(_fd, _buf, _len, _flags, _to, _tolen) (sendto(_get_osfhandle(_fd), _buf, _len, _flags, _to, _tolen))

#ifdef _DEBUG
extern int cpe_sock_close (int fd);
#else
#define cpe_sock_close(_fd) (closesocket(_get_osfhandle(_fd)))

#endif
#define cpe_sock_errno() WSAGetLastError()
extern const char *cpe_sock_errstr(int n);

#else

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define cpe_sock_open socket
#define cpe_connect connect
#define cpe_getsockopt getsockopt
#define cpe_getsockname getsockname
#define cpe_getpeername getpeername
#define cpe_bind bind
#define cpe_listen listen
#define cpe_accept accept
#define cpe_recv recv
#define cpe_recvfrom recvfrom
#define cpe_send send
#define cpe_sendto sendto

#define cpe_sock_close close
#define cpe_sock_errno() errno
#define cpe_sock_errstr(n) strerror(n)

#endif

int cpe_sock_set_none_block(int fd, int is_non_block);
int cpe_sock_set_reuseaddr(int fd, int is_reuseaddr);

#ifdef __cplusplus
}
#endif

#endif

