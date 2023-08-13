#include "sensor.hpp"
#include <string>


sensor::sensor(int index, std::string host):index(index),host(host),connect_flag(false) {}


void sensor::setConnectFlag(bool flag) {
	connect_flag = flag;
};

bool sensor::getConnectFlag() {
	return connect_flag;
};