#include "model_automation.hpp"

model_automation::model_automation(std::vector<sensor*> ver):sensor_list(ver){
	
}

void model_automation::stop() {
	// 停止 io_context
	for (std::vector<model_am_tcp_client*>::iterator it = client_list.begin(); it != client_list.end(); ++it) {
		(*it)->stop(false);
		delete* it;
	}
	client_list.clear();

	// 等待工作线程退出
	if (!worker) {
		worker->join();
		delete worker;
	}
}

void model_automation::start() {
	// 初始化阶段
	for (std::vector<sensor*>::const_iterator it = sensor_list.begin(); it != sensor_list.end(); ++it) {
		asio::io_context* ic = new asio::io_context();
		model_am_tcp_client* client = new model_am_tcp_client(ic, *it);
		client_list.push_back(client);
	}
	// 启动工作线程
	worker = new std::thread(&model_automation::worker_process, this);
}

void model_automation::worker_process(){
	
	// 连接 sensors
	for (std::vector<model_am_tcp_client*>::iterator it = client_list.begin(); it != client_list.end(); ++it) {
		(*it)->start();
	}

}