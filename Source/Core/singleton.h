#pragma once
#include <mutex>
#include <memory>

template <class T>
class CSingletonShared {
private:
	static inline std::mutex singleton_mutex_{}; // singleton mutex
	static inline std::shared_ptr<T> instance_{ nullptr }; // conv. to shared_ptr because of enable_shared_from_this classes
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
	void ResetSingleton() noexcept {
		if (!instance_) return;
		std::lock_guard<std::mutex> lock(singleton_mutex_);
		instance_.reset();
	}
public:
	CSingletonShared()=default;
	CSingletonShared& operator=(CSingletonShared&& other) = delete;
	CSingletonShared(CSingletonShared&& other) = delete;
	CSingletonShared(const CSingletonShared&) = delete;
	CSingletonShared& operator=(const CSingletonShared&) = delete;
};

template <class T>
class CSingleton {
private:
	static inline std::mutex singleton_mutex_{}; // singleton mutex
	static inline std::unique_ptr<T> instance_{ nullptr }; // conv. to shared_ptr because of enable_shared_from_this classes
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
	void ResetSingleton() noexcept {
		if (!instance_) return;
		std::lock_guard<std::mutex> lock(singleton_mutex_);
		instance_.reset();
	}
public:
	CSingleton()=default;
	CSingleton& operator=(CSingleton&& other) = delete;
	CSingleton(CSingleton&& other) = delete;
	CSingleton(const CSingleton&) = delete;
	CSingleton& operator=(const CSingleton&) = delete;
};
