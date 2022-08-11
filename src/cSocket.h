#include <string>
#include <WinSock2.h>
#include <wex.h>
#include "tcp.h"

namespace wex {
class cSocket
{
public:
    cSocket() : myWindow(maker::make()),
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
    /** Start server
     * @param[in] port to listen for clients
     * @param[in] connectHandler event handler to call when client connects
     * @param[in] readHandler event handler to call when client sands a message
     */
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

    /// Connect to server
    void client(const std::string &ipaddr, const std::string &port)
    {
        myTCP.client(ipaddr, port);
    }

    /// Send message to connected peer
    void send(const std::string &msg)
    {
        myTCP.send(msg);
    }

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
    void run()
    {
        myWindow.run();
    }

private:
    gui &myWindow;
    tcp myTCP;
    std::function<void(std::string &port)> myConnectHandler;
    std::function<void(std::string &port, const std::string &msg)> myReadHandler;
    std::string myPort;
};
}