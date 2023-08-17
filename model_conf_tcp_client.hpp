#pragma once

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
    model_conf_tcp_client(string host);
    ~model_conf_tcp_client();

    void connect();

    void read_search();

    void write_label();

    void disconnect();

private:
    void write();

    void read();

private:

    const string host_;
    volatile bool connect_flag = false;

    asio::io_context io_context_;
    tcp::socket socket_;
    // package_head
    u_char command_search_label[2];
    u_char command_write_label[2];
};