#include "cSocket.h"

class cIn2Out
{
public:
     cIn2Out(int ac, char **av);

    /// parse command line options for TCP addresses
    void ParseOptions(int ac, char **av);

    /// connect the input and output sockets
    void connect();

    /// handle some input
    void input(const std::string &msg);

    /// process a line of input
    std::string Process(const std::string &input);

private:
    int myInputPort;
    std::string myOutputIP;
    int myOutputPort;

    wex::cSocket myTCPinput;
    wex::cSocket myTCPoutput;

};
