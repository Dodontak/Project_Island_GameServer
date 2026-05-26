#pragma once

#include <functional>

class Job
{
public:
	Job(function<void()>&& callback) : _callback(move(callback)) {}

	template<typename T, typename Ret, typename... Args, typename... ActualArgs>
	Job(shared_ptr<T> owner, Ret(T::* memFunc)(Args...), ActualArgs&&... args)
	{
		_callback = [owner, memFunc, ...args = std::forward<ActualArgs>(args)]()
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

