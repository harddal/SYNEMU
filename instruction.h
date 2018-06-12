//
// Note: NOP can be used to store constants
//

#pragma once

#include <array>
#include <bitset>
#include <map>
#include <string>
#include <vector>

typedef std::bitset<5> vm_opcode;

enum instruction_set
{
	NOP, DB,   SR,
	LD,	 MOV,  ADD,
	SUB, SUBC, INC,
	DEC, RSH,  LSH,
	NOT, NAND, XOR,
	OR,  AND,  DCO,
	DCR, DCM,  DCP,
	LDP, SP,   JNE,
	JE,  JMP,  ICP,
	DIR, DIM,  DIP,
	IIP, RST
};

const std::array<const std::string, 32> instruction_asm_lookup =
{
	"NOP", "DB",   "SR",
	"LD",  "MOV",  "ADD",
	"SUB", "SUBC", "INC",
	"DEC", "RSH",  "LSH",
	"NOT", "NAND", "XOR",
	"OR",  "AND",  "DCO",
	"DCR", "DCM",  "DCP",
	"LDP", "SP",   "JNE",
	"JE",  "JMP",  "ICP",
	"DIR", "DIM",  "DIP",
	"IIP", "RST"
};

const std::array<const std::string, 32> instruction_bin_lookup =
{
	"00000","00001","00010",
	"00011","00100","00101",
	"00110","00111","01000",
	"01001","01010","01011",
	"01100","01101","01110",
	"01111","10000","10001",
	"10010","10011","10100",
	"10101","10110","10111",
	"11000","11001","11010",
	"11011","11100","11101",
	"11110","11111"
};

class mneumonic
{
	std::map<std::string, std::string> enumMap;

public:
	mneumonic();

	std::string CompareValue(std::string &value)
	{
		std::map<std::string, std::string>::const_iterator iValue = enumMap.find(value);

		if (iValue == enumMap.end())
		{
			iValue = enumMap.find("RST");

			return iValue->second;
		}

		return iValue->second;
	}
};

mneumonic::mneumonic()
{
	for (int i = 0U; i < 32; i++)
		enumMap[instruction_bin_lookup[i]] = instruction_asm_lookup[i];
}
