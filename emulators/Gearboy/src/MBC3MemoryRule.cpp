/*
 * Gearboy - Nintendo Game Boy Emulator
 * Copyright (C) 2012  Ignacio Sanchez

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/ 
 * 
 */

#include "MBC3MemoryRule.h"
#include "Video.h"
#include "Memory.h"
#include "Processor.h"
#include "Input.h"
#include "Cartridge.h"

MBC3MemoryRule::MBC3MemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo, Input* pInput,
        Cartridge* pCartridge, Audio* pAudio) : MemoryRule(pProcessor,
pMemory, pVideo, pInput, pCartridge, pAudio)
{
    m_pRAMBanks = new u8[0x8000];
    Reset(false);
}

MBC3MemoryRule::~MBC3MemoryRule()
{
    SafeDeleteArray(m_pRAMBanks);
}

u8 MBC3MemoryRule::PerformRead(u16 address)
{
    switch (address & 0xE000)
    {
        case 0x4000:
        case 0x6000:
        {
            u8* pROM = m_pCartridge->GetTheROM();
            return pROM[(address - 0x4000) + m_CurrentROMAddress];
        }
        case 0xA000:
        {
            if (m_iCurrentRAMBank >= 0)
            {
                if (m_bRamEnabled)
                {
                    return m_pRAMBanks[(address - 0xA000) + m_CurrentRAMAddress];
                }
                else
                {
                    Log("--> ** Attempting to read from disabled ram %X", address);
                    return 0xFF;
                }
            }
            else if (m_pCartridge->IsRTCPresent() && m_bRTCEnabled)
            {
                switch (m_RTCRegister)
                {
                    case 0x08:
                        return m_iRTCLatchedSeconds;
                        break;
                    case 0x09:
                        return m_iRTCLatchedMinutes;
                        break;
                    case 0x0A:
                        return m_iRTCLatchedHours;
                        break;
                    case 0x0B:
                        return m_iRTCLatchedDays;
                        break;
                    case 0x0C:
                        return m_iRTCLatchedControl;
                        break;
                    default:
                        return 0xFF;
                }
            }
            else
            {
                Log("--> ** Attempting to read from disabled RTC %X", address);
                return 0xFF;
            }
        }
        default:
        {
            return m_pMemory->Retrieve(address);
        }
    }
}

