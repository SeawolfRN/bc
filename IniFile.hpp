/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2014 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY Or FITNESS For A PARTICULAR PURPOSE.  See the
     GNU General Public License For more details.

     You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#ifndef __INIFILE_HPP_INCLUDED__
#define __INIFILE_HPP_INCLUDED__

#include "irrlicht.h"
#include <string>

namespace IniFile
{
    std::string enumerate1(std::string commandName, irr::u32 number);
    std::string enumerate2(std::string commandName, irr::u32 number1, irr::u32 number2);
    std::string iniFileToString(std::string fileName, std::string command);
    std::wstring iniFileToWString(std::string fileName, std::wstring command);
    irr::u32 iniFileTou32(std::string fileName, std::string command);
    irr::f32 iniFileTof32(std::string fileName, std::string command);
}

#endif
