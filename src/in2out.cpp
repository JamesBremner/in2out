#include <string>
#include <sstream>
#include <iostream>
#include <vector>

#include <boost/program_options.hpp>

#include "in2out.h"

cIn2Out::cIn2Out(int ac, char **av) : myframeCheck(false)
{
    // start keyboard monitor for exit command
    myKeyboardThread = new std::thread(
        keyboardmonitor, this);

    // parse command line
    ParseOptions(ac, av);

    // start the sockets
    connect();
}
cIn2Out::cIn2Out()
{
    test();
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

void cIn2Out::connect()
{
    // wait for connection on input
    try
    {
        myTCPinput.server(
            std::to_string(myInputPort),
            [&](std::string &port)
            {
                std::cout << "Input connected on " << port << "\n";
            },
            [&](std::string &port, const std::string &msg)
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
    connectOutputServer();
}

void cIn2Out::connectOutputServer()
{
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

void cIn2Out::run()
{
    try
    {
        myTCPinput.run();
    }
    catch (std::runtime_error &e)
    {
        std::cout << "cIn2Out::run: " << e.what();
    }
}

void cIn2Out::input(const std::string &msg)
{
    std::cout << "Input: " + msg << "\n";
    std::cout << "Input Hex: ";
    for (int k = 0; k < msg.size(); k++)
    {
        std::cout << std::hex << (int)msg[k] << " ";
    }
    std::cout << "\n\n";

    // Loop over complete lines
    for (auto &line : frameCheck(msg))
    {
        // modify message
        auto mod = Process(line);
        std::cout << "Output " + mod << "\n";

        // send message to output
        if( !myTCPoutput.isConnected() )
        {
            std::cout << "no-one is listening\n";
            connectOutputServer();
            continue;
        }
        myTCPoutput.send(mod);
    }
}

std::vector<std::string> cIn2Out::frameCheck(const std::string &msg)
{
    std::vector<std::string> output;
    static std::string partial;

    if (!myframeCheck)
    {
        output.push_back(msg);
        return output;
    }

    partial += msg;
    int p = partial.find("\n");
    if (p == -1)
        return output;
    while (p != -1)
    {
        output.push_back(partial.substr(0, p));
        partial = partial.substr(p + 1);
        p = partial.find("\n");
    }
    return output;
}

// handle some keyboard input
void cIn2Out::keyboardmonitor()
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

void cIn2Out::test()
{
    frameCheck(true);

    std::vector<std::pair<std::string, int>> vt{
        {"test1\n", 1},
        {"test2\n", 1},
        {"combined\ntest3\n", 2},
        {"partial", 0},
        {" test4\n", 1}};
    for (auto &t : vt)
    {
        if (frameCheck(t.first).size() != t.second)
        {
            std::cout << "Failed test " << t.first
                      << " => " << frameCheck(t.first).size()
                      << "\n";
            exit(1);
        }
    }
}
main(int argc, char *argv[])
{
    std::cout << "in2out built " + std::string(__DATE__) + " " + std::string(__TIME__) + "\n";

    // run tests
    cIn2Out test;

    // start sockets
    cIn2Out in2out(argc, argv);

    // enable frame checking
    in2out.frameCheck(true);

    // start event handler
    in2out.run();

    return 0;
}
