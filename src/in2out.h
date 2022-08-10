#include <string>
class cIn2Out
{
public:
    cIn2Out();

    /// parse command line options for TCP addresses
    void ParseOptions(int ac, char **av);

    int inputPort() const
    {
        return myInputPort;
    }
    std::string sInputPort() const
    {
        return std::to_string(myInputPort);
    }
    std::string outputIP() const
    {
        return myOutputIP;
    }
    std::string sOutputPort() const
    {
        return std::to_string(myOutputPort);
    }

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
