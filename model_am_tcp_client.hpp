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

//
// This class manages socket timeouts by applying the concept of a deadline.
// Some asynchronous operations are given deadlines by which they must complete.
// Deadlines are enforced by an "actor" that persists for the lifetime of the
// client object:
//
//  +----------------+
//  |                |
//  | check_deadline |<---+
//  |                |    |
//  +----------------+    | async_wait()
//              |         |
//              +---------+
//
// If the deadline actor determines that the deadline has expired, the socket
// is closed and any outstanding operations are consequently cancelled.
//
// Connection establishment involves trying each endpoint in turn until a
// connection is successful, or the available endpoints are exhausted. If the
// deadline actor closes the socket, the connect actor is woken up and moves to
// the next endpoint.
//
//  +---------------+
//  |               |
//  | start_connect |<---+
//  |               |    |
//  +---------------+    |
//           |           |
//  async_-  |    +----------------+
// connect() |    |                |
//           +--->| handle_connect |
//                |                |
//                +----------------+
//                          :
// Once a connection is     :
// made, the connect        :
// actor forks in two -     :
//                          :
// an actor for reading     :       and an actor for
// inbound messages:        :       sending heartbeats:
//                          :
//  +------------+          :          +-------------+
//  |            |<- - - - -+- - - - ->|             |
//  | start_read |                     | start_write |<---+
//  |            |<---+                |             |    |
//  +------------+    |                +-------------+    | async_wait()
//          |         |                        |          |
//  async_- |    +-------------+       async_- |    +--------------+
//   read_- |    |             |       write() |    |              |
//  until() +--->| handle_read |               +--->| handle_write |
//               |             |                    |              |
//               +-------------+                    +--------------+
//
// The input actor reads messages from the socket, where messages are delimited
// by the newline character. The deadline for a complete message is 30 seconds.
//
// The heartbeat actor sends a heartbeat (a message that consists of a single
// newline character) every 10 seconds. In this example, no deadline is applied
// to message sending.
//
class model_am_tcp_client
{
public:
    model_am_tcp_client(asio::io_context* io_context, sensor *sens);
    ~model_am_tcp_client();

    // Called by the user of the client class to initiate the connection process.
    // The endpoints will have been obtained using a tcp::resolver.
    void start();

    // This function terminates all the actors to shut down the connection. It
    // may be called by the user of the client class, or by the class itself in
    // response to graceful termination or an unrecoverable error.
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
    bool stopped_ = false;
    tcp::resolver::results_type endpoints_;
    tcp::socket socket_;
    // package_head
    u_char package_head[2];
    sensor *sensor_;

    std::thread* worker;
    asio::io_context* io_context;

    void worker_process();

    steady_timer heartbeat_timer_;
};