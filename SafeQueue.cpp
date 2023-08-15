//#include <queue>
//#include <mutex>
//#include <condition_variable>
//#include <initializer_list>
//#include "SafeQueue.hpp"
//#include <iostream>
//
//using namespace std;
//
//SafeQueue::SafeQueue(){}
//
//SafeQueue::SafeQueue(int size)
//{
//	m_max_size = size;
//}
//
//
//SafeQueue::~SafeQueue()
//{
//	std::unique_lock<std::mutex> lock(m_mutex);
//	std::cout << __FUNCTION__ << "queue size:" << m_queue.size() << endl;
//
//	while (m_queue.size())
//	{
//		auto data = std::move(m_queue.front());
//		m_queue.pop();
//		if (m_clean)
//		{
//			m_clean(data);
//		}
//	}
//}
//
//void SafeQueue::SetCleanFunction(PFEXIT func)
//{
//	m_clean = func;
//}
//
//bool SafeQueue::isEmpty()
//{
//	std::unique_lock<std::mutex> lock(m_mutex);
//	return m_queue.empty();
//}
//
//bool SafeQueue::PushData(void* data)
//{
//	std::unique_lock<std::mutex> lock(m_mutex);
//
//
//	if (m_queue.size() == m_max_size)
//	{
//		return false;
//	}
//	m_queue.emplace(data);
//	data_cond.notify_one();
//	return true;
//}
//
//bool SafeQueue::PopData(void* data)
//{
//	std::unique_lock<std::mutex> lock(m_mutex);
//
//	if (m_queue.empty())
//		return false;
//	data = std::move(m_queue.front());
//
//	m_queue.pop();
//
//	return true;
//
//}
//
//int SafeQueue::GetSize()
//{
//	std::unique_lock<std::mutex> lock(m_mutex);
//
//	return m_queue.size();
//}
//
//void* SafeQueue::wait_and_pop()
//{
//	std::unique_lock<std::mutex> lock(m_mutex);
//	data_cond.wait(lock, [this] {return !this->m_queue.empty(); });
//	if (m_queue.size() == 0)
//	{
//		return NULL;
//	}
//	else
//	{
//		auto value = std::move(m_queue.front());
//		m_queue.pop();
//		return value;
//	}
//}