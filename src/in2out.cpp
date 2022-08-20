#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>

#include "in2out.h"

cIn2Out::cIn2Out(int ac, char **av) : myframeCheck(false)
{
    // start keyboard monitor for exit command
    myKeyboardThread = new std::thread(
        keyboardmonitor, this);

    // parse command line
    ParseOptionsNoBoost(ac, av);

    // start the sockets
    connect();
}
cIn2Out::cIn2Out()
{
    test();
}

class cCommandOption
{
public:
    cCommandOption(
        const std::string &name,
        const std::string &desc)
        : myName(name),
          myDesc(desc)
    {
    }
    void describe() const
    {
        std::cout << " --"
                  << std::setw(10) << std::left << myName
                  << "\t" << myDesc << "\n";
    }
    void value(const std::string &v)
    {
        myValue = v;
    }
    std::string value() const
    {
        return myValue;
    }

private:
    std::string myName;
    std::string myDesc;
    std::string myValue;
};

class cCommandParser
{
public:
    void add(
        const std::string &name,
        const std::string &desc)
    {
        myOption.insert(
            std::make_pair(
                name,
                cCommandOption(name, desc)));
    }
    void describe()
    {
        std::cout << "\n";
        for (auto o : myOption)
            o.second.describe();
        std::cout << "\n";
    }
    void parse(int ac, char **av)
    {
        if (ac <= 1)
        {
            describe();
            exit(0);
        }

        std::string myLine;
        for (int t = 1; t < ac; t++)
            myLine += std::string(av[t]) + " ";

        std::istringstream iss(myLine);
        std::string token, v;
        for (int k = 0; k < (ac - 1) / 2; k++)
        {
            iss >> token >> v;
            if (token == "--help")
            {
                describe();
                exit(0);
            }
            auto o = myOption.find(token.substr(2));
            if (o == myOption.end())
                continue;
            o->second.value(v);
        }
    }
    std::string value(const std::string name) const
    {
        auto o = myOption.find(name);
        if (o == myOption.end())
            return "";
        return o->second.value();
    }

private:
    std::map<std::string, cCommandOption> myOption;
};

void cIn2Out::ParseOptionsNoBoost(int ac, char **av)
{
    cCommandParser CP;
    CP.add("help", "produce help message");
    CP.add("input", "Port to listen for input");
    CP.add("output", "IP address and port to send output e.g. 127.0.0.1:5001");

    CP.parse(ac, av);

    auto v = CP.value("input");
    if (!v.empty())
        myInputPort = atoi(v.c_str());

    v = CP.value("output");
    if (!v.empty())
    {
        int p = v.find(":");
        if (p == -1)
        {
            CP.describe();
            exit(0);
        }
        myOutputIP = v.substr(0, p);
        myOutputPort = atoi(v.substr(p + 1).c_str());
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
                  << myOutputIP << ":"
                  << std::dec << myOutputPort << "\n";

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

    // Loop over complete lines
    for (auto &line : frameCheck(msg))
    {
        // modify message
        auto mod = Process(line);
        std::cout << "Output " + mod << "\n";

        // send message to output
        if (!myTCPoutput.isConnected())
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
