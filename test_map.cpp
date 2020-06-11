// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <cstring>
#include <iostream>
#include "struct2map.h"
using namespace std;
using namespace struct2map_interface;




struct MyClass
{
	uint8_t a;
	uint16_t b;
	uint32_t c;
	uint64_t d;
	int32_t e;
	int64_t f;
	double g;
	char h[10];
	STRUCT2MAP("MyClass", a, b, c, d, e, f, g, h);
};

int main()
{
	// Open a Connection with *test.db*
	struct2map mapper;

	// Create a table for "MyClass"
	MyClass abc;
	std::map<std::string, std::string> result1;
	std::map<std::string, std::string> result2;
	abc.a = 1;
	abc.b = 1;
	abc.c = 1;
	abc.d = 1;
	abc.e = 1;
	abc.f = 1;
	abc.g = 1.1;
	strcpy_s(abc.h, "aaa");

	mapper.to_map(abc, result1);
	
	mapper.to_map(abc, result2, [](std::string k, std::string v) {return v+"_v";});
	return 0;
}
