#pragma once

#include <string>
#include <mutex>
#include <WinSock2.h>
#include <vector>

using namespace std;

namespace CONFIG{
	class conf {
	public:
		const string ip_;

		const string port_;

		const string model_;

		conf(string ip, string port, string model);
	};
}

