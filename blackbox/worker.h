#pragma once
#include <atomic>
#include <thread>
#include <vector>

struct Runnable
{
	virtual void Run () = 0;
};

struct ThreadPool
{
	std::vector<std::thread> m_threads;

	ThreadPool () { }

	size_t size () const { return m_threads.size(); }
	void clear () { m_threads.clear(); }

	void Create (Runnable & runnable)
	{
		m_threads.push_back(std::thread(&Runnable::Run, std::ref(runnable)));
	}

	void WaitForTerminate ()
	{
		for (std::thread & t : m_threads)
			t.join();
		m_threads.clear();
	}
};

inline void delayExecution (unsigned & counter)
{
	if (counter < 10)
		_mm_pause();
	else if (counter < 20)
		for (unsigned i = 0; i < 50; ++i)
			_mm_pause();
	else if (counter < 22)
		SwitchToThread();
	else if (counter < 24)
		Sleep(0);
	else if (counter < 26)
		Sleep(1);
	else
		Sleep(10);

	counter += 1;
}

class WorkerThread : public Runnable
{
	std::atomic<bool> m_terminate;

	virtual void Run () = 0;

public:
	WorkerThread () : m_terminate(false) { }

	bool GetTerminate () const
	{
		return m_terminate.load(std::memory_order_relaxed);
	}

	void SetTerminate ()
	{
		m_terminate.store(true, std::memory_order_relaxed);
	}
};