void MBC3MemoryRule::PerformWrite(u16 address, u8 value)
{
    switch (address & 0xE000)
    {
        case 0x0000:
        {
            if (m_pCartridge->GetRAMSize() > 0)
                m_bRamEnabled = (value & 0x0F) == 0x0A;
            m_bRTCEnabled = (value & 0x0F) == 0x0A;
            break;
        }
        case 0x2000:
        {
            m_iCurrentROMBank = value & 0x7F;
            if (m_iCurrentROMBank == 0)
                m_iCurrentROMBank = 1;
            m_iCurrentROMBank &= (m_pCartridge->GetROMBankCount() - 1);
            m_CurrentROMAddress = m_iCurrentROMBank * 0x4000;
            break;
        }
        case 0x4000:
        {
            if ((value >= 0x08) && (value <= 0x0C))
            {
                // RTC
                if (m_pCartridge->IsRTCPresent() && m_bRTCEnabled)
                {
                    m_RTCRegister = value;
                    m_iCurrentRAMBank = -1;
                }
            }
            else if (value <= 0x03)
            {
                if (m_bRamEnabled)
                {
                    m_iCurrentRAMBank = value;
                    m_iCurrentRAMBank &= (m_pCartridge->GetRAMBankCount() - 1);
                    m_CurrentRAMAddress = m_iCurrentRAMBank * 0x2000;
                }
            }
            break;
        }
        case 0x6000:
        {
            if (m_pCartridge->IsRTCPresent())
            {
                // RTC Latch
                if ((m_iRTCLatch == 0x00) && (value == 0x01))
                {
                    UpdateRTC();
                    m_iRTCLatchedSeconds = m_iRTCSeconds;
                    m_iRTCLatchedMinutes = m_iRTCMinutes;
                    m_iRTCLatchedHours = m_iRTCHours;
                    m_iRTCLatchedDays = m_iRTCDays;
                    m_iRTCLatchedControl = m_iRTCControl;
                }
                if ((value == 0x00) || (value == 0x01))
                {
                    m_iRTCLatch = value;
                }
            }
            break;
        }
        case 0xA000:
        {
            if (m_iCurrentRAMBank >= 0)
            {
                if (m_bRamEnabled)
                {
                    m_pRAMBanks[(address - 0xA000) + m_CurrentRAMAddress] = value;
                }
                else
                {
                    Log("--> ** Attempting to write on RAM when ram is disabled %X %X", address, value);
                }
            }
            else if (m_pCartridge->IsRTCPresent() && m_bRTCEnabled)
            {
                m_RTCLastTime = m_pCartridge->GetCurrentRTC();
                switch (m_RTCRegister)
                {
                    case 0x08:
                        m_iRTCSeconds = value;
                        break;
                    case 0x09:
                        m_iRTCMinutes = value;
                        break;
                    case 0x0A:
                        m_iRTCHours = value;
                        break;
                    case 0x0B:
                        m_iRTCDays = value;
                        break;
                    case 0x0C:
                        if (m_iRTCControl & 0x80)
                            m_iRTCControl = 0x80 | value;
                        else
                            m_iRTCControl = value;
                        break;
                }
            }
            else
            {
                Log("--> ** Attempting to write on RTC when RTC is disabled %X %X", address, value);
            }
            break;
        }
        default:
        {
            m_pMemory->Load(address, value);
            break;
        }
    }
}

void MBC3MemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_iCurrentRAMBank = 0;
    m_iCurrentROMBank = 1;
    m_bRamEnabled = false;
    m_bRTCEnabled = false;
    for (int i = 0; i < 0x8000; i++)
        m_pRAMBanks[i] = 0xFF;
    m_iRTCSeconds = 0;
    m_iRTCMinutes = 0;
    m_iRTCHours = 0;
    m_iRTCDays = 0;
    m_iRTCControl = 0;
    m_iRTCLatchedSeconds = 0;
    m_iRTCLatchedMinutes = 0;
    m_iRTCLatchedHours = 0;
    m_iRTCLatchedDays = 0;
    m_iRTCLatchedControl = 0;
    m_iRTCLatch = 0;
    m_RTCRegister = 0;
    m_RTCLastTime = -1;
    m_RTCLastTimeCache = 0;
    m_CurrentROMAddress = 0x4000;
    m_CurrentRAMAddress = 0;
}

void MBC3MemoryRule::SaveRam(std::ofstream & file)
{
    Log("MBC3MemoryRule save RAM...");

    for (int i = 0; i < 0x8000; i++)
    {
        u8 ram_byte = 0;
        ram_byte = m_pRAMBanks[i];
        file.write(reinterpret_cast<const char*> (&ram_byte), 1);
    }

    file.write(reinterpret_cast<const char*> (&m_iRTCSeconds), sizeof (s32));
    file.write(reinterpret_cast<const char*> (&m_iRTCMinutes), sizeof (s32));
    file.write(reinterpret_cast<const char*> (&m_iRTCHours), sizeof (s32));
    file.write(reinterpret_cast<const char*> (&m_iRTCDays), sizeof (s32));
    file.write(reinterpret_cast<const char*> (&m_iRTCControl), sizeof (s32));
    file.write(reinterpret_cast<const char*> (&m_iRTCLatchedSeconds), sizeof (s32));
    file.write(reinterpret_cast<const char*> (&m_iRTCLatchedMinutes), sizeof (s32));
    file.write(reinterpret_cast<const char*> (&m_iRTCLatchedHours), sizeof (s32));
    file.write(reinterpret_cast<const char*> (&m_iRTCLatchedDays), sizeof (s32));
    file.write(reinterpret_cast<const char*> (&m_iRTCLatchedControl), sizeof (s32));
    file.write(reinterpret_cast<const char*> (&m_RTCLastTime), sizeof (time_t));
    file.write(reinterpret_cast<const char*> (&m_RTCLastTimeCache), sizeof (time_t));
    file.write(reinterpret_cast<const char*> (&m_iRTCLatch), sizeof (s32));
    file.write(reinterpret_cast<const char*> (&m_RTCRegister), sizeof (u8));

    Log("MBC3MemoryRule save RAM done");
}

