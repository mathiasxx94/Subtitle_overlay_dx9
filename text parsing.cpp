#include "text parsing.h"

std::wstring FromUTF8(const char* str)
{
	const unsigned char* s = reinterpret_cast<const unsigned char*>(str);

	static const wchar_t badchar = '?';

	std::wstring ret;

	unsigned i = 0;
	while (s[i])
	{
		try
		{
			if (s[i] < 0x80)         // 00-7F: 1 byte codepoint
			{
				ret += s[i];
				++i;
			}
			else if (s[i] < 0xC0)    // 80-BF: invalid for midstream
				throw 0;
			else if (s[i] < 0xE0)    // C0-DF: 2 byte codepoint
			{
				if ((s[i + 1] & 0xC0) != 0x80)		throw 1;

				ret += ((s[i] & 0x1F) << 6) |
					((s[i + 1] & 0x3F));
				i += 2;
			}
			else if (s[i] < 0xF0)    // E0-EF: 3 byte codepoint
			{
				if ((s[i + 1] & 0xC0) != 0x80)		throw 1;
				if ((s[i + 2] & 0xC0) != 0x80)		throw 2;

				wchar_t ch =
					((s[i] & 0x0F) << 12) |
					((s[i + 1] & 0x3F) << 6) |
					((s[i + 2] & 0x3F));
				i += 3;

				// make sure it isn't a surrogate pair
				if ((ch & 0xF800) == 0xD800)
					ch = badchar;

				ret += ch;
			}
			else if (s[i] < 0xF8)    // F0-F7: 4 byte codepoint
			{
				if ((s[i + 1] & 0xC0) != 0x80)		throw 1;
				if ((s[i + 2] & 0xC0) != 0x80)		throw 2;
				if ((s[i + 3] & 0xC0) != 0x80)		throw 3;

				unsigned long ch =
					((s[i] & 0x07) << 18) |
					((s[i + 1] & 0x3F) << 12) |
					((s[i + 2] & 0x3F) << 6) |
					((s[i + 3] & 0x3F));
				i += 4;

				// make sure it isn't a surrogate pair
				if ((ch & 0xFFF800) == 0xD800)
					ch = badchar;

				if (ch < 0x10000)	// overlong encoding -- but technically possible
					ret += static_cast<wchar_t>(ch);
				else if (WCHAR_MAX < 0x110000)
				{
					// wchar_t is too small for 4 byte code point
					//  encode as UTF-16 surrogate pair

					ch -= 0x10000;
					ret += static_cast<wchar_t>((ch >> 10) | 0xD800);
					ret += static_cast<wchar_t>((ch & 0x03FF) | 0xDC00);
				}
				else
					ret += static_cast<wchar_t>(ch);
			}
			else                    // F8-FF: invalid
				throw 0;
		}
		catch (int skip)
		{
			if (!skip)
			{
				do
				{
					++i;
				} while ((s[i] & 0xC0) == 0x80);
			}
			else
				i += skip;
		}
	}

	return ret;
}

std::pair<float, float> toSeconds(std::wstring input)
{
	int minutes = std::stoi(input.substr(3, 2));
	int seconds = std::stoi(input.substr(6, 2));
	int hundreds = std::stoi(input.substr(9, 2));

	float finalsecsfirst = minutes * 60.f + seconds + hundreds / 100.f;

	minutes = std::stoi(input.substr(20, 2));
	seconds = std::stoi(input.substr(23, 2));
	hundreds = std::stoi(input.substr(26, 2));

	float finalsecssecond = minutes * 60.f + seconds + hundreds / 100.f;

	return std::make_pair(finalsecsfirst, finalsecssecond);
}

void fillVector()
{
	std::string Line;
	std::wstring wLine;
	std::ifstream subtitlefile("C://Users//mathi//Desktop//TERRACE.HOUSE//Terracehouse_s02e09_ja.vtt");
	
	while (getline(subtitlefile, Line))
	{
		timeSubpacket packet; //declare new inside loop to prevent data from previous subtitles with more lines than one.

		wLine = FromUTF8(Line.c_str());
		packet.currframe = std::stoi(Line.substr(0, 4)) - 1; 
		getline(subtitlefile, Line); //This should be the time slot of each packet
		wLine = FromUTF8(Line.c_str());

		std::pair<float, float> timeslotinsec = toSeconds(wLine);
		packet.secStart = timeslotinsec.first;
		packet.secEnd = timeslotinsec.second;

		getline(subtitlefile, Line); //This is first subtitle line, no check needed for blank line
		wLine = FromUTF8(Line.c_str());
		packet.subline1 = wLine;
		packet.sublines = 1;

		getline(subtitlefile, Line); //This is second subtitle line, or blank line
		wLine = FromUTF8(Line.c_str());

		if (wLine.length() == 0)
		{
			subtitlePacket.push_back(packet);
			continue;
		}
		else
		{
			packet.subline2 = wLine;
			packet.sublines = 2;
		}

		getline(subtitlefile, Line); //This is third subtitle line, or blank line
		wLine = FromUTF8(Line.c_str());

		if (wLine.length() == 0)
		{
			subtitlePacket.push_back(packet);
			continue;
		}
		else
		{
			packet.subline3 = wLine;
			packet.sublines = 3;
		}
		subtitlePacket.push_back(packet);
		getline(subtitlefile, Line); //To get correct parser alignment when three subtitle lines
	}

	subtitlefile.close();
}

