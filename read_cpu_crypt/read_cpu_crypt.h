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

class  read_cpu_crypt
{
public:
	/*
	* 哈希函数
	* 
	* @param input 输入字符串
	* 
	* @return 返回哈希值
	*/
	std::string hash(const std::string& input);

	/*
	* 获取主板序列号
	* 
	* @return 返回主板序列号
	*/
	std::string GetBoardSerial();

	/*
	* 获取CPU序列号
	* 
	* @return 返回CPU序列号
	*/
	std::string GetCpuSerial();
private:
	class Ipmi;
	Ipmi* p;
};
