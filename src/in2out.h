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
     * This is used by console type applications.
     * GUI applications should not call this
     * They will call run on the main application window when setup is complete
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

    wex::cSocket myTCPinput;
    wex::cSocket myTCPoutput;

    std::thread* myKeyboardThread;

    void test();
    void keyboardmonitor();
    void connectOutputServer();
};
