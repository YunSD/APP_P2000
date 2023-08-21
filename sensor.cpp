#include "sensor.hpp"
#include <string>


sensor::sensor(int index, std::string host):index(index),host(host),connect_flag(false) {}


void sensor::setConnectFlag(bool flag) {
	connect_flag = flag;
};

bool sensor::getConnectFlag() {
	return connect_flag;
};

vector<string> sensor::take_temporary() {
	unique_lock<mutex> lock(m_mutex);
	return temporary_data;
}


void sensor::put_temporary(vector<string> data) {
	unique_lock<mutex> lock(m_mutex);
	temporary_data = data;
}