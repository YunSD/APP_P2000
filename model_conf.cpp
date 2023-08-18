#include "model_conf.hpp"

model_conf::model_conf(std::vector<sensor*> ver):sensor_list(ver){};

void model_conf::stop() {
	if (status) {
		client->disconnect();
		delete client;
		status = false;
	}
};

void model_conf::scanner_label() {
	if (status)
	{
		if (client->connect_flag) {
			client->read_search();
		}
	}
}

bool model_conf::start(int index) {
	string host = sensor_list[index]->host;
	model_conf::stop();
	status = true;
	client = new model_conf_tcp_client();
	return client->connect(host);
};