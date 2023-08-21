#pragma once

#include <string>
#include <mutex>
#include <WinSock2.h>
#include <vector>

using namespace std;

class sensor {
public:
	const int index;
	const std::string host;

	sensor(int index, std::string host);
	void setConnectFlag(bool flag);
	bool getConnectFlag();
	
	vector<string> take_temporary();
	void put_temporary(vector<string> data);

private:
	std::mutex m_mutex;
	volatile bool connect_flag;

	vector<string> temporary_data;
};