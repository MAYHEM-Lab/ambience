#pragma once

namespace tos
{
	template <class T>
	class atomic
	{
	public:
		atomic(const T& t) : m_t(t) {}
		
		void add(const T& t);

		atomic& operator++();
		atomic& operator--();

		operator T();
	private:

		volatile T m_t;
	};
}

namespace tos
{
	template <class T>
	atomic<T>& atomic<T>::operator++()
	{
		add(1);
		return *this;
	}

	template <class T>
	atomic<T>& atomic<T>::operator--()
	{
		add(-1);
		return *this;
	}

	template <class T>
	atomic<T>::operator T()
	{
		return m_t;
	}
}