#include "LogUtil.h"
#include <DxLib.h>
#include <cstdio>

namespace infrastructure::utility
{
	int LogUtil::s_line = 0;

	void LogUtil::print(int r, int g, int b, const char* message)
	{
		SetDrawBright(r, g, b);
		printfDx("%s\n", message);
		SetDrawBright(LOG_COLOR_R, LOG_COLOR_G, LOG_COLOR_B);
		s_line++;
	}

	void LogUtil::log(const char* message)
	{
#ifdef _DEBUG
		print(LOG_COLOR_R, LOG_COLOR_G, LOG_COLOR_B, message);
#endif
	}

	void LogUtil::warning(const char* message)
	{
#ifdef _DEBUG
		print(WARNING_COLOR_R, WARNING_COLOR_G, WARNING_COLOR_B, message);
#endif
	}

	void LogUtil::error(const char* message)
	{
#ifdef _DEBUG
		print(ERROR_COLOR_R, ERROR_COLOR_G, ERROR_COLOR_B, message);
#endif
	}

	void LogUtil::clear()
	{
#ifdef _DEBUG
		clsDx();
		s_line = 0;
#endif
	}
}