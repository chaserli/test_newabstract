#pragma once
#include <SwizzleManagerClass.h>

class AresSwizzle
{
public:
	template<typename T>
	inline static HRESULT RegisterChange(T* was, T* is)
	{
		return SwizzleManagerClass::Instance->Here_I_Am((long)was, is);
	}

	template <typename T>
	inline static void RegisterPointerForChange(T*& ptr)
	{
		SwizzleManagerClass::Instance->Swizzle(reinterpret_cast<void**>(&ptr));
	}
};
