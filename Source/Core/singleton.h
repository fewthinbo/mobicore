#pragma once
#include <mutex>
#include <memory>

template <class T>
class CSingletonShared {
private:
	static std::mutex singleton_mutex_; // singleton mutex
	static std::shared_ptr<T> instance_; // conv. to shared_ptr because of enable_shared_from_this classes
public:
	static std::shared_ptr<T> getInstance() {
		if (!instance_) {
			std::lock_guard<std::mutex> lock(singleton_mutex_);
			if (!instance_){
				instance_ = std::make_shared<T>();
			}
		}
		return instance_;
	}
public:
	CSingletonShared()=default;
	CSingletonShared& operator=(CSingletonShared&& other) = delete;
	CSingletonShared(CSingletonShared&& other) = delete;
	CSingletonShared(const CSingletonShared&) = delete;
	CSingletonShared& operator=(const CSingletonShared&) = delete;
};

template <typename T> 
std::shared_ptr<T> CSingletonShared<T>::instance_ = nullptr;

template <typename T>
std::mutex CSingletonShared<T>::singleton_mutex_{};


template <class T>
class CSingleton {
private:
	static std::mutex singleton_mutex_; // singleton mutex
	static std::unique_ptr<T> instance_; // conv. to shared_ptr because of enable_shared_from_this classes
public:
	static T& getInstance() {	
		if (!instance_) {
			std::lock_guard<std::mutex> lock(singleton_mutex_);
			if (!instance_){
				instance_ = std::make_unique<T>();
			}
		}
		return *instance_;
	}
public:
	CSingleton()=default;
	CSingleton& operator=(CSingleton&& other) = delete;
	CSingleton(CSingleton&& other) = delete;
	CSingleton(const CSingleton&) = delete;
	CSingleton& operator=(const CSingleton&) = delete;
};


template <typename T> 
std::unique_ptr<T> CSingleton<T>::instance_ = nullptr;

template <typename T>
std::mutex CSingleton<T>::singleton_mutex_{};
