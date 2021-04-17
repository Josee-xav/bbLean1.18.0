#pragma once

struct JobManager;
struct JobWorkerThread : WorkerThread
{
	JobManager & m_jobmanager;
	unsigned m_sleep_counter;
	JobWorkerThread (JobManager & mgr) : m_jobmanager(mgr), m_sleep_counter(0) { }
	JobManager & GetJobManager () { return m_jobmanager; }
	void Run ();
};

struct JobManager
{
	std::atomic<bool> m_readyQueueLock;
	typedef Runnable * jobptr_t;
	std::vector<jobptr_t> m_readyQueue;
	unsigned m_sleep_counter;
	bool m_terminating;
	std::vector<JobWorkerThread *> m_workers;
	ThreadPool m_pool;

	bool AddJob (jobptr_t j)
	{
		if (m_terminating)
			return false;

		bool lock_expected = false;
		while (!m_readyQueueLock.compare_exchange_weak(lock_expected, true) && lock_expected != false)
			;

		m_readyQueue.push_back(j);

		bool unlock_expected = true;
		while (!m_readyQueueLock.compare_exchange_weak(unlock_expected, false) && unlock_expected != true)
			;

		return true;
	}

	jobptr_t AcquireJob ()
	{
		jobptr_t job = 0;

		bool lock_expected = false;
		while (!m_readyQueueLock.compare_exchange_weak(lock_expected, true) && lock_expected != false)
			;
		//while (atomic::atomic_cas32(&m_readyQueueLock, 1, 0) != 0)
		//		;

		if (!m_readyQueue.empty())
		{
			job = m_readyQueue.back();
			m_readyQueue.pop_back();
		}

		bool unlock_expected = true;
		while (!m_readyQueueLock.compare_exchange_weak(unlock_expected, false) && unlock_expected != true)
			;
		//while (atomic::atomic_cas32(&m_readyQueueLock, 0, 1) != 1)
		//		;

		return job;
	}

	JobManager ()
		: m_readyQueueLock(0)
		, m_sleep_counter(0)
		, m_terminating(0)
		, m_pool()
	{ }

	void Create (size_t n = 1)
	{
		for (size_t i = 0; i < n; ++i)
		{
			m_workers.push_back(new JobWorkerThread(*this));
			m_pool.Create(*m_workers[i]);
		}
	}

	size_t size () { return m_pool.size(); }

	void Stop ()
	{
		m_terminating = true;
		SetQuit();
		m_pool.WaitForTerminate();
		Reset();
	}

	void Reset ()
	{
		m_pool.clear();
		m_workers.clear();
		m_sleep_counter = 0;
		m_readyQueueLock.store(false, std::memory_order_relaxed);
		m_terminating = false;
	}

	void SetQuit ()
	{
		for (size_t i = 0, ie = m_workers.size(); i < ie; ++i)
			m_workers[i]->SetTerminate();
	}
};

inline void JobWorkerThread::Run ()
{
	while (!GetTerminate())
	{
		if (JobManager::jobptr_t job = GetJobManager().AcquireJob())
			job->Run();
		delayExecution(m_sleep_counter);
	}
}


