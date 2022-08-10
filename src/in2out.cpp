#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <boost/program_options.hpp>
#include <wex.h>

class cIn2Out
{
public:
    cIn2Out();
    void ParseOptions(int ac, char **av);

private:
    int myInputPort;
    std::string myOutputIP;
     int myOutputPort;
};

cIn2Out::cIn2Out()
    : myInputPort(-1), myOutputPort(-1)
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

main(int argc, char *argv[])
{
    cIn2Out in2out;
    in2out.ParseOptions(argc, argv);
    return 0;
}
