#include <string>
class cIn2Out
{
public:
    cIn2Out();
    void ParseOptions(int ac, char **av);
    int inputPort() const
    {
        return myInputPort;
    }
    std::string sInputPort() const
    {
        return std::to_string(myInputPort);
    }
    std::string Process( const std::string& input );

private:
    int myInputPort;
    std::string myOutputIP;
    int myOutputPort;
};
