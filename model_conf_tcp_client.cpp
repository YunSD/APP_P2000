#include "model_conf_tcp_client.hpp"

using namespace std;

model_conf_tcp_client::model_conf_tcp_client(string host):host_(host), socket_(io_context_){};

model_conf_tcp_client::~model_conf_tcp_client() {};

void connect();

void read_search();

void write_label();

void disconnect();