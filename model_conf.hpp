#pragma once

#include "sensor.hpp"
#include <vector>
#include <thread>
#include "asio/io_context.hpp"
#include "model_conf_tcp_client.hpp"



class model_conf {
public:
    model_conf(std::vector<sensor*> ver);

    void stop();

    void scanner_label();

    bool start(int index);

private:
    volatile bool status = false;

    const std::vector<sensor*> sensor_list;

    model_conf_tcp_client* client;

};