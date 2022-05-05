#include <sese/net/TcpServer.h>
#include <ctime>
#include <unistd.h>
#include <fcntl.h>
#include <sese/record/LogHelper.h>

using sese::IOContext;
using sese::IPAddress;
using sese::Socket;
using sese::TcpServer;

IOContext::IOContext() noexcept {}

IOContext::~IOContext() noexcept {}

int64_t IOContext::recv(void *buffer, size_t size) noexcept {
    int32_t count = 0;
    int64_t transfer = 0;
    int64_t len;
    while (true) {
        if (size >= 1024) {
            len = socket->read(buffer, 1024);
        } else {
            len = socket->read(buffer, size);
        }

        if (len <= 0) {
            return -1;
        } else {
            transfer += len;
            size -= len;
            if (0 == size) {
                break;
            }
        }

        count += 1;
        if (count == 64) {
            return transfer;
        }
    }
    return transfer;
}

int64_t IOContext::send(const void *buffer, size_t size) noexcept {
    int32_t count = 0;
    int64_t transfer = 0;
    int64_t len;
    while (true) {
        if (size >= 1024) {
            len = socket->write(buffer, 1024);
        } else {
            len = socket->write(buffer, size);
        }

        if (len <= 0) {
            return -1;
        } else {
            transfer += len;
            size -= len;
            if (0 == size) {
                break;
            }
        }

        count += 1;
        if (count == 64) {
            return transfer;
        }
    }
    return transfer;
}

int32_t IOContext::shutdown(Socket::ShutdownMode mode) const noexcept {
    return socket->shutdown(mode);
}

void IOContext::close() const noexcept {
    socket->close();
}

const IPAddress::Ptr &IOContext::getClientAddress() const noexcept {
    return reinterpret_cast<const IPAddress::Ptr &>(socket->getAddress());
}

bool TcpServer::init(const IPAddress::Ptr &ipAddress, size_t threads) noexcept {
    auto family = ipAddress->getRawAddress()->sa_family;
    socket_t serverSocket = ::socket(family, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == -1) {
        return false;
    }

    socket = std::make_shared<Socket>(serverSocket, ipAddress);

    if (!socket->setNonblocking(true)) {
        return false;
    }

    if (-1 == socket->bind(ipAddress)) {
        socket->close();
        return false;
    }

    if (-1 == socket->listen(SERVER_MAX_CONNECTION)) {
        socket->close();
        return false;
    }

    kqueueFd = kqueue();
    if (kqueueFd == -1) {
        socket->close();
        return false;
    }

    struct kevent event {};
    EV_SET(&event, serverSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
    if (-1 == kevent(kqueueFd, &event, 1, nullptr, 0, nullptr)) {
        socket->close();
        return false;
    }

    threadPool = std::make_shared<ThreadPool>("TcpServer", threads);
    isShutdown = false;
    return true;
}

sese::LogHelper helper("loop"); // NOLINT

void TcpServer::loopWith(const std::function<void(IOContext *)> &handler) {
    struct timespec timeout {
        1, 0
    };

    while (!isShutdown) {
        int32_t nev = kevent(kqueueFd, nullptr, 0, events, 64, &timeout);
        for (int32_t n = 0; n < nev; n++) {
            if (events[n].ident == socket->getRawSocket()) {
                socket_t client = accept(socket->getRawSocket(), nullptr, nullptr);
                if (client == -1) {
                    continue;
                }

                int32_t opt = fcntl(client, F_GETFL);
                if (opt == -1) {
                    close(client);
                    continue;
                }
                if (fcntl(client, F_SETFL, opt | O_NONBLOCK) != 0) {
                    close(client);
                    continue;
                }

                struct kevent event {};
                EV_SET(&event, client, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, nullptr);
                if (-1 == kevent(kqueueFd, &event, 1, nullptr, 0, nullptr)) {
                    socket->close();
                    continue;
                }
            } else {
                auto fd = events[n].ident;
                sockaddr_storage sockaddrStorage{};
                socklen_t socklen = sizeof(sockaddrStorage);
                getpeername(fd, reinterpret_cast<struct sockaddr *>(&sockaddrStorage), &socklen);
                Address::Ptr address = Address::create(reinterpret_cast<struct sockaddr *>(&sockaddrStorage), socklen);

                Socket::Ptr client = std::make_shared<Socket>(fd, address);
                auto *ioContext = new IOContext;
                ioContext->socket = client;
                threadPool->postTask([handler, ioContext]() {
                    handler(ioContext);
                    delete ioContext;
                });
            }
        }
    }
}

void TcpServer::shutdown() {
    isShutdown = true;
    threadPool->shutdown();
    close(kqueueFd);
    socket->close();
}