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
		T item_buffer[kItemRepositorySize]; // ��Ʒ������, ��� read_position �� write_position ģ�ͻ��ζ���.
		size_t read_position; // �����߶�ȡ��Ʒλ��.
		size_t write_position; // ������д���Ʒλ��.
		std::mutex mtx; // ������,������Ʒ������
		std::condition_variable repo_not_full; // ��������, ָʾ��Ʒ��������Ϊ��.
		std::condition_variable repo_not_empty; // ��������, ָʾ��Ʒ��������Ϊ��.
	} m_ItemRepository; // ��Ʒ��ȫ�ֱ���, �����ߺ������߲����ñ���.

	typedef struct ItemRepository ItemRepository;

public:
	SingleConsumerSingleProducer() {
		m_ItemRepository.write_position = 0; // ��ʼ����Ʒд��λ��.
		m_ItemRepository.read_position = 0; // ��ʼ����Ʒ��ȡλ��.
	}

	void ProduceItem(T item)
	{
		std::unique_lock<std::mutex> lock(m_ItemRepository.mtx);
		while (((m_ItemRepository.write_position + 1) % kItemRepositorySize)
			== m_ItemRepository.read_position) { // item buffer is full, just wait here.
			//std::cout << "Producer is waiting for an empty slot...\n";
			(m_ItemRepository.repo_not_full).wait(lock); // �����ߵȴ�"��Ʒ�⻺������Ϊ��"��һ��������.
		}

		(m_ItemRepository.item_buffer)[m_ItemRepository.write_position] = item; // д���Ʒ.
		(m_ItemRepository.write_position)++; // д��λ�ú���.

		if (m_ItemRepository.write_position == kItemRepositorySize) // д��λ�������ڶ����������������Ϊ��ʼλ��.
			m_ItemRepository.write_position = 0;

		(m_ItemRepository.repo_not_empty).notify_all(); // ֪ͨ�����߲�Ʒ�ⲻΪ��.
		lock.unlock(); // ����.
	}

	T ConsumeItem()
	{
		T data;
		std::unique_lock<std::mutex> lock(m_ItemRepository.mtx);
		// item buffer is empty, just wait here.
		while (m_ItemRepository.write_position == m_ItemRepository.read_position) {
			std::cout << "Consumer is waiting for items...\n";
			(m_ItemRepository.repo_not_empty).wait(lock); // �����ߵȴ�"��Ʒ�⻺������Ϊ��"��һ��������.
		}

		data = (m_ItemRepository.item_buffer)[m_ItemRepository.read_position]; // ��ȡĳһ��Ʒ
		(m_ItemRepository.read_position)++; // ��ȡλ�ú���
		//std::cout << "one item consumed...\n";

		if (m_ItemRepository.read_position >= kItemRepositorySize) // ��ȡλ�����Ƶ������������λ.
			m_ItemRepository.read_position = 0;

		(m_ItemRepository.repo_not_full).notify_all(); // ֪ͨ�����߲�Ʒ�ⲻΪ��.
		lock.unlock(); // ����.

		return data; // ���ز�Ʒ.
	}

};