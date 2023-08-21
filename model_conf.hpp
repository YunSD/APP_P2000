#pragma once

#include "sensor.hpp"
#include <vector>
#include <thread>
#include "asio/io_context.hpp"
#include "model_conf_tcp_client.hpp"
#include "StrUtil.h"



class model_conf {
public:
    model_conf(std::vector<sensor*> ver);

    void stop();

    vector<string> scanner_label();

    bool set_voice_model(int flag);

    bool write_label(string ori_label, string new_label);

    bool start(int index);

private:
    volatile bool status = false;

    const std::vector<sensor*> sensor_list;

    model_conf_tcp_client* client;

};