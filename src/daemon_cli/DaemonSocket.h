#include <cstddef>
#include <optional>
#include <string>
#include <sys/types.h>
#include <utility>

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
    ~DaemonSocket();

    /**
     * @brief Connect to the Unix-domain socket.
     * @param err Output parameter populated on failure with a human-readable message.
     * @return true if connection succeeded, false otherwise.
     */
    bool connect_socket(std::string &err);

    /**
     * @brief Send a newline-terminated message to the daemon.
     * @param msg Message to send (a trailing newline will be added if missing).
     * @param err Output parameter populated on failure.
     * @return true on success, false on error.
     */
    bool send_message(const std::string &msg, std::string &err);

    /**
     * @brief Receive a response from the daemon up to a newline (or socket close).
     * @param err Output parameter populated on failure.
     * @return optional string with the received response (without trailing newline), or
     *         std::nullopt on error.
     */
    std::optional<std::string> receive_response(std::string &err);

private:
    int sockfd_;
    std::string path_;

    /**
     * @brief Close the underlying socket if open.
     */
    void close_socket();

    /**
     * @brief Write the entire buffer to the socket, handling partial writes.
     * @param data Pointer to data buffer.
     * @param len Length of buffer in bytes.
     * @return number of bytes written on success, -1 on error.
     */
    ssize_t write_all(const char *data, size_t len);
};