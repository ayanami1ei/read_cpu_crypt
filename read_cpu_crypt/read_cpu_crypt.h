#pragma once
#include <windows.h>
#include <intrin.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <comdef.h>
#include <Wbemidl.h>
#include <fstream>

#ifdef BUILD_DLL
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

class DLL_API read_cpu_crypt
{
public:
	std::string hash(const std::string& input);

	std::string GetBoardSerial();

	std::string GetCpuSerial();
private:
	class Ipmi;
	Ipmi* p;
};
