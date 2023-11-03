#ifndef LOCK_FREE_FLAG_H
#define LOCK_FREE_FLAG_H
#include <atomic>

class LockFreeFlag
{
public:
	LockFreeFlag() :
		m_flag(false)
	{
	}

	bool operator==(bool value) const
	{
		return m_flag.load(std::memory_order_acquire) == value;
	}

	LockFreeFlag& operator=(bool value)
	{
		m_flag.store(value, std::memory_order_release);
		return *this;
	}

	explicit operator bool() const
	{
		return *this == true;
	}

private:
	std::atomic<bool> m_flag;
};
#endif // LOCK_FREE_FLAG_H
