#include "model_conf.hpp"

model_conf::model_conf(std::vector<sensor*> ver):sensor_list(ver){};

void model_conf::stop() {
	if (status) {
		client->disconnect();
		delete client;
		status = false;
	}
};

            //data
vector<string> model_conf::scanner_label() {
	if (status)
	{
		if (client->connect_flag) {
			try {
				return client->read_search();
			}
			catch (...) {
				model_conf::stop();
			}
			
		}
	}
	vector<string> v_null;
	return v_null;
}

bool model_conf::set_voice_model(int flag) {
	if (status) {
		if (client->connect_flag) {
			return client->write_voice_model(flag);
		}
	}
	return false;
}

bool model_conf::write_label(string ori_label, string new_label){
	if (status) {
		if (client->connect_flag) {
			vector<string> hex_v1 = StrUtil::explode(ori_label, ' ');
			u_char* input1 = new u_char[hex_v1.size()];

			for (int i = 0; i < hex_v1.size(); i++) {
				input1[i] = std::stoi(hex_v1[i], nullptr, 16);
			}

			vector<string> hex_v2 = StrUtil::explode(new_label, ' ');
			u_char* input2 = new u_char[hex_v2.size()];

			for (int i = 0; i < hex_v2.size(); i++) {
				input2[i] = std::stoi(hex_v2[i], nullptr, 16);
			}

			client->write_label(input1, hex_v1.size(), input2, hex_v2.size());
			delete[] input1;
			delete[] input2;
			return true;
		}
	}
	return false;
};

bool model_conf::start(int index) {
	string host = sensor_list[index]->host;
	model_conf::stop();
	status = true;
	client = new model_conf_tcp_client();
	return client->connect(host);
};