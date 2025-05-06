#include <cc/IoStream>
#include <sys/types.h>
#include <sys/ioctl.h> // ioctl
#include <sys/socket.h> // socketpair, shutdown, SHUT_*
#include <sys/poll.h> // poll, POLLIN, POLLOUT
#include <termios.h> // struct termios, ECHO
#include <unistd.h> // read, write, select, sysconf, isatty

namespace cc {

IoStream::State::~State()
{
    if (fd_ >= 3) ::close(fd_);
}

bool IoStream::State::wait(IoEvent event, int timeout)
{
    struct pollfd fds;
    fds.fd = fd_;
    fds.events = 0;
    if (event & IoEvent::ReadyRead) fds.events |= POLLIN;
    if (event & IoEvent::ReadyWrite) fds.events |= POLLOUT;

    if (timeout < 0) timeout = -1;
    int ret = -1;
    do ret = ::poll(&fds, 1, timeout);
    while (ret == -1 && errno == EINTR);
    if (ret == -1) CC_SYSTEM_DEBUG_ERROR(errno);
    assert(ret == 0 || ret == 1);
    assert(timeout != -1 || ret == 1);

    return ret == 1;
}

long IoStream::State::read(Out<Bytes> buffer, long maxFill)
{
    long n = (maxFill < 0 || buffer().count() < maxFill) ? buffer().count() : maxFill;
    long m = -1;
    do m = ::read(fd_, buffer(), n);
    while (m == -1 && errno == EINTR);
    if (m == -1) {
        if (errno == EWOULDBLOCK) throw Timeout{};
        if (errno == ECONNRESET || errno == EPIPE) throw InputExhaustion{};
        #if defined __CYGWIN__ || defined __CYGWIN32__
        if (errno == ECONNABORTED) return 0;
        #endif
        CC_SYSTEM_DEBUG_ERROR(errno);
    }
    return m;
}

void IoStream::State::write(const Bytes &buffer, long fill)
{
    const uint8_t *p = buffer.bytes();
    long n = (0 < fill && fill < buffer.count()) ? fill : buffer.count();

    while (n > 0) {
        long m = -1;
        do m = ::write(fd_, p, n);
        while (m == -1 && errno == EINTR);
        if (m == -1) {
            if (errno == EWOULDBLOCK) throw Timeout{};
            if (errno == ECONNRESET || errno == EPIPE) throw OutputExhaustion{};
            CC_SYSTEM_DEBUG_ERROR(errno);
        }
        p += m;
        n -= m;
    }
}

void IoStream::State::write(const List<Bytes> &buffers)
{
    write(buffers.join());
}

IoStream &IoStream::input()
{
    static thread_local IoStream stream { STDIN_FILENO };
    return stream;
}

IoStream &IoStream::output()
{
    #ifdef ESP_PLATFORM
    return error();
    #else
    static thread_local IoStream stream { STDOUT_FILENO };
    return stream;
    #endif
}

IoStream &IoStream::error()
{
    static thread_local IoStream stream { STDERR_FILENO };
    return stream;
}

IoStream::IoStream(int fd):
    Stream{new IoStream::State{fd}}
{}

void IoStream::shutdown(IoShutdown mode)
{
    if (fd() >= 0) {
        int how = 0;
        if (mode == IoShutdown::Read) how |= SHUT_RD;
        if (mode == IoShutdown::Write) how |= SHUT_WR;
        if (mode == IoShutdown::Full) how |= SHUT_RDWR;
        if (::shutdown(fd(), how) == -1)
            CC_SYSTEM_DEBUG_ERROR(errno);
    }
}

void IoStream::duplicateTo(IoStream &other)
{
    if (::dup2(fd(), other.me().fd_) == -1)
        CC_SYSTEM_DEBUG_ERROR(errno);
}

IoStream IoStream::duplicate() const
{
    int fd2 = ::dup(fd());
    if (fd2 == -1) CC_SYSTEM_DEBUG_ERROR(errno);
    return IoStream{fd2};
}

bool IoStream::isInteractive() const
{
    return ::isatty(fd());
}

void IoStream::echo(bool on)
{
    struct termios settings;

    if (::tcgetattr(fd(), &settings) == -1)
        CC_SYSTEM_DEBUG_ERROR(errno);

    if (((settings.c_lflag & ECHO) != 0) == on) return;

    if (on)
        settings.c_lflag |= ECHO;
    else
        settings.c_lflag &= ~ECHO;

    if (::tcsetattr(fd(), TCSAFLUSH, &settings) == -1)
        CC_SYSTEM_DEBUG_ERROR(errno);
}

int IoStream::ioctl(int request, void *arg)
{
    int value = ::ioctl(fd(), request, arg);

    if (value == -1)
        CC_SYSTEM_DEBUG_ERROR(errno);

    return value;
}

} // namespace cc
