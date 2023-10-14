#pragma once

template<class T>
class TOD_Singleton
{
public:
	static T& GetInstance()
	{
		static T instance;

		return instance;
	}

protected:
	TOD_Singleton() = default;

private:
	TOD_Singleton(const TOD_Singleton&) = delete;
	TOD_Singleton(TOD_Singleton&&) = delete;
	TOD_Singleton& operator= (TOD_Singleton&) = delete;
	TOD_Singleton& operator= (const TOD_Singleton&&) = delete;
};