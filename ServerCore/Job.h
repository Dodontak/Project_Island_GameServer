#pragma once

#include <functional>

class Job
{
public:
	Job(function<void()>&& callback) : _callback(move(callback)) {}

	template<typename T, typename Ret, typename... Args>
	Job(shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args&&... args)
	{
		_callback = [owner, memFunc, args...]()
			{
				(owner.get()->*memFunc)(args...);
			};
	}

	void Execute()
	{
		_callback();
	}

private:
	function<void()> _callback;
};

