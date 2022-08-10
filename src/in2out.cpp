#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <boost/program_options.hpp>
#include <wex.h>
#include "tcp.h"
#include "cStarterGUI.h"

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
class cGUI : public cStarterGUI
{
public:
    cGUI(int ac, char **av)
        : cStarterGUI(
              "Starter",
              {50, 50, 1000, 500}),
          myTCPinput(wex::maker::make<wex::tcp>(fm))
    {
        in2out.ParseOptions(ac, av);

        myStartTimer = new wex::timer(fm, 1000);
        fm.events().timer(
            [this](int id)
            {
                delete myStartTimer;
                connect();
            });

        fm.events()
            .tcpServerAccept(
                [this]
                {
                    std::cout << "Input source connected\n";
                    myTCPinput.read();
                });

        fm.events().tcpRead(
            [this]
            {
                input();
            });

        run();
    }

private:
    wex::tcp &myTCPinput;
    wex::timer *myStartTimer;
    cIn2Out in2out;

    void connect();
    void input();
};

cIn2Out::cIn2Out() : myInputPort(-1), myOutputPort(-1)

{
}

void cIn2Out::ParseOptions(int ac, char **av)
{
    namespace po = boost::program_options;

    // Declare the supported options.
    po::options_description desc("Command line options\nOmmitted options will be read from file P4WRSConfig.txt");
    desc.add_options()("help", "produce help message\n")("input", po::value<int>()->default_value(5000), "Port to listen for input")("output", po::value<std::string>()->default_value("127.0.0.1:5001"), "IP address and port to send output");

    po::variables_map vm;

    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);

    if (ac == 1 || vm.count("help"))
    {
        std::cout << "\n"
                  << desc << "\n";
        exit(0);
    }
    if (vm.count("input"))
        myInputPort = vm["input"].as<int>();
    if (vm.count("output"))
    {
        auto s = vm["output"].as<std::string>();
        int p = s.find(":");
        if (p == -1)
        {
            std::cout << "\n"
                      << desc << "\n";
            exit(0);
        }
        myOutputIP = s.substr(0, p - 1);
        myOutputPort = atoi(s.substr(p + 1).c_str());
    }

    if (myInputPort == -1 ||
        myOutputPort == -1 ||
        myOutputIP == "")
    {
        std::cout << "\n"
                  << desc << "\n";
        exit(0);
    }
}

void cGUI::connect()
{
    try
    {
        myTCPinput.server(in2out.sInputPort());
        std::cout << "Waiting for input on port "
                  << in2out.inputPort() << "\n";
    }
    catch (std::runtime_error &e)
    {
        std::cout << "Cannot start server " << e.what() << "\n";
    }

    // else
    // {
    //     try
    //     {
    //         myTCP.client();
    //     }
    //     catch (std::runtime_error &e)
    //     {
    //         status(std::string("Cannot connect to server ") + e.what());
    //     }
    // }
}

void cGUI::input()
{
    if (!myTCPinput.isConnected())
    {
        std::cout << "Input Connection closed, waiting for new client\n";

        myTCPinput.server(in2out.sInputPort());
        return;
    }
    auto msg = myTCPinput.readMsg();

    std::cout << "Input: " + myTCPinput.readMsg() << "\n\n";

    std::cout << "Output " + in2out.Process( msg ) << "\n";

    // setup for next message
    myTCPinput.read();
}

main(int argc, char *argv[])
{

    cGUI theGUI(argc, argv);

    return 0;
}
