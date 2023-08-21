#include "model_conf_tcp_client.hpp"
#include "StrUtil.h"

using namespace std;
using asio::ip::tcp;

model_conf_tcp_client::model_conf_tcp_client(): socket_(io_context_){};

model_conf_tcp_client::~model_conf_tcp_client() {};

bool model_conf_tcp_client::connect(string host) {
    model_conf_tcp_client::disconnect();
    try {
        tcp::resolver res(io_context_);
        vector<string> address{StrUtil::explode(host, ':')};
        endpoints_ = res.resolve(address[0], address[1]);
        asio::connect(socket_, endpoints_);
        connect_flag = true;
        return 1;
    }
    catch (...) {
        return 0;
    }
};

static u_char* command_search_label = new u_char[6]{ 0xA0, 0x04, 0x01, 0x89, 0x01, 0xD1 };

vector<string> model_conf_tcp_client::read_search() {
    model_conf_tcp_client::write(command_search_label, 6);
    return model_conf_tcp_client::read(true);
};

void model_conf_tcp_client::write_label(u_char* data1, int len1, u_char* data2, int len2) {
    
    // 第一次进行 epc 匹配
    int ac_len = 2 + 1 + 1 + 1 + 1 + len1 + 1;
    u_char* match_command_1 = new u_char[ac_len];
    match_command_1[0] = 0xA0;
    match_command_1[1] = static_cast<u_char>(ac_len-2);
    match_command_1[2] = 0x01;
    match_command_1[3] = 0x85;
    match_command_1[4] = 0x00;
    match_command_1[5] = static_cast<u_char>(len1);

    for (int i = 6, j = 0; i < 6 + len1; i++, j++)
    {
        match_command_1[i] = data1[j];
    }
    match_command_1[ac_len - 1] = HEX::crc_calculate(match_command_1, ac_len - 1);

    model_conf_tcp_client::write(match_command_1, ac_len);
    model_conf_tcp_client::read(false);

    delete[] match_command_1;

    // 第二次读信息
    static u_char* match_command_2 = new u_char[12]{ 0xA0, 0x0A, 0x01, 0x81, 0x01, 0x02, 0x06, 0x00, 0x00, 0x00, 0x00, 0xCB};
    
    model_conf_tcp_client::write(match_command_2, 12);
    model_conf_tcp_client::read(false);

    // 第三次写入信息
    ac_len = 4 + 4 + 3 + len2 + 1;
    u_char* match_command_3 = new u_char[ac_len];
    match_command_3[0] = 0xA0;
    match_command_3[1] = static_cast<u_char>(ac_len-2);
    match_command_3[2] = 0x01;
    match_command_3[3] = 0x94;
    match_command_3[4] = 0x00;
    match_command_3[5] = 0x00;
    match_command_3[6] = 0x00;
    match_command_3[7] = 0x00;
    match_command_3[8] = 0x01;
    match_command_3[9] = 0x02;
    match_command_3[10] = 0x06;

    for (int i = 11, j = 0; i < 11 + len2; i++, j++)
    {
        match_command_3[i] = data2[j];
    }
    match_command_3[ac_len - 1] = HEX::crc_calculate(match_command_3, ac_len - 1);

    model_conf_tcp_client::write(match_command_3, ac_len);
    model_conf_tcp_client::read(false);

    delete[] match_command_3;
    
};

static u_char* voice_0 = new u_char[6]{ 0xA0, 0x04, 0x01, 0x7A, 0x00, 0xE1 };
static u_char* voice_1 = new u_char[6]{ 0xA0, 0x04, 0x01, 0x7A, 0x01, 0xE0 };
static u_char* voice_2 = new u_char[6]{ 0xA0, 0x04, 0x01, 0x7A, 0x02, 0xDF };

bool model_conf_tcp_client::write_voice_model(int flag) {
    if (flag == 0) {
        model_conf_tcp_client::write(voice_0, 6);
    }
    else if (flag == 1) {
        model_conf_tcp_client::write(voice_1, 6);
    }
    else if (flag == 2) {
        model_conf_tcp_client::write(voice_2, 6);
    }

    model_conf_tcp_client::read(false);
    return true;
}

void model_conf_tcp_client::disconnect() {
    if (!connect_flag) {
        std::error_code ignored_error;
        socket_.close(ignored_error);
        connect_flag = false;
    }
};

void model_conf_tcp_client::write(u_char *data,int len) {
    Log::LOG_DEBUG("MODE-C write: {}", HEX::u_hex_to_string(data, len, ' '));
    asio::write(socket_, asio::buffer(data, len));
};

vector<string> model_conf_tcp_client::read(bool circuit) {
    static u_char p_head[2];

    vector<string> v_char;
    bool block = true;

    if (circuit) {
        while (block) {
            asio::read(socket_, asio::buffer(p_head));
            u_int len = (u_int)p_head[1];

            if (len == 0x0A || len == 0x04) block = false;

            u_char* data = new u_char[len];
            asio::read(socket_, asio::buffer(data, len));

            if (block) {
                string data_ = HEX::u_hex_to_string(data, len - 2, ' ');
                v_char.push_back(data_.substr(1, data_.size() - 2));
            }
            delete[] data;
        }
    }
    else {
        asio::read(socket_, asio::buffer(p_head));
        u_int len = (u_int)p_head[1];

        u_char* data = new u_char[len];
        asio::read(socket_, asio::buffer(data, len));

        string data_ = HEX::u_hex_to_string(data, len - 2, ' ');
        v_char.push_back(data_.substr(1, data_.size() - 2));
        delete[] data;
    }
    
    return v_char;
};