void MBC3MemoryRule::LoadRam(std::ifstream & file)
{
    Log("MBC3MemoryRule load RAM...");

    for (int i = 0; i < 0x8000; i++)
    {
        u8 ram_byte = 0;
        file.read(reinterpret_cast<char*> (&ram_byte), 1);
        m_pRAMBanks[i] = ram_byte;
    }

    file.read(reinterpret_cast<char*> (&m_iRTCSeconds), sizeof (s32));
    file.read(reinterpret_cast<char*> (&m_iRTCMinutes), sizeof (s32));
    file.read(reinterpret_cast<char*> (&m_iRTCHours), sizeof (s32));
    file.read(reinterpret_cast<char*> (&m_iRTCDays), sizeof (s32));
    file.read(reinterpret_cast<char*> (&m_iRTCControl), sizeof (s32));
    file.read(reinterpret_cast<char*> (&m_iRTCLatchedSeconds), sizeof (s32));
    file.read(reinterpret_cast<char*> (&m_iRTCLatchedMinutes), sizeof (s32));
    file.read(reinterpret_cast<char*> (&m_iRTCLatchedHours), sizeof (s32));
    file.read(reinterpret_cast<char*> (&m_iRTCLatchedDays), sizeof (s32));
    file.read(reinterpret_cast<char*> (&m_iRTCLatchedControl), sizeof (s32));
    file.read(reinterpret_cast<char*> (&m_RTCLastTime), sizeof (time_t));
    file.read(reinterpret_cast<char*> (&m_RTCLastTimeCache), sizeof (time_t));
    file.read(reinterpret_cast<char*> (&m_iRTCLatch), sizeof (s32));
    file.read(reinterpret_cast<char*> (&m_RTCRegister), sizeof (u8));

    m_iRTCControl &= 0x01;
    m_iRTCLatchedControl &= 0x01;

    Log("MBC3MemoryRule load RAM done");
}

int MBC3MemoryRule::GetRamBanksSize()
{
    return 0x8000 + (sizeof (s32) * 11) + (sizeof (time_t) * 2) + 1;
}

void MBC3MemoryRule::UpdateRTC()
{
    time_t now = m_pCartridge->GetCurrentRTC();

    if (m_RTCLastTimeCache != now)
    {
        m_RTCLastTimeCache = now;
        time_t difference = now - m_RTCLastTime;
        if (difference > 0)
        {
            m_iRTCSeconds += (int) (difference % 60);
            if (m_iRTCSeconds > 59)
            {
                m_iRTCSeconds -= 60;
                m_iRTCMinutes++;
            }

            difference /= 60;

            m_iRTCMinutes += (int) (difference % 60);
            if (m_iRTCMinutes > 59)
            {
                m_iRTCMinutes -= 60;
                m_iRTCHours++;
            }

            difference /= 60;

            m_iRTCHours += (int) (difference % 24);
            if (m_iRTCHours > 23)
            {
                m_iRTCHours -= 24;
                m_iRTCDays++;
            }
            difference /= 24;

            m_iRTCDays += (int) (difference & 0xffffffff);
            if (m_iRTCDays > 0xFF)
            {
                if (m_iRTCDays > 511)
                {
                    m_iRTCDays %= 512;
                    m_iRTCControl |= 0x80;
                }
                m_iRTCControl = (m_iRTCControl & 0xFF) | (m_iRTCDays > 0xFF ? 1 : 0);
            }
        }
        m_RTCLastTime = now;
    }
}

