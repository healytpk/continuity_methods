// xxthreadpool internal header
#ifndef _XXTHREADPOOL_
#define _XXTHREADPOOL_

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <cstdlib>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

_STD_BEGIN
		// TEMPLATE CLASS _Ws_deque
template<class _Ty,
	size_t _Mysize = 1024>
class _WS_deque
	{	// window into an array
	typedef int_fast64_t _Idx_t;

public:
	_WS_deque()
		: _Bottom(0), _Top(0)
		{	// default construct
		}

	_Idx_t _Size() const _NOEXCEPT
		{	// get current element count
		return ((_Bottom - _Top) % _Mysize);
		}

	void _Push(_Ty _Item) _NOEXCEPT
		{	// push an item
		assert(_Size() < _Mysize);
		_Idx_t _Tbottom = _Bottom;
		_Array[_Tbottom % _Mysize] = _Item;
		_Bottom = _Tbottom + 1;
		}

	_Ty _Pop() _NOEXCEPT
		{	// pop an item
		_Idx_t _Tbottom = _Bottom;
		--_Tbottom;
		_Bottom = _Tbottom;

		_Idx_t _Ttop = _Top;
		_Idx_t _Tsize = _Tbottom - _Ttop;

		if (_Tsize < 0)
			{	// make it empty
			_Bottom = _Ttop;
			return (0);
			}
		_Ty _Res = _Array[_Tbottom % _Mysize];
		if (0 < _Tsize)
			return (_Res);
		if (!_Top.compare_exchange_strong(_Ttop, _Ttop + 1))
			_Res = 0;
		_Bottom = _Ttop + 1;
		return ( _Res);
		}

	_Ty _Steal() _NOEXCEPT
		{	// steal a task
		_Idx_t _Ttop = _Top;
		_Idx_t _Tbottom = _Bottom;
		_Idx_t _Tsize = _Tbottom - _Ttop;
		if (_Tsize <= 0)
			return (0);
		_Ty _Res = _Array[_Ttop % _Mysize];
		if (!_Top.compare_exchange_strong(_Ttop, _Ttop + 1))
			_Res = 0;
		return (_Res);
		}

private:
	_Ty _Array[_Mysize];
	atomic<_Idx_t> _Bottom;
	atomic<_Idx_t> _Top;
	};

class _Pool_worker
	{	// stores work information
	friend class _Pool_task;
	friend class _Threadpool;

public:
	_Pool_worker();
	void _Add_task(_Pool_task *_Tsk);

private:
	void _Run_loop();
	void _Push_task(_Pool_task *_Tsk);
	void _Wait_for_task();
	_Pool_task *_Get_task();

	thread _Thr;
	_WS_deque<_Pool_task *, 2> _Tasks;
	mutex _MtxW;
	condition_variable _CndW;
	bool _Queue_is_empty;

 #if _INCLUDE_POOL_STATISTICS
	/* Statistics */
public:
	int _Get_task_count() const;
	int _Get_max_tasks() const;
	int _Get_steal_count() const;
	int _Get_failed_steal_count() const;
	void _Clear_counters();

private:
	int _Task_count;
	int _Max_tasks;
	atomic<int> _Steal_count;
	atomic<int> _Failed_steal_count;
 #endif	/* _INCLUDE_POOL_STATISTICS */
	};

		// CLASS _Thread_pool
class _Threadpool
	{	// pool of threads
	friend class _Pool_worker;

public:
	_Threadpool();
	void _Add_task(_Pool_task *_Tsk);

	int _Get_thread_count() const
		{	// get number of worker threads
		return (_Workers.size());
		}

private:
	void _Add_to_blocked(_Pool_worker *_Wrkr);
	_Pool_task *_Steal_task();

	mutex _MtxT;
	condition_variable _CndT;
	deque<_Pool_task *> _Waiting_tasks;
	deque<_Pool_worker *> _Blocked_workers;
	int _Blocked_worker_count;
	int _Worker_count;
	vector<_Pool_worker> _Workers;	// must be after _Worker_count.

 #if _INCLUDE_POOL_STATISTICS
public:
	static void _Show_and_clear_counters();
	static int _Get_task_count_and_clear_counters();

private:
	int _Do_get_task_count_and_clear_counters();
	void _Do_show_and_clear_counters();
	int _Queue_attempts;
	int _Queue_successes;
 #endif	/* _INCLUDE_POOL_STATISTICS */
	};
_STD_END
#endif	/* _XXTHREADPOOL_ */

/*
 * Copyright (c) by P.J. Plauger. All rights reserved.
 * Consult your license regarding permissions and restrictions.
 * V8.05/17:1422 */
