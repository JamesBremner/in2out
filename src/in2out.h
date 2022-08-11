#include <string>
#include <wex.h>
#include "tcp.h"

class cSocket
{
public:
    cSocket() : myWindow(wex::maker::make()),
                myTCP(&myWindow),
                myConnectHandler([](std::string port) {})
    {
        //  when a client connects, setup to read anything sent
        myWindow.events()
            .tcpServerAccept(
                [this]
                {
                    myConnectHandler(myPort);
                    myTCP.read();
                });

        //  handle data received on input
        myWindow.events().tcpRead(
            [this]
            {
                // check for connection closed
                if (!myTCP.isConnected())
                {
                    std::cout << "Input Connection closed, waiting for new client\n";

                    server(
                        myPort,
                        myConnectHandler,
                        myReadHandler);
                    return;
                }

                myReadHandler(
                    myPort,
                    myTCP.readMsg());

                // setup for next message
                myTCP.read();
            });
    }
    void server(
        const std::string &port,
        std::function<void(std::string &port)> connectHandler,
        std::function<void(std::string &port, const std::string &msg)> readHandler)
    {
        myPort = port;
        myConnectHandler = connectHandler;
        myReadHandler = readHandler;
        myTCP.server(port);
    }

    void client(const std::string &ipaddr, const std::string &port)
    {
        myTCP.client(ipaddr, port);
    }

    void send(const std::string &msg)
    {
        myTCP.send(msg);
    }

private:
    wex::gui &myWindow;
    wex::tcp myTCP;
    std::function<void(std::string &port)> myConnectHandler;
    std::function<void(std::string &port, const std::string &msg)> myReadHandler;
    std::string myPort;
};

class cIn2Out
{
public:
    // cIn2Out();

    /// parse command line options for TCP addresses
    void ParseOptions(int ac, char **av);

    /// connect the input and output sockets
    void connect();

    /// handle some input
    void input(const std::string &msg);

    /// process a line of onput
    std::string Process(const std::string &input);

private:
    int myInputPort;
    std::string myOutputIP;
    int myOutputPort;

    cSocket myTCPinput;
    cSocket myTCPoutput;

};
