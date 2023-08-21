#pragma once


#ifndef HEX_H
#define HEX_H
#include <sstream>
#include <WinSock2.h>

using namespace std;

namespace HEX {

    string hex_to_string(char* bytes, int len);

    string u_hex_to_string(u_char* bytes, int len, char split);

    string intToHex(int num);

    u_char crc_calculate(u_char* bytes, int len);
}
#endif