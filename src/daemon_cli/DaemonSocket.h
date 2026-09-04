#ifndef DAEMONSOCKET_H
#define DAEMONSOCKET_H

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
     * @param strPath Filesystem path to the Unix-domain socket.
     */
    explicit DaemonSocket(std::string strPath) : m_iSockfd(-1), m_strPath(std::move(strPath)) {}

    /**
     * @brief Destroy the DaemonSocket object, closing the socket if open.
     */
    ~DaemonSocket();

    /**
     * @brief Connect to the Unix-domain socket.
     * @param strErr Output parameter populated on failure with a human-readable message.
     * @return true if connection succeeded, false otherwise.
     */
    bool connect_socket(std::string &strErr);

    /**
     * @brief Send a newline-terminated message to the daemon.
     * @param strMsg Message to send (a trailing newline will be added if missing).
     * @param strErr Output parameter populated on failure.
     * @return true on success, false on error.
     */
    bool send_message(const std::string &strMsg, std::string &strErr);

    /**
     * @brief Receive a response from the daemon up to a newline (or socket close).
     * @param strErr Output parameter populated on failure.
     * @return optional string with the received response (without trailing newline), or
     *         std::nullopt on error.
     */
    std::optional<std::string> receive_response(std::string &strErr);

private:
    int m_iSockfd;
    std::string m_strPath;

    /**
     * @brief Close the underlying socket if open.
     */
    void close_socket();

    /**
     * @brief Write the entire buffer to the socket, handling partial writes.
     * @param pcData Pointer to data buffer.
     * @param szLen Length of buffer in bytes.
     * @return number of bytes written on success, -1 on error.
     */
    ssize_t write_all(const char *pcData, size_t szLen);
};

#endif // DAEMONSOCKET_H
