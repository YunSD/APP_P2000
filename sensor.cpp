#include "sensor.hpp"
#include <string>


sensor::sensor(int index, std::string host):index(index),host(host),connect_flag(false) {}


void sensor::setConnectFlag(bool flag) {
	connect_flag = flag;
};

bool sensor::getConnectFlag() {
	return connect_flag;
};


vector<u_char> sensor::take_temporary() {
	unique_lock<mutex> lock(m_mutex);
	vector<u_char> data;
	if (data_len != 0)
	{
		for (int i = 0; i < data_len; i++) {
			data.push_back(temporary_data[i]);
		}
	}
	return data;
}


void sensor::put_temporary(u_char* data, long len) {
	unique_lock<mutex> lock(m_mutex);
	if (NULL != temporary_data)
	{
		delete[] temporary_data;
	}
	
	temporary_data = data;
	data_len = len;
}