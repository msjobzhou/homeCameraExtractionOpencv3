#pragma once

#include <cstdlib>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include<chrono>

template <typename T>
class SingleConsumerSingleProducer{

private:
	static const int kItemRepositorySize = 5; // Item buffer size.

	struct ItemRepository {
		T item_buffer[kItemRepositorySize]; // 产品缓冲区, 配合 read_position 和 write_position 模型环形队列.
		size_t read_position; // 消费者读取产品位置.
		size_t write_position; // 生产者写入产品位置.
		std::mutex mtx; // 互斥量,保护产品缓冲区
		std::condition_variable repo_not_full; // 条件变量, 指示产品缓冲区不为满.
		std::condition_variable repo_not_empty; // 条件变量, 指示产品缓冲区不为空.
	} m_ItemRepository; // 产品库全局变量, 生产者和消费者操作该变量.

	typedef struct ItemRepository ItemRepository;

public:
	SingleConsumerSingleProducer() {
		m_ItemRepository.write_position = 0; // 初始化产品写入位置.
		m_ItemRepository.read_position = 0; // 初始化产品读取位置.
	}

	void ProduceItem(T item)
	{
		std::unique_lock<std::mutex> lock(m_ItemRepository.mtx);
		while (((m_ItemRepository.write_position + 1) % kItemRepositorySize)
			== m_ItemRepository.read_position) { // item buffer is full, just wait here.
			//std::cout << "Producer is waiting for an empty slot...\n";
			(m_ItemRepository.repo_not_full).wait(lock); // 生产者等待"产品库缓冲区不为满"这一条件发生.
		}

		(m_ItemRepository.item_buffer)[m_ItemRepository.write_position] = item; // 写入产品.
		(m_ItemRepository.write_position)++; // 写入位置后移.

		if (m_ItemRepository.write_position == kItemRepositorySize) // 写入位置若是在队列最后则重新设置为初始位置.
			m_ItemRepository.write_position = 0;

		(m_ItemRepository.repo_not_empty).notify_all(); // 通知消费者产品库不为空.
		lock.unlock(); // 解锁.
	}

	T ConsumeItem()
	{
		T data;
		std::unique_lock<std::mutex> lock(m_ItemRepository.mtx);
		// item buffer is empty, just wait here.
		while (m_ItemRepository.write_position == m_ItemRepository.read_position) {
			std::cout << "Consumer is waiting for items...\n";
			(m_ItemRepository.repo_not_empty).wait(lock); // 消费者等待"产品库缓冲区不为空"这一条件发生.
		}

		data = (m_ItemRepository.item_buffer)[m_ItemRepository.read_position]; // 读取某一产品
		(m_ItemRepository.read_position)++; // 读取位置后移
		//std::cout << "one item consumed...\n";

		if (m_ItemRepository.read_position >= kItemRepositorySize) // 读取位置若移到最后，则重新置位.
			m_ItemRepository.read_position = 0;

		(m_ItemRepository.repo_not_full).notify_all(); // 通知消费者产品库不为满.
		lock.unlock(); // 解锁.

		return data; // 返回产品.
	}

};