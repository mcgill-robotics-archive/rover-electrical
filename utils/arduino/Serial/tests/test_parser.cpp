#include "SerialParser.hpp"
#include <cassert>
#include <stdio.h>

#include <cerrno>
#include <iostream>

void test_int()
{
	Message m;
	m.data = "R300 X21.10 Y0.122 Z50";
	Variant v = parse_message(m, 'R');
	assert(v.type == Variant::TYPE_INTEGER);
	assert(v.as_int == 300);

	v = parse_message(m, 'Z');
	assert(v.type == Variant::TYPE_INTEGER);
	assert(v.as_int == 50);
	printf("Integer passed\n");
}

void test_double()
{
	Message m;
	m.data = "R300 X21.10 Y0.122";
	Variant v = parse_message(m, 'X');
	assert(v.type == Variant::TYPE_DOUBLE);
	assert(v.as_double == 21.10);

	v = parse_message(m, 'Y');
	assert(v.type == Variant::TYPE_DOUBLE);
	assert(v.as_double == 0.122);
	printf("Double passed\n");
}

void test_invalid()
{
	Message m;
	m.data = "R300 X21.10 Y0.122";
	Variant v = parse_message(m, 'Z');
	assert(v.type == Variant::TYPE_INVALID);

	m = Message();
	v = parse_message(m, 'R');
	assert(v.type == Variant::TYPE_INVALID);

	m.data = "R300X12.3 Y456";
	v = parse_message(m, 'R');
	assert(v.type == Variant::TYPE_INVALID);
	printf("Invalid passed\n");
}

int main()
{
	test_int();
	test_double();
	test_invalid();
	printf("All parser tests passed\n");
}