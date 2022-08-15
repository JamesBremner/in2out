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

        // attempt connect just once
        myTCPoutput.RetryConnectServer(false);

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

    // add to end og witing buffer
    waitAddEnd(msg);

    // check server is connected
    if (!myTCPoutput.isConnected())
    {
        std::cout << "no-one is listening\n";

        // attempt new connection to listener
        connectOutputServer();

        // at this point we do not know wether or not the connect to server will succeed
        // the message is safely stored in the waiting buffer
        // and will be sent when the next input arrives if connection was succesful
        return;
    }

    // Loop over complete lines
    auto completeLines = frameCheck(std::string(""));
    for (auto &line : completeLines)
    {
        // modify message
        auto mod = Process(line);
        std::cout << "Output " + mod << "\n";

        // check server is still connected
        if (!myTCPoutput.isConnected())
        {
            std::cout << "listener disconnected while sending\n"
                      << "flushing wait buffer\n";
            waitFlush();

            // attempt new connection to listener
            connectOutputServer();

            return;
        }

        // send message to output
        myTCPoutput.send(mod);
    }
}

std::vector<std::string> cIn2Out::frameCheck(const std::string &msg)
{
    std::vector<std::string> output;

    if (!myframeCheck)
    {
        // no frame checking required
        output.push_back(msg);
        return output;
    }

    // protect wait buffer from being accessed from another thread
    std::lock_guard<std::mutex> lck(myWaitingMutex);

    // check for complete line in wait buffer
    int p = myWaiting.find("\n");
    if (p == -1)
        return output;      // nothing ready to be sent

    // loop over complete lines in wait buffer    
    while (p != -1)
    {
        // copy line to output buffer
        output.push_back(myWaiting.substr(0, p));

        // remove line from wait buffer
        myWaiting = myWaiting.substr(p + 1);

        // check for another line
        p = myWaiting.find("\n");
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

        // check for flush request
        if (myString == "flush" ||
            myString == "FLUSH")
        {
            waitFlush();
        }
    }
}

void cIn2Out::waitFlush()
{
    std::lock_guard<std::mutex> lck(myWaitingMutex);
    std::cout << "flushing waiting " << myWaiting.size() << "\n";
    myWaiting = "";
    return;
}
void cIn2Out::waitAddFront(const std::string &s)
{
    std::lock_guard<std::mutex> lck(myWaitingMutex);
    myWaiting = s + myWaiting;
}
void cIn2Out::waitAddEnd(const std::string &s)
{
    std::lock_guard<std::mutex> lck(myWaitingMutex);
    myWaiting += s;
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
        waitAddEnd(t.first);
        if (frameCheck(std::string("")).size() != t.second)
        {
            std::cout << "Failed test " << t.first
                      << " => " << frameCheck(std::string("")).size()
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
