#include "in2out.h"

/** Modify input for output
 * @param[in] input the last line of a packet received
 * @return the modified string to output
 */
std::string cIn2Out::Process( const std::string& input )
{
    std::string mod;

    // TODO: replace with whatever modification you require
    mod = input + " modified";
    
    return mod;
}