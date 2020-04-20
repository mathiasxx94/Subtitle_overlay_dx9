#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <utility>


struct timeSubpacket
{
	float secStart;
	float secEnd;
	std::wstring subline1;
	std::wstring subline2;
	std::wstring subline3;
	int currframe;
	int sublines;

	int lineHeight;
	int longestWidth;
	int line1Width;
	int line2Width;
	int line3Width;
};


extern std::vector<timeSubpacket> subtitlePacket;

std::wstring FromUTF8(const char* str);
void fillVector();
