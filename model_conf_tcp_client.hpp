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

    void read_search();

    void write_label();

    void disconnect();

    volatile bool connect_flag = false;

private:
    void write(u_char* data);

    shared_ptr<u_char[]> read();

    asio::io_context io_context_;
    tcp::socket socket_;
    tcp::resolver::results_type endpoints_;
};