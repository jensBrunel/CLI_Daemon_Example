#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include "DaemonSocket.h"

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
     * @return 0 for help, 1 for invalid arguments, and -1 when parsing succeeded.
     */
    static int ParseArgs(int iArgc, char **ppszArgv, std::string &strSocketPath);

    /**
     * @brief Construct a parser bound to a daemon socket path.
     * @param strSocketPath Unix domain socket path used to talk to the daemon.
     */
    explicit CommandParser(std::string strSocketPath);

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
    DaemonSocket m_socket;
};

#endif // COMMANDPARSER_H
