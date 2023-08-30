#ifndef SINGLETON_H
#define SINGLETON_H

namespace JoyEngine
{
	template <typename T>
	class Singleton
	{
	public:
		Singleton()
		{
			m_instance = static_cast<T*>(this);
		}

		static inline T* Get() { return m_instance; }

	protected:
		static inline T* m_instance;
	};
}

#endif // SINGLETON_H
