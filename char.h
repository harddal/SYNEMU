#pragma once

#include <array>

typedef std::bitset<6> vm_char;

const std::array<char, 63> char_lookup =
{
	'1','2','3','4','5',
	'6','7','8','9','0',
	' ',' ',' ',' ','b',
	'n','e','c','\0','|',
	' ',' ',' ',' ',
	' ',' ',' ',' ',' ',
	' ',' ','A','B','C',
	'D','E','F','G','H',
	'I','J','K','L','M',
	'N','O','P','Q','R',
	'S','T','U','V','W',
	'X','Y','Z','.',',',
	'\'',' ','!','?'
};

const std::array<std::string, 63> char_bin_lookup =
{
	"000001","000010","000011","000100",
	"000101","000110","000111","001000",
	"001001","001010","001011","001100",
	"001101","001110","001111","010000",
	"010001","010010","010011","010100",
	"010101","010110","010111","011000",
	"011001","011010","011011","011100",
	"011101","011110","011111","100000",
	"100001","100010","100011","100100",
	"100101","100110","100111","101000",
	"101001","101010","101011","101100",
	"101101","101110","101111","110000",
	"110001","110010","110011","110100",
	"110101","110110","110111","111000",
	"111001","111010","111011","111100",
	"111101","111110","111111"
};

class character
{
	std::map<std::string, char> enumMap;

public:
	character();

	char CompareValue(std::string &value)
	{
		std::map<std::string, char>::const_iterator iValue = enumMap.find(value);

		if (iValue == enumMap.end())
		{
			iValue = enumMap.find("010011");

			return iValue->second;
		}

		return iValue->second;
	}
};

character::character()
{
	for (int i = 0U; i < 63; i++)
		enumMap[char_bin_lookup[i]] = char_lookup[i];
}
