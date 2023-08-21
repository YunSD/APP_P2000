#pragma once

#include "sensor.hpp"
#include <vector>
#include <thread>
#include "asio/io_context.hpp"
#include "model_am_tcp_client.hpp"
#include "model_am_up_tcp_client.hpp"



class model_automation {
public:
    model_automation(std::vector<sensor*> ver, string host, string port);

    void stop();

    void start();

private:
    const std::vector<sensor*> sensor_list;
    std::thread* worker;

    model_am_up_tcp_client* up_client;
    std::vector<model_am_tcp_client*> client_list;

    void worker_process();

    const string up_host;
    const string up_port;
};