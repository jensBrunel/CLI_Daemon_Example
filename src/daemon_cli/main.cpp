/**
 * @file main.cpp
 * @brief Simple CLI client for Daemon_Socket using Unix domain sockets.
 *
 * This executable connects to a Unix-domain socket (default: /var/run/Daemon_Socket)
 * and sends/receives newline-terminated messages. It supports one-shot sends via
 * `--send` and an interactive REPL mode.
 */

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

/**
 * @brief RAII wrapper for a Unix-domain socket connection to the daemon.
 */
class DaemonSocket {
public:
    /**
     * @brief Construct a new DaemonSocket object
     * @param path Filesystem path to the Unix-domain socket.
     */
    explicit DaemonSocket(std::string path) : sockfd_(-1), path_(std::move(path)) {}

    /**
     * @brief Destroy the DaemonSocket object, closing the socket if open.
     */
    ~DaemonSocket() { close_socket(); }

    /**
     * @brief Connect to the Unix-domain socket.
     * @param err Output parameter populated on failure with a human-readable message.
     * @return true if connection succeeded, false otherwise.
     */
    bool connect_socket(std::string &err) {
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
    bool send_message(const std::string &msg, std::string &err) {
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
    std::optional<std::string> receive_response(std::string &err) {
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

private:
    int sockfd_;
    std::string path_;

    /**
     * @brief Close the underlying socket if open.
     */
    void close_socket() {
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
    ssize_t write_all(const char *data, size_t len) {
        size_t total = 0;
        while (total < len) {
            ssize_t w = ::write(sockfd_, data + total, len - total);
            if (w <= 0) return -1;
            total += static_cast<size_t>(w);
        }
        return static_cast<ssize_t>(total);
    }
};

/**
 * @brief Print usage information for the CLI.
 * @param prog Program name (argv[0]).
 */
static void print_usage(const char *prog) {
    std::cerr << "Usage: " << prog << " [--socket PATH] [--send MESSAGE]\n";
    std::cerr << "If --send is omitted, enters interactive mode where each line is sent.\n";
}

/**
 * @brief Program entry point.
 *
 * Supported command-line options:
 * - `--socket PATH` : path to the daemon unix-domain socket (default `/var/run/Daemon_Socket`)
 * - `--send MESSAGE`: send a single MESSAGE and exit (one-shot mode)
 * - `-h`, `--help`   : print this help message
 */
int main(int argc, char **argv) {
    std::string socket_path = "/var/run/Daemon_Socket"; // default path
    std::optional<std::string> one_shot_msg;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--socket" && i + 1 < argc) {
            socket_path = argv[++i];
        } else if (a == "--send" && i + 1 < argc) {
            one_shot_msg = argv[++i];
        } else if (a == "-h" || a == "--help") {
            print_usage(argv[0]);
            return 0;
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    DaemonSocket client(socket_path);
    std::string err;
    if (!client.connect_socket(err)) {
        std::cerr << "Failed to connect to " << socket_path << ": " << err << "\n";
        return 2;
    }

    if (one_shot_msg.has_value()) {
        if (!client.send_message(*one_shot_msg, err)) {
            std::cerr << "Send failed: " << err << "\n";
            return 3;
        }
        auto resp = client.receive_response(err);
        if (!resp) {
            std::cerr << "Receive failed: " << err << "\n";
            return 4;
        }
        std::cout << *resp << std::endl;
        return 0;
    }

    // interactive mode
    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "quit" || line == "exit") break;
        if (!client.send_message(line, err)) {
            std::cerr << "Send failed: " << err << "\n";
            continue;
        }
        auto resp = client.receive_response(err);
        if (!resp) {
            std::cerr << "Receive failed: " << err << "\n";
            continue;
        }
        std::cout << *resp << std::endl;
    }

    return 0;
}
