#include "hex_util.h"

string HEX::hex_to_string(char* bytes, int len) {
    std::stringstream sstream;
    sstream << std::uppercase << std::hex << "[";
    int i = 0;
    while (i < len)
    {
        if (i != 0) sstream << ",";
        std::string num = intToHex(static_cast<int>(bytes[i]));
        sstream << num;
        i++;
    }

    sstream << "]";

    return sstream.str();
}

string HEX::u_hex_to_string(u_char* bytes, int len, char split) {
    std::stringstream sstream;
    sstream << std::uppercase << std::hex << "[";
    int i = 0;
    while (i < len)
    {
        if (i != 0) sstream << split;
        std::string num = intToHex(static_cast<int>(bytes[i]));
        sstream << num;
        i++;
    }

    sstream << "]";

    return sstream.str();
}


string HEX::intToHex(int num) {
    std::stringstream stream;
    stream << std::hex << num;
    std::string result(stream.str());

    // 如果需要补齐位数，则取消下面的注释，并设置相应的位数
    // 如果不需要补齐位数，则可以删除下面的代码块

     if (result.length() % 2 != 0) {
         result.insert(0, "0");  // 补齐位数为偶数
     }

    return result;
}

u_char HEX::crc_calculate(u_char* bytes, int len) {
    u_char i, uSum = 0;
    for (i = 0; i < len; i++)
    {
        uSum = uSum + bytes[i];
    }
    uSum = (~uSum) + 1;
    return uSum;
};
