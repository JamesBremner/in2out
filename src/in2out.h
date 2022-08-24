#include "tcp.h"

class cIn2Out
{
public:
    /// CTOR to parse command line options and connect sockets
    cIn2Out(int ac, char **av);

    /// CTOR to run tests
    cIn2Out();

    /// parse command line options for TCP addresses
    void ParseOptionsNoBoost(int ac, char **av);

    /// connect the input and output sockets
    void connect();

    /// handle some input
    void input(const std::string &msg);

    /** recover from framing errors
     * @param[in] msg most recent message
     * @return vector of complete lines received
     */
    std::vector<std::string> frameCheck(const std::string &msg);

    /// process a line of input
    std::string Process(const std::string &input);

    /** Start the windex event handler
     *
     * This blocks!
     *
     * Call this once when everything has been setup
     *
     */
    void run();

    /// Enable or disable frame checking
    void frameCheck(bool f)
    {
        myframeCheck = f;
    }

private:
    int myInputPort;
    std::string myOutputIP;
    int myOutputPort;
    bool myframeCheck;
    std::string myBacklog;

    wex::cSocket myTCPinput;
    wex::cSocket myTCPoutput;

    std::thread *myKeyboardThread;

    // run some unit tests
    void test();

    /** Monitor input on the keyboard
     * 
     * Blocks - so run this in its own thread
     * If "exit<enter>" is typed this will exit the app.
     */
    void keyboardmonitor();

    /** Attempt connection to output server - the listener
     * 
     * Asynchronous - this returns immediatly.
     * At some later moment the connection may or may not be made
     * The connection status will be discovered 
     * when the next write output is run - don't do so immediatly!
     */
    void connectOutputServer();

    /** Handle loss of server connection
     * 
     * - display message
     * - discard any data waiting to be output
     * - start new connection attempt
     */
    void noServerHandler();

    /// Display input message as text and as hexadecimal bytes
    void inputDisplay(const std::string &msg);

    // Count line waiting to be output, not including partial
    int countLines() const;

    /** Check for last data line in a package
     * @param[in] line1 the line to test
     * @param[in] line2 the line that will be output after line1
     * @return true if line1 is the last data line of a package
     * 
     * line1 is the last data line in a package
     * if it is a data line and line is not.
     * 
     * Details: https://github.com/JamesBremner/in2out/wiki/Input
     */
    bool isLastData(
        const std::string &line1,
        const std::string &line2) const;
};
