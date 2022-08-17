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

		static T* Get() { return m_instance; }

	protected:
		static T* m_instance;
	};
}

#define IMPLEMENT_SINGLETON(T) T* Singleton<T>::m_instance = nullptr;


#endif // SINGLETON_H
