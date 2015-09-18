#include <stdio.h>
#include <algorithm>
#include <cctype>
#include "NWSys.h"

bool is_number(const std::string& s)
{
	return !s.empty() && std::find_if(s.begin(), 
		s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

//argc - count; argv - value 
int main(int argc, char *argv[])
{
	NWSys* System;

	System = new NWSys;
	if (!System)
		return 0;

	float tickRate = 60;

	if (argc > 1 && is_number(argv[1]))
		tickRate = atoi(argv[1]);
	else
	{
		cout << "Enter tickrate for the server : ";
		cin >> tickRate;
	}

	if (!System->Init(tickRate))
		return 0;

	while (System->Online())
	{
		System->Update();
	}

	delete System;
	System = 0;

	return 0;
}