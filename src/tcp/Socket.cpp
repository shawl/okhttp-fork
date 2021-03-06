//
// Created by Good_Pudge.
//

#include "../SocketImpl.hpp"
#include <ohf/tcp/Socket.hpp>

#ifdef OKHTTPFORK_UNIX
    #define OHF_FLAGS MSG_NOSIGNAL
#else
    #define OHF_FLAGS 0
#endif

namespace ohf {
    namespace tcp {
        Socket::Socket() : ohf::Socket(Type::TCP) {}

        Socket::Socket(tcp::Socket&& socket) noexcept : tcp::Socket() {
            mFD = socket.mFD;
            socket.mFD = SocketImpl::invalidSocket();

            mBlocking = socket.mBlocking;
            socket.mBlocking = true;
        }

        void Socket::connect(const InetAddress &address, Uint16 port) {
            create(address.family());

            bool is_blocking = isBlocking();
            if (!is_blocking) blocking(true);

            SocketImpl::SocketLength length;
            sockaddr_storage socket_address = SocketImpl::createAddress(address, port, length);
            if (::connect(mFD, (sockaddr *) &socket_address, length) == -1) {
                throw Exception(Exception::Code::FAILED_TO_CREATE_CONNECTION,
                                "Failed to create connection: " + SocketImpl::getError());
            }

            blocking(is_blocking);
        }

        void Socket::connect(Family family, const HttpURL &url) {
            connect(InetAddress(url.host(), family), url.port());
        }

        void Socket::disconnect() {
            close();
        }

        Int32 Socket::send(const char *data, Int32 size) const {
            if (!data || size == 0) throw Exception(Exception::Code::NO_DATA_TO_SEND, "No data to send: ");

            Int32 sent = ::send(mFD, data, size, OHF_FLAGS);
            if (sent < 0) {
                throw Exception(Exception::Code::FAILED_TO_SEND_DATA,
                        "Failed to send data: " + SocketImpl::getError());
            }

            return sent;
        }

        Int32 Socket::send(const std::vector<Int8> &data) const {
            return send(data.data(), data.size());
        }

        Int32 Socket::send(const std::string &data) const {
            return send(data.data(), data.size());
        }

        Int32 Socket::receive(char *data, Int32 size) const {
            Int32 received = recv(mFD, data, size, OHF_FLAGS);
            if(received < 0) {
                throw Exception(Exception::Code::FAILED_TO_RECEIVE_DATA,
                                "Failed to receive data: " + SocketImpl::getError());
            }
            return received;
        }

        Int32 Socket::receive(std::vector<Int8> &data, Int32 size) const {
            data.clear();
            data.resize(size);
            Int32 received = receive(data.data(), size);
            data.resize(received);
            data.shrink_to_fit();
            return received;
        }

        Int32 Socket::receive(std::string &data, Int32 size) const {
            data.clear();
            data.resize(size);
            Int32 received = receive(&data[0], size);
            data.resize(received);
            data.shrink_to_fit();
            return received;
        }

        Socket& Socket::operator =(tcp::Socket&& right) noexcept {
            mFD = right.mFD;
            right.mFD = SocketImpl::invalidSocket();

            mBlocking = right.mBlocking;
            right.mBlocking = true;

            return *this;
        }
    }
}

namespace std {
    using namespace ohf;

    void swap(tcp::Socket& a, tcp::Socket& b) {
        std::swap(a.mFD, b.mFD);
        std::swap(a.mBlocking, b.mBlocking);
    }
}
