#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <sstream>

namespace myLogger {
	
	enum Level {Debug, Info, Error, Critical};
	class BaseLogger {
		class LoggerStream;
		
		protected:
			std::mutex m_lock;
			Level m_level;
			
			std::string getCurrentTime() {
				auto tp = std::chrono::system_clock::now();  //得到系统时间
				time_t tt = std::chrono::system_clock::to_time_t(tp);            //将系统时间time_point 转化为time_t格式
				std::string strTime = ctime(&tt);  
				//删除末尾回车换行符
				strTime.pop_back();
				return strTime;
			}
			virtual void output(std::string str) = 0;
		
		public:
			LoggerStream operator()(Level level);
	};
	
	class BaseLogger::LoggerStream : public std::ostringstream {
		private:
			BaseLogger& m_logger;
			
		public:
			LoggerStream(BaseLogger& cl): m_logger(cl){};
			LoggerStream(const LoggerStream& ls): m_logger(ls.m_logger){};
			~LoggerStream() {
				m_logger.m_lock.lock();
				m_logger.output(this->str());
				m_logger.m_lock.unlock();
			}
	};
	//这个()的重载已经要放在类定义的外面，否则在GCC编译器下回出现对“不完全的类型‘class myLogger::BaseLogger::LoggerStream’的非法使用”的错误提示
	//每一行日志打印以调用()开始，其返回一个临时LoggerStream对象，在其析构函数中加锁输出日志具体内容，防止多线程使用时出现乱序
	BaseLogger::LoggerStream BaseLogger::operator()(Level level){
		m_level = level;
		return LoggerStream(*this);
	}
	class ConsoleLogger : public BaseLogger {
		protected:
			void output(std::string str) {
				std::cout << '[' << getCurrentTime() << ']' \
					<< '[' << m_level << ']' << str;
			}
	};
}