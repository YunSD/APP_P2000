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

static u_char* command_search_label = new u_char[3]{ 0x0A, 0x00, 0x01 };

void model_conf_tcp_client::read_search() {
    model_conf_tcp_client::write(command_search_label);
    shared_ptr<u_char[]> ptr = model_conf_tcp_client::read();
};

void model_conf_tcp_client::write_label() {
    model_conf_tcp_client::write(command_search_label);
    shared_ptr<u_char[]> ptr = model_conf_tcp_client::read();
};

void model_conf_tcp_client::disconnect() {
    if (!connect_flag) {
        std::error_code ignored_error;
        socket_.close(ignored_error);
        connect_flag = false;
    }
};

void model_conf_tcp_client::write(u_char *data) {
    u_char a[3] = { 0x0A, 0x00, 0x01 };
    asio::write(socket_, asio::buffer(a));
    asio::write(socket_, asio::buffer(command_search_label, 3));
};

shared_ptr<u_char[]> model_conf_tcp_client::read() {
    static u_char p_head[2];
    asio::read(socket_, asio::buffer(p_head));
    u_int len = (u_int)p_head[1];

    u_char* data = new u_char[len];
    shared_ptr<u_char[]> ptr_data(data, [](u_char* data) {delete[] data; });

    asio::read(socket_, asio::buffer(data, len));

    return ptr_data;
};