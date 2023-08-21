#include "model_am_up_tcp_client.hpp"
#include "StrUtil.h"

using namespace std;

model_am_up_tcp_client::model_am_up_tcp_client(asio::io_context* io_c, const string host,
    const string port, const vector<sensor*> sensor_list) : 
    socket_(*io_c), sensor_list_(sensor_list), heartbeat_timer_(*io_c), io_context(io_c)
{
    tcp::resolver* r = new tcp::resolver(*io_context);
    endpoints_ = r->resolve(host, port);
}

model_am_up_tcp_client::~model_am_up_tcp_client() {
    /*if (NULL == worker) delete worker;*/
    delete io_context;
}

void model_am_up_tcp_client::start()
{
    model_am_up_tcp_client::start_connect(endpoints_.begin());
    worker = new std::thread(&model_am_up_tcp_client::worker_process, this);
}

void model_am_up_tcp_client::worker_process() {
    io_context->run();
};

void model_am_up_tcp_client::stop(bool flag)
{
    stopped_ = true;
    std::error_code ignored_error;
    heartbeat_timer_.cancel();
    socket_.close(ignored_error);
    // update flag
    if (stopped_ && flag) {
        stopped_ = false;
        socket_.async_connect(endpoints_->endpoint(),
            std::bind(&model_am_up_tcp_client::handle_connect,
                this, _1, endpoints_));
    }
    else {
        io_context->stop();
        if (worker) worker->join();
        // 移除
        delete worker;
    }
}

void model_am_up_tcp_client::start_connect(tcp::resolver::results_type::iterator endpoint_iter)
{
    if (endpoint_iter != endpoints_.end())
    {
        socket_.async_connect(endpoint_iter->endpoint(),
            std::bind(&model_am_up_tcp_client::handle_connect,
                this, _1, endpoint_iter));
    }
    else
    {
        stop();
    }
}

void model_am_up_tcp_client::handle_connect(const std::error_code& error,
    tcp::resolver::results_type::iterator endpoint_iter)
{
    if (stopped_)
        return;

    if (!socket_.is_open())
    {
        start_connect(endpoint_iter);
    }

    else if (error)
    {
        socket_.close();
        start_connect(endpoint_iter);
    }

    // Otherwise we have successfully established a connection.
    else
    {
        start_read();
        start_write();
    }
}

static u_char* ignore_data = new u_char[100];
void model_am_up_tcp_client::start_read()
{
    asio::async_read(socket_, asio::buffer(ignore_data, 100),
        std::bind(&model_am_up_tcp_client::handle_read, this, _1, _2));
}

void model_am_up_tcp_client::handle_read(const std::error_code& error, std::size_t n)
{
    if (stopped_)
        return;

    if (!error)
    {
        start_read();
    }
    else
    {
        stop();
    }
}

void model_am_up_tcp_client::gen_message() {
    string content = gen_message_content();
    int len = 2 + 4 + content.size() + 2;
    
    static char header[] = { 0xAA, 0xDA };
    static char end[] = {0xFF, 0xFF};

    char* ucharArr = const_cast<char*>(content.data());

    char* data = new char[len];
    // head
    data[0] = header[0];
    data[1] = header[1];
    // len
    for (int i = 0; i < 4; i++)
        data[i + 2] = (content.size() + 2 >> 8 * (3 - i) & 0xFF);
    // content
    for (int i = 0; i < content.size(); i++)
    {
        data[i + 2 + 4] = ucharArr[i];
    }
    // end
    data[content.size() + 2 + 4 + 1] = end[0];
    data[content.size() + 2 + 4 + 2] = end[1];

    data_len = len;
    data_ = data;
}

string model_am_up_tcp_client::gen_message_content() {
    stringstream ss;
    ss << u8"[";
    for (int i = 0; i < sensor_list_.size(); i++) {
        
        string index = std::to_string(sensor_list_[i]->index);
        string flag = sensor_list_[i]->getConnectFlag() ? "true" : "false" ;
        if (i > 0) ss << ",";
        ss << u8"{";
        ss << u8"\"index\":\"" << index << u8"\",";
        ss << u8"\"flag\":\"" << flag << u8"\",";

        if (sensor_list_[i]->getConnectFlag()) {
            vector<string> data_list = sensor_list_[i]->take_temporary();
            
            ss << u8"\"data\":\"" << flag << u8"\"";
            for (int j = 0; j < data_list.size(); j++) {
                if (j > 0) ss << ",";
                ss << data_list[j];
            };
            ss << u8"\"";
        }
       
        ss << u8"}";
    }
    ss << u8"]";
    return ss.str();
}

void model_am_up_tcp_client::start_write()
{
    if (stopped_)
        return;
    char* data = data_;
    int len = data_len;
    Log::LOG_DEBUG("UP MES write: {}", HEX::hex_to_string(data, len));
    asio::async_write(socket_, asio::buffer(data, len),
        std::bind(&model_am_up_tcp_client::handle_write, this, _1));
}

void model_am_up_tcp_client::repeat_handle_write(const std::error_code& error) {
    if (!error) {
        delete[] data_;
        start_write();
    }
}

void model_am_up_tcp_client::handle_write(const std::error_code& error)
{
    if (stopped_)
        return;
    if (!error)
    {
        // Wait 10 seconds before sending the next heartbeat.
        heartbeat_timer_.cancel();
        heartbeat_timer_.expires_after(std::chrono::seconds(5));
        heartbeat_timer_.async_wait(std::bind(&model_am_up_tcp_client::repeat_handle_write, this, _1));
    }
    else
    {
        stop();
    }
}




