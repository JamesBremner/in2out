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
#include "in2out.h"

class cGUI : public cStarterGUI
{
public:
    cGUI(int ac, char **av);

private:
    wex::timer *myStartTimer;
    cIn2Out in2out;
};

// cIn2Out::cIn2Out() : myInputPort(-1), myOutputPort(-1)
// {
// }

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
        myOutputIP = s.substr(0, p);
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

cGUI::cGUI(int ac, char **av)
    : cStarterGUI(
          "Starter",
          {50, 50, 1000, 500})
{
    in2out.ParseOptions(ac, av);

    // delay connection attempts until windows setup complete
    myStartTimer = new wex::timer(fm, 1000);
    fm.events().timer(
        [this](int id)
        {
            delete myStartTimer;
            in2out.connect();
        });

    // setup the windows
    run();
}

void cIn2Out::connect()
{
    // wait for connection on input
    try
    {
        myTCPinput.server(
            std::to_string(myInputPort),
            [&](std::string & port)
                {
                    std::cout << "Input connected on " << port << "\n";
                },
            [&](std::string & port, const std::string &msg)
                {
                    input(msg);
                });
        std::cout << "Waiting for input on port "
                  << myInputPort << "\n";
    }
    catch (std::runtime_error &e)
    {
        std::cout << "Cannot start input server " << e.what() << "\n";
    }

    // attempt connection to server for output
    try
    {
        std::cout << "looking for output server "
                  << myOutputIP << ":" << myOutputPort << "\n";
        myTCPoutput.client(
            myOutputIP,
            std::to_string(myOutputPort));
    }
    catch (std::runtime_error &e)
    {
        std::cout << "Cannot connect to output server " << e.what();
    }
}

void cIn2Out::input(const std::string& msg)
{

    std::cout << "Input: " + msg << "\n";

    // modify message
    auto mod = Process(msg);
    std::cout << "Output " + mod << "\n";

    // send message to output
    myTCPoutput.send(mod);
}

// handle some keyboard input
void keyboardmonitor()
{
    std::string myString;
    while (true)
    {
        std::cin >> myString;

        // check for stop request
        if (myString == "exit" ||
            myString == "EXIT")
        {
            exit(0);
        }
    }
}

main(int argc, char *argv[])
{
    std::thread t(keyboardmonitor);

    cGUI theGUI(argc, argv);

    return 0;
}
