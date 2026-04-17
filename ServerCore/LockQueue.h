#pragma once

template<typename T>
class LockQueue
{
public:
	void Push(T item)
	{
		lock_guard<mutex> lock(_m);
		_items.push(item);
	}

	T Pop()
	{
		lock_guard<mutex> lock(_m);
		if (_items.empty())
			return T();

		T ret = _items.front();
		_items.pop();
		return ret;
	}

	T PopNoLock()
	{
		if (_items.empty())
			return T();

		T ret = _items.front();
		_items.pop();
		return ret;
	}

	void PopAll(OUT vector<T>& items)
	{
		lock_guard<mutex> lock(_m);
		while (T item = PopNoLock())
			items.push_back(item);
	}

	void Clear()
	{
		lock_guard<mutex> lock(_m);
		_items = queue<T>();
	}

private:
	mutex _m;
	queue<T> _items;
};