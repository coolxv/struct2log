#include <cstring>
#include <iostream>

#include "struct2log.h"
using namespace std;
using namespace struct2log_interface;

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

	STRUCT2LOG (MyClass,a,b,c,d,e,f,g,h);
};

int main ()
{
	// Open a Connection with *test.db*
	struct2log mapper;

	// Create a table for "MyClass"
	MyClass abc;
	mapper.create_tbl (abc);
	abc.a=1;
	abc.b=1;
	abc.c=1;
	abc.d=1;
	abc.e=1;
	abc.f=1;
	abc.g=1.1;
	strcpy(abc.h,"aaa");
	mapper.insert_row(abc);
	strcpy(abc.h,"bbb");
	mapper.update_row(abc);
	mapper.delete_row(abc);

	return 0;
}
