#include <string>
#include <wex.h>
#include "tcp.h"
class cIn2Out
{
public:
    cIn2Out();

    /// parse command line options for TCP addresses
    void ParseOptions(int ac, char **av);

    /// connect the input and output sockets
    void connect();

    /// handle some input
    void input();

    /// process a line of onput
    std::string Process( const std::string& input );

private:
    int myInputPort;
    std::string myOutputIP;
    int myOutputPort;

    wex::gui &myfmInput;
    wex::gui &myfmOutput;

    wex::tcp &myTCPinput;
    wex::tcp &myTCPoutput;

};
