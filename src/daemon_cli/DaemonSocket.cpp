#include "DaemonSocket.h"

#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

    /**
     * @brief Destroy the DaemonSocket object, closing the socket if open.
     */
    DaemonSocket::~DaemonSocket() { close_socket(); }

    /**
     * @brief Connect to the Unix-domain socket.
     * @param err Output parameter populated on failure with a human-readable message.
     * @return true if connection succeeded, false otherwise.
     */
    bool DaemonSocket::connect_socket(std::string &err) {
        sockfd_ = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sockfd_ == -1) {
            err = std::strerror(errno);
            return false;
        }

        struct sockaddr_un addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        if (path_.size() >= sizeof(addr.sun_path)) {
            err = "socket path too long";
            ::close(sockfd_);
            sockfd_ = -1;
            return false;
        }
        std::strncpy(addr.sun_path, path_.c_str(), sizeof(addr.sun_path) - 1);

        if (connect(sockfd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == -1) {
            err = std::strerror(errno);
            ::close(sockfd_);
            sockfd_ = -1;
            return false;
        }
        return true;
    }

    /**
     * @brief Send a newline-terminated message to the daemon.
     * @param msg Message to send (a trailing newline will be added if missing).
     * @param err Output parameter populated on failure.
     * @return true on success, false on error.
     */
    bool DaemonSocket::send_message(const std::string &msg, std::string &err) {
        if (sockfd_ == -1) {
            err = "not connected";
            return false;
        }
        std::string out = msg;
        if (out.empty() || out.back() != '\n') out.push_back('\n');

        ssize_t sent = write_all(out.data(), out.size());
        if (sent == -1) {
            err = std::strerror(errno);
            return false;
        }
        return true;
    }

    /**
     * @brief Receive a response from the daemon up to a newline (or socket close).
     * @param err Output parameter populated on failure.
     * @return optional string with the received response (without trailing newline), or
     *         std::nullopt on error.
     */
    std::optional<std::string> DaemonSocket::receive_response(std::string &err) {
        if (sockfd_ == -1) {
            err = "not connected";
            return std::nullopt;
        }

        std::string resp;
        char buf[1024];
        ssize_t n = 0;
        // read until newline or socket close
        while ((n = ::read(sockfd_, buf, sizeof(buf))) > 0) {
            resp.append(buf, buf + n);
            if (resp.find('\n') != std::string::npos) break;
        }
        if (n == -1) {
            err = std::strerror(errno);
            return std::nullopt;
        }
        // strip trailing newline
        if (!resp.empty() && resp.back() == '\n') resp.pop_back();
        return resp;
    }

    /**
     * @brief Close the underlying socket if open.
     */
    void DaemonSocket::close_socket() {
        if (sockfd_ != -1) {
            ::close(sockfd_);
            sockfd_ = -1;
        }
    }

    /**
     * @brief Write the entire buffer to the socket, handling partial writes.
     * @param data Pointer to data buffer.
     * @param len Length of buffer in bytes.
     * @return number of bytes written on success, -1 on error.
     */
    ssize_t DaemonSocket::write_all(const char *data, size_t len) {
        size_t total = 0;
        while (total < len) {
            ssize_t w = ::write(sockfd_, data + total, len - total);
            if (w <= 0) return -1;
            total += static_cast<size_t>(w);
        }
        return static_cast<ssize_t>(total);
    }
