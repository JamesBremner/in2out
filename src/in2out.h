#include "cSocket.h"

class cIn2Out
{
public:
    /// CTOR to parse command line options and connect sockets
    cIn2Out(int ac, char **av);

    /// CTOR to run tests
    cIn2Out();

    /// parse command line options for TCP addresses
    void ParseOptions(int ac, char **av);

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

    /// input server config
    int myInputPort;

    /// listerner server config
    std::string myOutputIP;
    int myOutputPort;

    /// flag, true if framing error recovery required
    bool myframeCheck;

    /// socket wrappers
    wex::cSocket myTCPinput;
    wex::cSocket myTCPoutput;

    /// thread watching for keyboard input
    std::thread* myKeyboardThread;

    /** Buffer containing unmodified data waiting to be sent
     * Data may be waiting because:
     * - a partial line arrived and is waiting for the rest of the line
     * - several lines arrived in one message and lines are waiting for previous lines to be sent
     * - data may have arrived when the listener was disconnected
     */
    std::string myWaiting;
    std::mutex myWaitingMutex;

    void test();
    void keyboardmonitor();
    void connectOutputServer();

    /// Thread safe acces to waiting to be sent
    void waitFlush();
    void waitAddFront( const std::string& s);
    void waitAddEnd( const std::string& s);

};
