#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include "DaemonSocket.h"
#include "IniConfig.h"

class CommandParser {
public:
    /**
     * @brief Print usage information for the CLI.
     * @param pszProg Program name passed as argv[0].
     */
    static void PrintUsage(const char *pszProg);

    /**
     * @brief Parse command-line arguments and select the daemon socket path.
     * @param iArgc Number of arguments.
     * @param ppszArgv Argument vector.
     * @param strSocketPath Output socket path.
     * @param strIniPath Output INI file path.
     * @return 0 for help, 1 for invalid arguments, and -1 when parsing succeeded.
     */
    static int ParseArgs(int iArgc, char **ppszArgv, std::string &strSocketPath, std::string &strIniPath);

    /**
     * @brief Resolve the daemon socket path from an optional INI file.
     * @param strSocketPath Current socket path.
     * @param strIniPath INI file path, or empty to skip config lookup.
     * @return Final socket path after applying the INI override.
     */
    static std::string ResolveSocketPath(const std::string &strSocketPath, const std::string &strIniPath);

    /**
     * @brief Load the socket path from an INI file using IniConfig.
     * @param strIniPath Path to the INI file.
     * @return The socket path from SOCKET_PATH if present, otherwise the default path.
     */
    static std::string LoadSocketPathFromIni(const std::string &strIniPath);

    /**
     * @brief Construct a parser from raw CLI arguments and resolve the final socket path.
     * @param iArgc Number of arguments.
     * @param ppszArgv Argument vector.
     */
    explicit CommandParser(int iArgc, char **ppszArgv);

    /**
     * @brief Construct a parser bound to a daemon socket path.
     * @param strSocketPath Unix domain socket path used to talk to the daemon.
     */
    explicit CommandParser(std::string strSocketPath);

    /**
     * @brief Query whether the parser was initialized successfully.
     * @return true when the arguments are valid, false for help or invalid usage.
     */
    bool IsValid() const;

    /**
     * @brief Get the exit code for the current argument parse state.
     * @return 0 for help, 1 for invalid arguments, and -1 for normal operation.
     */
    int ExitCode() const;

    /**
     * @brief Get the resolved socket path currently in use.
     * @return Socket path string.
     */
    const std::string &SocketPath() const;

    /**
     * @brief Set the raw command text to be parsed and executed.
     * @param strInput Input line entered by the user.
     */
    void SetInput(std::string strInput);

    /**
     * @brief Connect the internal socket to the daemon.
     * @param strErr Output buffer receiving the error description on failure.
     * @return true if the connection succeeded, false otherwise.
     */
    bool Connect(std::string &strErr);

    /**
     * @brief Split the current input into tokens.
     * @return Vector containing the parsed command arguments.
     */
    std::vector<std::string> Parse() const;

    /**
     * @brief Get the raw input line as originally stored.
     * @return The unmodified command string.
     */
    std::string Raw() const;

    /**
     * @brief Send the current command to the daemon and read its response.
     * @param strErr Output buffer receiving any send/receive failure description.
     * @return Optional response string from the daemon, or std::nullopt on failure.
     */
    std::optional<std::string> Execute(std::string &strErr);

    /**
     * @brief Process one entered command line end-to-end.
     * @param strErr Output buffer receiving a detailed error if execution fails.
     * @param out Stream used for writing the daemon response or a failure message.
     * @return false when the command is quit/exit, true otherwise.
     */
    bool HandleInput(std::string &strErr, std::ostream &out);

private:
    std::string m_strInput;
    std::string m_strSocketPath;
    DaemonSocket m_socket;
    bool m_bValid;
    int m_iExitCode;
};

#endif // COMMANDPARSER_H
