#include "LogUtil.h"
#include <DxLib.h>
#include <cstdarg>
#include <cstdio>

namespace utility
{
	void LogUtil::debug(const char* format, ...)
	{
#ifdef _DEBUG
		va_list args;
		va_start(args, format);
		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), format, args);
		va_end(args);
		printfDx("%s", buffer);
#endif
	}

	void LogUtil::clear()
	{
#ifdef _DEBUG
		clsDx();
#endif
	}
}