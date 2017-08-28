/*
 * Copyright (c) 2017, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Intel Corporation nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "SmbiosType4Utility.h"

namespace core
{
namespace system
{

core::system::SmbiosType4Utility &core::system::SmbiosType4Utility::getUtility()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	// Creating the singleton on class init as a static class member
	// can lead to static initialization order issues.
	// This is a thread-safe form of lazy initialization.
	static SmbiosType4Utility *pSingleton = new SmbiosType4Utility();
	return *pSingleton;
}

std::string SmbiosType4Utility::getProcessorType(unsigned char byteValue) const
{
	std::map<unsigned char, std::string> m;
	m[1] = "Other";
	m[2] = "Unknown";
	m[3] = "Central Processor";
	m[4] = "Math Processor";
	m[5] = "DSP Processor";
	m[6] = "Video Processor";
	return m[byteValue];
}

std::string SmbiosType4Utility::getProcessorFamily(unsigned int wordValue) const
{
	std::map<unsigned int, std::string> m;

	m[0x01] = "Other";
	m[0x02] = "Unknown";
	m[0x03] = "8086";
	m[0x04] = "80286";
	m[0x05] = "80386";
	m[0x06] = "80486";
	m[0x07] = "8087";
	m[0x08] = "80287";
	m[0x09] = "80387";
	m[0x0A] = "80487";
	m[0x0B] = "Pentium";
	m[0x0C] = "Pentium Pro";
	m[0x0D] = "Pentium II";
	m[0x0E] = "Pentium MMX";
	m[0x0F] = "Celeron";
	m[0x10] = "Pentium II Xeon";
	m[0x11] = "Pentium III";
	m[0x12] = "M1";
	m[0x13] = "M2";
	m[0x14] = "Celeron M";
	m[0x15] = "Pentium 4 HT";

	m[0x18] = "Duron";
	m[0x19] = "K5";
	m[0x1A] = "K6";
	m[0x1B] = "K6-2";
	m[0x1C] = "K6-3";
	m[0x1D] = "Athlon";
	m[0x1E] = "AMD29000";
	m[0x1F] = "K6-2+";
	m[0x20] = "Power PC";
	m[0x21] = "Power PC 601";
	m[0x22] = "Power PC 603";
	m[0x23] = "Power PC 603+";
	m[0x24] = "Power PC 604";
	m[0x25] = "Power PC 620";
	m[0x26] = "Power PC x704";
	m[0x27] = "Power PC 750";
	m[0x28] = "Core Duo";
	m[0x29] = "Core Duo Mobile";
	m[0x2A] = "Core Solo Mobile";
	m[0x2B] = "Atom";
	m[0x2C] = "Core M";
	m[0x2D] = "Core m3";
	m[0x2E] = "Core m5";
	m[0x2F] = "Core m7";
	m[0x30] = "Alpha";
	m[0x31] = "Alpha 21064";
	m[0x32] = "Alpha 21066";
	m[0x33] = "Alpha 21164";
	m[0x34] = "Alpha 21164PC";
	m[0x35] = "Alpha 21164a";
	m[0x36] = "Alpha 21264";
	m[0x37] = "Alpha 21364";
	m[0x38] = "Turion II Ultra Dual-Core Mobile M";
	m[0x39] = "Turion II Dual-Core Mobile M";
	m[0x3A] = "Athlon II Dual-Core M";
	m[0x3B] = "Opteron 6100";
	m[0x3C] = "Opteron 4100";
	m[0x3D] = "Opteron 6200";
	m[0x3E] = "Opteron 4200";
	m[0x3F] = "FX";
	m[0x40] = "MIPS";
	m[0x41] = "MIPS R4000";
	m[0x42] = "MIPS R4200";
	m[0x43] = "MIPS R4400";
	m[0x44] = "MIPS R4600";
	m[0x45] = "MIPS R10000";
	m[0x46] = "C-Series";
	m[0x47] = "E-Series";
	m[0x48] = "A-Series";
	m[0x49] = "G-Series";
	m[0x4A] = "Z-Series";
	m[0x4B] = "R-Series";
	m[0x4C] = "Opteron 4300";
	m[0x4D] = "Opteron 6300";
	m[0x4E] = "Opteron 3300";
	m[0x4F] = "FirePro";
	m[0x50] = "SPARC";
	m[0x51] = "SuperSPARC";
	m[0x52] = "MicroSPARC II";
	m[0x53] = "MicroSPARC IIep";
	m[0x54] = "UltraSPARC";
	m[0x55] = "UltraSPARC II";
	m[0x56] = "UltraSPARC IIi";
	m[0x57] = "UltraSPARC III";
	m[0x58] = "UltraSPARC IIIi";

	m[0x60] = "68040";
	m[0x61] = "68xxx";
	m[0x62] = "68000";
	m[0x63] = "68010";
	m[0x64] = "68020";
	m[0x65] = "68030";
	m[0x66] = "Athlon X4";
	m[0x67] = "Opteron X1000";
	m[0x68] = "Opteron X2000";
	m[0x69] = "Opteron A-Series";
	m[0x6A] = "Opteron X3000";
	m[0x6B] = "Zen";

	m[0x70] = "Hobbit";

	m[0x78] = "Crusoe TM5000";
	m[0x79] = "Crusoe TM3000";
	m[0x7A] = "Efficeon TM8000";

	m[0x80] = "Weitek";

	m[0x82] = "Itanium";
	m[0x83] = "Athlon 64";
	m[0x84] = "Opteron";
	m[0x85] = "Sempron";
	m[0x86] = "Turion 64";
	m[0x87] = "Dual-Core Opteron";
	m[0x88] = "Athlon 64 X2";
	m[0x89] = "Turion 64 X2";
	m[0x8A] = "Quad-Core Opteron";
	m[0x8B] = "Third-Generation Opteron";
	m[0x8C] = "Phenom FX";
	m[0x8D] = "Phenom X4";
	m[0x8E] = "Phenom X2";
	m[0x8F] = "Athlon X2";
	m[0x90] = "PA-RISC";
	m[0x91] = "PA-RISC 8500";
	m[0x92] = "PA-RISC 8000";
	m[0x93] = "PA-RISC 7300LC";
	m[0x94] = "PA-RISC 7200";
	m[0x95] = "PA-RISC 7100LC";
	m[0x96] = "PA-RISC 7100";

	m[0xA0] = "V30";
	m[0xA1] = "Quad-Core Xeon 3200";
	m[0xA2] = "Dual-Core Xeon 3000";
	m[0xA3] = "Quad-Core Xeon 5300";
	m[0xA4] = "Dual-Core Xeon 5100";
	m[0xA5] = "Dual-Core Xeon 5000";
	m[0xA6] = "Dual-Core Xeon LV";
	m[0xA7] = "Dual-Core Xeon ULV";
	m[0xA8] = "Dual-Core Xeon 7100";
	m[0xA9] = "Quad-Core Xeon 5400";
	m[0xAA] = "Quad-Core Xeon";
	m[0xAB] = "Dual-Core Xeon 5200";
	m[0xAC] = "Dual-Core Xeon 7200";
	m[0xAD] = "Quad-Core Xeon 7300";
	m[0xAE] = "Quad-Core Xeon 7400";
	m[0xAF] = "Multi-Core Xeon 7400";
	m[0xB0] = "Pentium III Xeon";
	m[0xB1] = "Pentium III Speedstep";
	m[0xB2] = "Pentium 4";
	m[0xB3] = "Xeon";
	m[0xB4] = "AS400";
	m[0xB5] = "Xeon MP";
	m[0xB6] = "Athlon XP";
	m[0xB7] = "Athlon MP";
	m[0xB8] = "Itanium 2";
	m[0xB9] = "Pentium M";
	m[0xBA] = "Celeron D";
	m[0xBB] = "Pentium D";
	m[0xBC] = "Pentium EE";
	m[0xBD] = "Core Solo";
	/* 0xBE handled as a special case */
	m[0xBF] = "Core 2 Duo";
	m[0xC0] = "Core 2 Solo";
	m[0xC1] = "Core 2 Extreme";
	m[0xC2] = "Core 2 Quad";
	m[0xC3] = "Core 2 Extreme Mobile";
	m[0xC4] = "Core 2 Duo Mobile";
	m[0xC5] = "Core 2 Solo Mobile";
	m[0xC6] = "Core i7";
	m[0xC7] = "Dual-Core Celeron";
	m[0xC8] = "IBM390";
	m[0xC9] = "G4";
	m[0xCA] = "G5";
	m[0xCB] = "ESA/390 G6";
	m[0xCC] = "z/Architecture";
	m[0xCD] = "Core i5";
	m[0xCE] = "Core i3";

	m[0xD2] = "C7-M";
	m[0xD3] = "C7-D";
	m[0xD4] = "C7";
	m[0xD5] = "Eden";
	m[0xD6] = "Multi-Core Xeon";
	m[0xD7] = "Dual-Core Xeon 3xxx";
	m[0xD8] = "Quad-Core Xeon 3xxx";
	m[0xD9] = "Nano";
	m[0xDA] = "Dual-Core Xeon 5xxx";
	m[0xDB] = "Quad-Core Xeon 5xxx";

	m[0xDD] = "Dual-Core Xeon 7xxx";
	m[0xDE] = "Quad-Core Xeon 7xxx";
	m[0xDF] = "Multi-Core Xeon 7xxx";
	m[0xE0] = "Multi-Core Xeon 3400";

	m[0xE4] = "Opteron 3000";
	m[0xE5] = "Sempron II";
	m[0xE6] = "Embedded Opteron Quad-Core";
	m[0xE7] = "Phenom Triple-Core";
	m[0xE8] = "Turion Ultra Dual-Core Mobile";
	m[0xE9] = "Turion Dual-Core Mobile";
	m[0xEA] = "Athlon Dual-Core";
	m[0xEB] = "Sempron SI";
	m[0xEC] = "Phenom II";
	m[0xED] = "Athlon II";
	m[0xEE] = "Six-Core Opteron";
	m[0xEF] = "Sempron M";

	m[0xFA] = "i860";
	m[0xFB] = "i960";

	m[0x100] = "ARMv7";
	m[0x101] = "ARMv8";
	m[0x104] = "SH-3";
	m[0x105] = "SH-4";
	m[0x118] = "ARM";
	m[0x119] = "StrongARM";
	m[0x12C] = "6x86";
	m[0x12D] = "MediaGX";
	m[0x12E] = "MII";
	m[0x140] = "WinChip";
	m[0x15E] = "DSP";
	m[0x1F4] = "Video Processor";

	m[1] = "Other";
	return m[wordValue];
}


}
}