#include "model_am_tcp_client.hpp"
#include "StrUtil.h"

using namespace std;

model_am_tcp_client::model_am_tcp_client(asio::io_context* io_context_, sensor *sens)
    : socket_(*io_context_), sensor_(sens), heartbeat_timer_(*io_context_), io_context(io_context_)
{ 
    tcp::resolver* r = new tcp::resolver(*io_context);
    vector<string> address{StrUtil::explode(sens->host, ':')};
    endpoints_ = r->resolve(address[0], address[1]);
}

model_am_tcp_client::~model_am_tcp_client() {
    if (NULL == worker) delete worker;
    if (NULL == io_context) delete io_context;
}

// Called by the user of the client class to initiate the connection process.
// The endpoints will have been obtained using a tcp::resolver.
void model_am_tcp_client::start()
{
    // Start the connect actor.
    start_connect(endpoints_.begin());

    // work
    worker = new std::thread(&model_am_tcp_client::worker_process, this);

// Start the deadline actor. You will note that we're not setting any
// particular deadline here. Instead, the connect and input actors will
// update the deadline prior to each asynchronous operation.
// deadline_.async_wait(std::bind(&client::check_deadline, this));
}

void model_am_tcp_client::worker_process() {
    io_context->run();
};

// This function terminates all the actors to shut down the connection. It
// may be called by the user of the client class, or by the class itself in
// response to graceful termination or an unrecoverable error.

void model_am_tcp_client::stop(bool flag)
{
    stopped_ = true;
    std::error_code ignored_error;
    //deadline_.cancel();
    heartbeat_timer_.cancel();
    socket_.close(ignored_error);
    // update flag
    sensor_->setConnectFlag(false);
    if (stopped_ && flag) {
        stopped_ = false;
        socket_.async_connect(endpoints_->endpoint(),
            std::bind(&model_am_tcp_client::handle_connect,
                this, _1, endpoints_));
    }
    else {
        io_context->stop();
        if(worker) worker->join();
        // 移除
        delete worker;
    }
}

void model_am_tcp_client::start_connect(tcp::resolver::results_type::iterator endpoint_iter)
{
    if (endpoint_iter != endpoints_.end())
    {
        std::cout << "Trying " << endpoint_iter->endpoint() << "...\n";

        // Set a deadline for the connect operation.
        //deadline_.expires_after(std::chrono::seconds(60));

        // Start the asynchronous connect operation.
        socket_.async_connect(endpoint_iter->endpoint(),
            std::bind(&model_am_tcp_client::handle_connect,
                this, _1, endpoint_iter));
    }
    else
    {
        // There are no more endpoints to try. Shut down the client.
        stop();
    }
}

void model_am_tcp_client::handle_connect(const std::error_code& error,
    tcp::resolver::results_type::iterator endpoint_iter)
{
    if (stopped_)
        return;

    // The async_connect() function automatically opens the socket at the start
    // of the asynchronous operation. If the socket is closed at this time then
    // the timeout handler must have run first.
    if (!socket_.is_open())
    {
        std::cout << "Connect timed out\n";

        // Try the next available endpoint.
        start_connect(endpoint_iter);
    }

    // Check if the connect operation failed before the deadline expired.
    else if (error)
    {
        std::cout << "Connect error: " << error.message() << "\n";

        // We need to close the socket used in the previous connection attempt
        // before starting a new one.
        socket_.close();

        // Try the next available endpoint.
        //std::this_thread::sleep_for(std::chrono::seconds(10));
        start_connect(endpoint_iter);
    }

    // Otherwise we have successfully established a connection.
    else
    {
        std::cout << "Connected to " << endpoint_iter->endpoint() << "\n";
        sensor_->setConnectFlag(true);

        // Start the input actor.
        start_read();

        // Start the heartbeat actor.
        start_write();
    }
}

void model_am_tcp_client::start_read()
{
    // Set a deadline for the read operation.
    //deadline_.expires_after(std::chrono::seconds(30));

    // Start an asynchronous operation to read a newline-delimited message.
    asio::async_read(socket_, asio::buffer(package_head), 
        std::bind(&model_am_tcp_client::handle_read, this, _1, _2));
    //asio::async_read_until(socket_,
    //    asio::dynamic_buffer(input_buffer_), '\n',
    //    std::bind(&model_am_tcp_client::handle_read, this, _1, _2));
}

void model_am_tcp_client::handle_read(const std::error_code& error, std::size_t n)
{
    if (stopped_)
        return;

    if (!error)
    {   
        // get len
        u_int len = (u_int)package_head[1];

        u_char* data = new u_char[len];

        // 全部读取
        try {
            asio::read(socket_, asio::buffer(data, len));
            sensor_->put_temporary(data, len);
        }
        catch (...) {
            stop();
        }
        
        
        // reset
        //package_head.erase(0, n);

        // Empty messages are heartbeats and so ignored.
        /*if (!line.empty())
        {
            std::cout << "Received: " << line << "\n";
        }*/

        start_read();
    }
    else
    {
        std::cout << "Error on receive: " << error.message() << "\n";

        stop();
    }
}

void model_am_tcp_client::start_write()
{
    if (stopped_)
        return;

    // Start an asynchronous operation to send a heartbeat message.
    asio::async_write(socket_, asio::buffer("\n", 1),
        std::bind(&model_am_tcp_client::handle_write, this, _1));
}

void model_am_tcp_client::repeat_handle_write(const std::error_code& error) {
    if (!error) {
        start_write();
    }
}

void model_am_tcp_client::handle_write(const std::error_code& error)
{
    if (stopped_)
        return;
    if (!error)
    {
        // Wait 10 seconds before sending the next heartbeat.
        heartbeat_timer_.cancel();
        heartbeat_timer_.expires_after(std::chrono::seconds(5));
        heartbeat_timer_.async_wait(std::bind(&model_am_tcp_client::repeat_handle_write, this, _1));
    }
    else
    {
        std::cout << "Error on heartbeat: " << error.message() << "\n";

        stop();
    }
}




