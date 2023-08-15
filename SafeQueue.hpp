//#pragma once
//
//#include <mutex>
//#include <queue>
//#include <functional>
//#include <future>
//#include <thread>
//#include <utility>
//#include <vector>
//
//#include <condition_variable>
//
//
//typedef void(*PFEXIT)(void* param);
//
//class SafeQueue
//{
//private:
//	std::queue<void*> m_queue; //利用模板函数构造队列
//	mutable  std::mutex m_mutex; // 访问互斥信号量
//	mutable  std::condition_variable data_cond;
//	int m_max_size = 5;
//	PFEXIT m_clean = NULL;
//public:
//	SafeQueue();
//	SafeQueue(int size);
//	~SafeQueue();
//
//	bool isEmpty();
//	int GetSize();
//	void SetCleanFunction(PFEXIT func);
//	bool PushData(void* data);
//	bool PopData(void* data) = delete;
//	void* wait_and_pop();
//};