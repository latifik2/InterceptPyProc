#pragma once
#include <iostream>
#include <string>
#include <Windows.h>

namespace dbg
{
	class Debug
	{
	public:
		Debug(std::string optitonalMessage);
		static void GetLastErrorAsString(std::string optitonalMessage);
	private:
		std::string optitonalMessage;
	};
}

