#pragma once

#include <string>

class sensor {
public:
	const int index;
	const std::string host;

	sensor(int index, std::string host);
	void setConnectFlag(bool flag);
	bool getConnectFlag();

private:
	volatile bool connect_flag;
};