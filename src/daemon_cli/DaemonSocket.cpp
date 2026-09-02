#include "DaemonSocket.h"

#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <rapidjson/document.h>

/**
 * @brief Destroy the DaemonSocket object, closing the socket if open.
 */
DaemonSocket::~DaemonSocket() { close_socket(); }

/**
 * @brief Connect to the Unix-domain socket.
 * @param strErr Output parameter populated on failure with a human-readable message.
 * @return true if connection succeeded, false otherwise.
 */
bool DaemonSocket::connect_socket(std::string &strErr) {
    m_iSockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_iSockfd == -1) {
        strErr = std::strerror(errno);
        return false;
    }

    struct sockaddr_un addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    if (m_strPath.size() >= sizeof(addr.sun_path)) {
        strErr = "socket path too long";
        ::close(m_iSockfd);
        m_iSockfd = -1;
        return false;
    }
    std::strncpy(addr.sun_path, m_strPath.c_str(), sizeof(addr.sun_path) - 1);

    if (connect(m_iSockfd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) == -1) {
        strErr = std::strerror(errno);
        ::close(m_iSockfd);
        m_iSockfd = -1;
        return false;
    }
    return true;
}

/**
 * @brief Send a newline-terminated message to the daemon.
 * @param strMsg Message to send (a trailing newline will be added if missing).
 * @param strErr Output parameter populated on failure.
 * @return true on success, false on error.
 */
bool DaemonSocket::send_message(const std::string &strMsg, std::string &strErr) {
    if (m_iSockfd == -1) {
        strErr = "not connected";
        return false;
    }
    std::string strOut = strMsg;
    if (strOut.empty() || strOut.back() != '\n') strOut.push_back('\n');

    ssize_t sszSent = write_all(strOut.data(), strOut.size());
    if (sszSent == -1) {
        strErr = std::strerror(errno);
        return false;
    }
    return true;
}

/**
 * @brief Receive a response from the daemon up to a newline (or socket close).
 * @param strErr Output parameter populated on failure.
 * @return optional string with the received response (without trailing newline), or
 *         std::nullopt on error.
 */
std::optional<std::string> DaemonSocket::receive_response(std::string &strErr) {
    if (m_iSockfd == -1) {
        strErr = "not connected";
        return std::nullopt;
    }

    std::string strResp;
    char acBuf[1024];
    ssize_t sszRead = 0;
    // read until newline or socket close
    while ((sszRead = ::read(m_iSockfd, acBuf, sizeof(acBuf))) > 0) {
        strResp.append(acBuf, acBuf + sszRead);
        if (strResp.find('\n') != std::string::npos) break;
    }
    if (sszRead == -1) {
        strErr = std::strerror(errno);
        return std::nullopt;
    }
    // strip trailing newline
    if (!strResp.empty() && strResp.back() == '\n') strResp.pop_back();
    return strResp;
}

/**
 * @brief Close the underlying socket if open.
 */
void DaemonSocket::close_socket() {
    if (m_iSockfd != -1) {
        ::close(m_iSockfd);
        m_iSockfd = -1;
    }
}

/**
 * @brief Write the entire buffer to the socket, handling partial writes.
 * @param pcData Pointer to data buffer.
 * @param szLen Length of buffer in bytes.
 * @return number of bytes written on success, -1 on error.
 */
ssize_t DaemonSocket::write_all(const char *pcData, size_t szLen) {
    size_t szTotal = 0;
    while (szTotal < szLen) {
        ssize_t sszWritten = ::write(m_iSockfd, pcData + szTotal, szLen - szTotal);
        if (sszWritten <= 0) return -1;
        szTotal += static_cast<size_t>(sszWritten);
    }
    return static_cast<ssize_t>(szTotal);
}
