#pragma once

#include "asio.hpp"
#include "asio/buffer.hpp"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/read_until.hpp"
#include "asio/read.hpp"
#include "asio/steady_timer.hpp"
#include "asio/write.hpp"
#include "sensor.hpp"
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include "hex_util.h"
#include "log.hpp"

using asio::steady_timer;
using asio::ip::tcp;
using std::placeholders::_1;
using std::placeholders::_2;

class model_conf_tcp_client
{
public:
    model_conf_tcp_client();
    ~model_conf_tcp_client();

    bool connect(string host);

    vector<string> read_search();

    void write_label(u_char* data1, int len1, u_char* data2, int len2);

    // 1:安静 2：盘存后蜂鸣 3：读到标签时蜂鸣(每次)
    bool write_voice_model(int flag);

    void disconnect();

    volatile bool connect_flag = false;

private:
    void write(u_char* data, int len);

    vector<string> read(bool circuit);

    asio::io_context io_context_;
    tcp::socket socket_;
    tcp::resolver::results_type endpoints_;
};