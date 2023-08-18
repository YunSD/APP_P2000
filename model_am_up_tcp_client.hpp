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
using namespace std;

class model_am_up_tcp_client
{
public:
    model_am_up_tcp_client(asio::io_context* io_context, const vector<sensor*> sensor_list);
    ~model_am_up_tcp_client();

    void start();

    void stop(bool flag = true);

private:
    void start_connect(tcp::resolver::results_type::iterator endpoint_iter);

    void handle_connect(const std::error_code& error,
        tcp::resolver::results_type::iterator endpoint_iter);

    void start_read();

    void handle_read(const std::error_code& error, std::size_t n);

    void start_write();

    void repeat_handle_write(const std::error_code& error);

    void handle_write(const std::error_code& error);

private:
    const std::vector<sensor*> sensor_list;

    bool stopped_ = false;
    tcp::resolver::results_type endpoints_;
    tcp::socket socket_;

    std::thread* worker;
    asio::io_context* io_context;

    void worker_process();

    steady_timer heartbeat_timer_;
};