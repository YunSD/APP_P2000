#pragma once

#include "sensor.hpp"
#include <vector>
#include <thread>
#include "asio/io_context.hpp"
#include "model_am_tcp_client.hpp"



class model_automation {
public:
    model_automation(std::vector<sensor*> ver);

    void stop();

    void start();

private:
    const std::vector<sensor*> sensor_list;
    std::thread* worker;

    std::vector<model_am_tcp_client*> client_list;

    asio::io_context io_context;

    void worker_process();
};