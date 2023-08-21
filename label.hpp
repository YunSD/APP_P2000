#pragma once

#include <string>
#include <mutex>
#include <WinSock2.h>
#include <vector>

using namespace std;

class sensor_epc {
public:
	const string epc;
	const string pc;
	const int num;

	sensor_epc(string index, string pc, int num);
	void setConnectFlag(bool flag);
	bool getConnectFlag();

	vector<u_char> take_temporary();
	void put_temporary(u_char* data, long len);

private:
	std::mutex m_mutex;
	volatile bool connect_flag;

	u_char* temporary_data;
	long data_len = 0;
};