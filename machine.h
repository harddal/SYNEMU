#pragma once

#include <algorithm>
#include <array>
#include <bitset>
#include <chrono>
#include <iostream>
#include <fstream>
#include <map>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>

#include "char.h"
#include "instruction.h"

#include <Windows.h>

#define _reg_size 10
#define _ram_size 255
#define _rom_size 889

// 2 Hz 
#define _execution_delay 500

#define _opcode 0, 5
#define _arga 5, 12
#define _argb 17

typedef std::bitset<12> vm_uint;
typedef std::bitset<29> vm_byte;

struct cpu
{
	cpu()
	{
		std::ifstream file;
		file.open("fibonacci.sc");

		auto i = 0U;
		std::string line;

		while (std::getline(file, line))
			rom[i++] = vm_byte(line);
	}

	bool halt = false;

	unsigned int pc = 0;

	std::array<vm_byte, _reg_size> reg;

	std::array<vm_byte, _ram_size> ram;
	std::array<vm_byte, _rom_size> rom;
};

using std::chrono::high_resolution_clock;
class machine
{
private:
	high_resolution_clock m_cpuClock;
	high_resolution_clock::time_point m_lastCycle;

	std::vector<std::string> m_outputBuffer;

	cpu m_cpu;

public:
	void manualHalt(bool state) {
		m_cpu.halt = state;
	}

	bool isHalted() {
		return m_cpu.halt;
	}

	void bufferOutput(char data) {
		m_outputBuffer.push_back(std::to_string(data));
	}
	void bufferOutput(unsigned long data) {
		m_outputBuffer.push_back(std::to_string(data));
	}
	void bufferOutput(vm_byte data) {
		// todo
	}

	// debug
	void bufferOutput(std::string data) {
		m_outputBuffer.push_back(data);
	}

	void getOutputBufferData(std::stringstream &sstream) {
		if (m_outputBuffer.size() > 0)
			std::copy(m_outputBuffer.begin(), m_outputBuffer.end(), std::ostream_iterator<std::string>(sstream));
	}

	cpu getMachineDebugData() {
		return m_cpu;
	}

	cpu &getMachineData() {
		return m_cpu;
	}

	unsigned int run()
	{
		m_outputBuffer = {};

		Sleep(_execution_delay);// TODO: if (m_cpuClock.now() - m_lastCycle >= std::chrono::milliseconds(_execution_delay))
		{
			//m_lastCycle = m_cpuClock.now();

			if (m_cpu.pc > _rom_size)
			{
				m_cpu.halt = true;
				std::cerr << " \n[error] Program counter out of bounds\n";
			}

			if (!m_cpu.halt)
			{
				switch (vm_opcode(m_cpu.rom[m_cpu.pc].to_string().substr(_opcode)).to_ulong())
				{
				case instruction_set::NOP:
					// Do nothing...
					m_cpu.pc++;

					break;

				case instruction_set::DB:
					try {
						m_cpu.ram.at(vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_arga)).to_ulong()) =
							vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_argb));
					}
					catch (const std::out_of_range& err) {
						std::cerr << " \n[error] " << err.what() << " in " << __FILE__ << ":" << __LINE__ << '\n';
						m_cpu.halt = true;
					}

					m_cpu.pc++;
					break;

