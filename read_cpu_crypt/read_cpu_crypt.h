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
	* ��ϣ����
	* 
	* @param input �����ַ���
	* 
	* @return ���ع�ϣֵ
	*/
	std::string hash(const std::string& input);

	/*
	* ��ȡ�������к�
	* 
	* @return �����������к�
	*/
	std::string GetBoardSerial();

	/*
	* ��ȡCPU���к�
	* 
	* @return ����CPU���к�
	*/
	std::string GetCpuSerial();
private:
	class Ipmi;
	Ipmi* p;
};
