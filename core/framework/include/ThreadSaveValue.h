// Copyright (c) 2025 kong9812
#pragma once
#include <mutex>

template<typename T>
class ThreadSaveValue
{
public:
	ThreadSaveValue(const T& v) {
		Set(v);
	}

	T Get() const {
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_value;
	}

	void Set(const T& v) {
		std::lock_guard<std::mutex> lock(m_mutex);
		m_value = v;
	}

private:
	T m_value;
	mutable std::mutex m_mutex;
};