				case instruction_set::SR:
					try {
						m_cpu.ram.at(vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_arga)).to_ulong()) =
							m_cpu.reg.at(vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_argb)).to_ulong());
					}
					catch (const std::out_of_range& err) {
						std::cerr << " \n[error] " << err.what() << " in " << __FILE__ << ":" << __LINE__ << '\n';
						m_cpu.halt = true;
					}

					m_cpu.pc++;
					break;

				case instruction_set::LD:
					try {
						m_cpu.reg.at(vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_argb)).to_ulong()) =
							m_cpu.ram.at(vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_arga)).to_ulong());
					}
					catch (const std::out_of_range& err) {
						std::cerr << " \n[error] " << err.what() << " in " << __FILE__ << ":" << __LINE__ << '\n';
						m_cpu.halt = true;
					}

					m_cpu.pc++;
					break;

				case instruction_set::MOV:
					try {
						m_cpu.reg.at(vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_argb)).to_ulong()) =
							m_cpu.reg.at(vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_arga)).to_ulong());
					}
					catch (const std::out_of_range& err) {
						std::cerr << " \n[error] " << err.what() << " in " << __FILE__ << ":" << __LINE__ << '\n';
						m_cpu.halt = true;
					}

					m_cpu.pc++;
					break;

				case instruction_set::ADD:
					try {
						auto a = m_cpu.reg.at(vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_arga)).to_ulong()).to_ulong();
						auto b = m_cpu.reg.at(vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_argb)).to_ulong()).to_ulong();

						m_cpu.reg.at(vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_arga)).to_ulong()) = vm_byte(a + b);
					}
					catch (const std::out_of_range& err) {
						std::cerr << " \n[error] " << err.what() << " in " << __FILE__ << ":" << __LINE__ << '\n';
						m_cpu.halt = true;
					}

					m_cpu.pc++;
					break;

				case instruction_set::SUB:
					try {
						auto a = m_cpu.reg.at(vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_arga)).to_ulong()).to_ulong();
						auto b = m_cpu.reg.at(vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_argb)).to_ulong()).to_ulong();

						m_cpu.reg.at(0) = vm_byte(a - b);
					}
					catch (const std::out_of_range& err) {
						std::cerr << " \n[error] " << err.what() << " in " << __FILE__ << ":" << __LINE__ << '\n';
						m_cpu.halt = true;
					}

					m_cpu.pc++;
					break;

				case instruction_set::SUBC:
					// What is SUBC even supposed to do?
					m_cpu.pc++;
					break;

				case instruction_set::INC:
					try {
						auto a = m_cpu.reg.at(vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_arga)).to_ulong()).to_ulong();
						m_cpu.reg.at(vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_arga)).to_ulong()) = vm_byte(++a);
					}
					catch (const std::out_of_range& err) {
						std::cerr << " \n[error] " << err.what() << " in " << __FILE__ << ":" << __LINE__ << '\n';
						m_cpu.halt = true;
					}

					m_cpu.pc++;
					break;

				case instruction_set::DEC:
					try {
						auto a = m_cpu.reg.at(vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_arga)).to_ulong()).to_ulong();
						m_cpu.reg.at(vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_arga)).to_ulong()) = vm_byte(--a);
					}
					catch (const std::out_of_range& err) {
						std::cerr << " \n[error] " << err.what() << " in " << __FILE__ << ":" << __LINE__ << '\n';
						m_cpu.halt = true;
					}

					m_cpu.pc++;
					break;

				case instruction_set::RSH:

					m_cpu.pc++;
					break;

				case instruction_set::LSH:


					m_cpu.pc++;
					break;

				case instruction_set::NOT:


					m_cpu.pc++;
					break;

				case instruction_set::NAND:


					m_cpu.pc++;
					break;

				case instruction_set::XOR:


					m_cpu.pc++;
					break;

				case instruction_set::OR:


					m_cpu.pc++;
					break;

				case instruction_set::AND:


					m_cpu.pc++;
					break;

				case instruction_set::DCO:
					try {
						auto arga = m_cpu.rom.at(m_cpu.pc).to_string().substr(_arga);
						auto argb = m_cpu.rom.at(m_cpu.pc).to_string().substr(_argb);
					
						static char data[4];
						character ch;
					
						data[3] = ch.CompareValue(arga.substr(0, 6));
						data[2] = ch.CompareValue(arga.substr(6));
						data[1] = ch.CompareValue(argb.substr(0, 6));
						data[0] = ch.CompareValue(argb.substr(6));
						
						bufferOutput(std::string(data));

					}
					catch (const std::out_of_range& err) {
						std::cerr << " \n[error] " << err.what() << " in " << __FILE__ << ":" << __LINE__ << '\n';
						m_cpu.halt = true;
					}

					m_cpu.pc++;
					break;

				case instruction_set::DCR:


					m_cpu.pc++;
					break;

				case instruction_set::DCM:


					m_cpu.pc++;
					break;

				case instruction_set::DCP:


					m_cpu.pc++;
					break;

				case instruction_set::LDP:


					m_cpu.pc++;
					break;

				case instruction_set::SP:


					m_cpu.pc++;
					break;

					// Added support to compare any registers with two
					// 6 bit pairs in ARGA (optional, affects JNE and JE)
				case instruction_set::JNE:
				{						
					try {
						auto arga = vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_arga));

						auto address = vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_argb));
						auto col = vm_byte(address.to_string().substr(2, 3));
						auto row = vm_byte(address.to_string().substr(6));

						if (arga.none())
							if (m_cpu.reg.at(0).to_ulong() == m_cpu.reg.at(0).to_ulong())
								m_cpu.pc = col.to_ulong() * 7U + row.to_ulong();
							else
								m_cpu.pc++;
						else
						{
							auto x = vm_byte(arga.to_string().substr(17, 6));
							auto y = vm_byte(arga.to_string().substr(23));

							if (m_cpu.reg.at(x.to_ulong()).to_ulong() != m_cpu.reg.at(y.to_ulong()).to_ulong())
								m_cpu.pc = col.to_ulong() * 7U + row.to_ulong();
							else
								m_cpu.pc++;
						}
					}
					catch (const std::out_of_range& err) {
						std::cerr << " \n[error] " << err.what() << " in " << __FILE__ << ":" << __LINE__ << '\n';
						m_cpu.halt = true;
					}
				}
				break;

				case instruction_set::JE:
				{
					try {
						auto arga = vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_arga));

						auto address = vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_argb));
						auto col = vm_byte(address.to_string().substr(2, 3));
						auto row = vm_byte(address.to_string().substr(6));

						if (arga.none())
							if (m_cpu.reg.at(0).to_ulong() == m_cpu.reg.at(0).to_ulong())
								m_cpu.pc = col.to_ulong() * 7U + row.to_ulong();
							else
								m_cpu.pc++;
						else
						{
							auto x = vm_byte(arga.to_string().substr(17, 6));
							auto y = vm_byte(arga.to_string().substr(23));

							if (m_cpu.reg.at(x.to_ulong()).to_ulong() == m_cpu.reg.at(y.to_ulong()).to_ulong())
								m_cpu.pc = col.to_ulong() * 7U + row.to_ulong();
							else
								m_cpu.pc++;
						}
					}
					catch (const std::out_of_range& err) {
						std::cerr << " \n[error] " << err.what() << " in " << __FILE__ << ":" << __LINE__ << '\n';
						m_cpu.halt = true;
					}
				}
				break;

				case instruction_set::JMP:
				{
					auto address = vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_argb));
					auto col = vm_byte(address.to_string().substr(2, 3));
					auto row = vm_byte(address.to_string().substr(6));

					m_cpu.pc = col.to_ulong() * 7U + row.to_ulong();
				}
				break;

				case instruction_set::ICP:


					m_cpu.pc++;
					break;

				case instruction_set::DIR:
					try {
						bufferOutput(std::to_string(m_cpu.reg.at(vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_arga)).to_ulong()).to_ulong()));
					}
					catch (const std::out_of_range& err) {
						std::cerr << " \n[error] " << err.what() << " in " << __FILE__ << ":" << __LINE__ << '\n';
						m_cpu.halt = true;
					}

					m_cpu.pc++;
					break;

				case instruction_set::DIM:
					try {
						auto result = m_cpu.ram.at(vm_byte(m_cpu.rom.at(m_cpu.pc).to_string().substr(_arga)).to_ulong()).to_ulong();
						std::cout << result;
					}
					catch (const std::out_of_range& err) {
						std::cerr << " \n[error] " << err.what() << " in " << __FILE__ << ":" << __LINE__ << '\n';
						m_cpu.halt = true;
					}

					m_cpu.pc++;
					break;

				case instruction_set::DIP:


					m_cpu.pc++;
					break;

				case instruction_set::IIP:


					m_cpu.pc++;
					break;

				case instruction_set::RST:
					m_cpu.halt = true;
					//std::cout << "\n[info] Virtual machine halted\n";
					break;

				default:
					break;
				}
			}
		}

		return 0;
	}
};
