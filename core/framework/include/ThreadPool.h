// Copyright (c) 2025 kong9812
#pragma once
#include <type_traits>
#include <future>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>
#include <memory>
#include <atomic>

class ThreadPool
{
public:
	static ThreadPool& Instance()
	{
		static ThreadPool instance;
		return instance;
	}

	// ジョブキューに追加
	template<class F, class... Args>
	auto enqueue(F&& f, Args&&... args)
		-> std::future<std::invoke_result_t<F, Args...>>
	{
		// 関数FにArgs...を渡した時の戻り値型
		using return_type = std::invoke_result_t<F, Args...>;

		// 関数とfutureを結びつけるラッパ & 引数なし関数に変換
		auto task = std::make_shared<std::packaged_task<return_type()>>
			(
				std::bind(std::forward<F>(f), std::forward<Args>(args)...)
			);

		// 非同期実行結果を受け取る
		std::future<return_type> result = task->get_future();

		{
			// ジョブ追加
			std::unique_lock<std::mutex> lock(this->m_queueMutex);
			// 停止チェック
			if (m_stop || m_force_stop)
			{
				throw std::runtime_error("Enqueue on stopped thread pool.");
			}
			// ジョブキューにラムダ追加
			m_jobs.emplace([task]() { (*task)(); });
		}

		m_condition.notify_one();
		return result;
	}

	// 終了
	void Shutdown()
	{
		{
			std::unique_lock<std::mutex> lock(this->m_queueMutex);
			m_force_stop = true;
		}
		m_condition.notify_all();

		// 全ワーカーの終了待ち
		for (auto& worker : m_workers)
		{
			if (worker.joinable())
			{
				worker.join();
			}
		}
	}

	~ThreadPool()
	{
		Shutdown();
	}

	// コピー・代入禁止
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;
	ThreadPool(ThreadPool&&) = delete;
	ThreadPool& operator=(ThreadPool&) = delete;

private:
	// ワーカースレッド生成
	ThreadPool() : m_stop(false), m_force_stop(false)
	{
		for (size_t i = 0; i < std::thread::hardware_concurrency(); i++)
		{
			// ワーカースレッドを生成し、workersに保管
			m_workers.emplace_back([this]
				{
					while (true)
					{
						std::function<void()> job;	// 実行するジョブ
						{
							std::unique_lock<std::mutex> lock(this->m_queueMutex);

							// stopまたはforce_stop、またはジョブが来るまで待つ
							this->m_condition.wait(lock, [this]
								{
									return this->m_stop || this->m_force_stop || !this->m_jobs.empty();
								});

							// 強制終了フラグが立っていれば即座にスレッド終了
							if (this->m_force_stop) return;

							// stopフラグやジョブがないなら終了
							if (this->m_stop && this->m_jobs.empty()) return;

							// ジョブがあれば取り出す
							if (!this->m_jobs.empty())
							{
								job = std::move(this->m_jobs.front());
								this->m_jobs.pop();
							}
							else
							{
								// ジョブがなければ再度待機
								continue;
							}
						}
						job();
					}
				}
			);
		}
	}

	std::vector<std::thread> m_workers;
	std::queue<std::function<void()>> m_jobs;
	std::mutex m_queueMutex;
	std::condition_variable m_condition;
	bool m_stop;
	std::atomic<bool> m_force_stop;
};