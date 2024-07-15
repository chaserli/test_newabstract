#pragma once
#include <SwizzleManagerClass.h>

class AresSwizzle
{
public:
	template<typename T>
	inline static HRESULT RegisterChange(void* was, T* is)
	{
		return SwizzleManagerClass::Instance->Here_I_Am((long)was, is);
	}

	template <typename T>
	inline static void RegisterPointerForChange(T*& ptr)
	{
		auto pptr = const_cast<std::remove_cv_t<T>**>(&ptr);
		SwizzleManagerClass::Instance->Swizzle(reinterpret_cast<void**>(pptr));
	}
};

struct MSwizzle {
	template <typename T> requires std::is_pointer_v<T>
	MSwizzle(T& object)
	{
		AresSwizzle::RegisterPointerForChange(object);
	}
};
