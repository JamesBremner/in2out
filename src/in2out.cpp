#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>

#include "cCommandParser.h"

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
    // test();
}

void cIn2Out::ParseOptionsNoBoost(int ac, char **av)
{
    raven::set::cCommandParser CP;
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
                // Some input has arrived
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
        std::cout << "cIn2Out::run unhandled exception " << e.what();
    }
}

void cIn2Out::input(const std::string &msg)
{
    inputDisplay(msg);

    // Loop over complete lines
    for (auto &line : frameCheck(msg))
    {
        if (!myTCPoutput.isConnected())
        {
            noServerHandler();
            return;
        }

        // send line to output
        try
        {
            myTCPoutput.send(line);
        }
        catch (std::runtime_error &e)
        {
            noServerHandler();
            return;
        }
    }
}

void cIn2Out::noServerHandler()
{
    // let user know what is going on
    std::cout << "no-one is listening\n"
                 "A new connect to server will be attempted\n"
                 "Meanwhile input data will be discarded\n";

    // discard remaining backlog
    frameCheck("#$%clear");

    // attempt new connection
    connectOutputServer();
}
void cIn2Out::inputDisplay(const std::string &msg)
{
    std::cout << "Input: " + msg << "\n";
    std::cout << "Input Hex: ";
    for (int k = 0; k < msg.size(); k++)
    {
        std::cout << std::hex << (int)msg[k] << " ";
    }
    std::cout << "\n\n";
}

std::vector<std::string> cIn2Out::frameCheck(const std::string &msg)
{
    std::vector<std::string> output;

    if (!myframeCheck)
    {
        output.push_back(msg);
        return output;
    }
    if (msg == "#$%clear")
    {
        myBacklog = "";
        return output;
    }

    myBacklog += msg;

    int lineCount = countLines();

    while (lineCount > 1)
    {
        int p = myBacklog.find("\n");
        if (p == -1)
            throw std::runtime_error("frameCheck line count");
        auto line1 = myBacklog.substr(0, p);
        int q = myBacklog.substr(p + 1).find("\n");
        auto line2 = myBacklog.substr(p + 1, q);
        myBacklog = myBacklog.substr(p + 1);
        lineCount--;
        if (isLastData(line1, line2))
        {
            std::cout << "Package end\n";

            // output modified last line of package
            output.push_back(Process(line1));

            // output package terminator
            output.push_back("package_end\n");

            return output;
        }
        // output data line or HEADER X
        output.push_back(line1);
    }
    return output;
}

bool cIn2Out::isLastData(
    const std::string &line1,
    const std::string &line2) const
{
    // std::cout << "isLastData  "
    //           << "\nline1 " << line1
    //           << "\nline2 " << line2 << "\n";

    bool line1Data = (line1[2] == '/' &&
                      line1[5] == '/' &&
                      atoi(line1.c_str()) != 0 &&
                      atoi(line1.substr(3).c_str()) != 0 &&
                      atoi(line1.substr(6).c_str()) != 0);
    bool line2Data = (line2[2] == '/' &&
                      line2[5] == '/' &&
                      atoi(line2.c_str()) != 0 &&
                      atoi(line2.substr(3).c_str()) != 0 &&
                      atoi(line2.substr(6).c_str()) != 0);

    return (line1Data && (!line2Data));
}

int cIn2Out::countLines() const
{
    auto w = myBacklog;
    int count = 0;
    int p = w.find("\n");
    while (p != -1)
    {
        count++;
        w = w.substr(p + 1);
        p = w.find("\n");
    }
    return count;
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
