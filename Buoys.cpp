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

#include "Buoys.hpp"

#include "Buoy.hpp"
#include "NavLight.hpp"
#include "IniFile.hpp"
#include "Constants.hpp"
#include "RadarData.hpp"
#include "SimulationModel.hpp"

using namespace irr;

Buoys::Buoys()
{

}

Buoys::~Buoys()
{
    //dtor
}

void Buoys::load(const std::string& worldName, irr::scene::ISceneManager* smgr, SimulationModel* model)
{
    //get buoy.ini filename
    std::string scenarioBuoyFilename = worldName;
    scenarioBuoyFilename.append("/buoy.ini");

    //get light.ini filename
    std::string scenarioLightFilename = worldName;
    scenarioLightFilename.append("/light.ini");

    //Find number of buoys
    u32 numberOfBuoys;
    numberOfBuoys = IniFile::iniFileTou32(scenarioBuoyFilename,"Number");
    for(u32 currentBuoy=1;currentBuoy<=numberOfBuoys;currentBuoy++) {

        //Get buoy type and construct filename
        std::string buoyName = IniFile::iniFileToString(scenarioBuoyFilename,IniFile::enumerate1("Type",currentBuoy));
        //Get buoy position
        f32 buoyX = model->longToX(IniFile::iniFileTof32(scenarioBuoyFilename,IniFile::enumerate1("Long",currentBuoy)));
        f32 buoyZ = model->latToZ(IniFile::iniFileTof32(scenarioBuoyFilename,IniFile::enumerate1("Lat",currentBuoy)));

        //get buoy RCS if set
        f32 rcs = IniFile::iniFileTof32(scenarioBuoyFilename,IniFile::enumerate1("RCS",currentBuoy));

        //Create buoy and load into vector
        buoys.push_back(Buoy (buoyName.c_str(),core::vector3df(buoyX,0.0f,buoyZ),rcs,smgr));

        //Load buoy light information from light.ini file if available

        //Find number of lights
        u32 numberOfLights;
        numberOfLights = IniFile::iniFileTou32(scenarioLightFilename,"Number");
        //Run through lights, and check if any are associated with this buoy
        for (u32 currentLight=1;currentLight<=numberOfLights;currentLight++) {
            if (IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Buoy",currentLight)) ==currentBuoy ) {
                //Light on this buoy, add a light to the buoysLights vector in this location if required (FIXME: Think about response to waves?)
                f32 lightHeight = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("Height",currentLight));
                f32 lightR = IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Red",currentLight));
                f32 lightG = IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Green",currentLight));
                f32 lightB = IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Blue",currentLight));
                f32 lightRange = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("Range",currentLight));
                std::string lightSequence = IniFile::iniFileToString(scenarioLightFilename,IniFile::enumerate1("Sequence",currentLight));
                f32 lightStart = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("StartAngle",currentLight));
                f32 lightEnd = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("EndAngle",currentLight));
                lightRange = lightRange * M_IN_NM;


                buoysLights.push_back(NavLight (0,smgr,core::dimension2d<f32>(5, 5), core::vector3df(buoyX,lightHeight,buoyZ),video::SColor(255,lightR,lightG,lightB),lightStart,lightEnd,lightRange, lightSequence));
            }
        }
    }
}

void Buoys::update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::f32 tideHeight, irr::core::vector3df viewPosition, irr::u32 lightLevel)
{
    for(std::vector<Buoy>::iterator it = buoys.begin(); it != buoys.end(); ++it) {
        f32 xPos, yPos, zPos;
        core::vector3df pos = it->getPosition();
        xPos = pos.X;
        yPos = tideHeight;
        zPos = pos.Z;
        it->setPosition(core::vector3df(xPos,yPos,zPos));
    }

    for(std::vector<NavLight>::iterator it = buoysLights.begin(); it != buoysLights.end(); ++it) {
        it->update(scenarioTime, viewPosition, lightLevel);
    }
}

RadarData Buoys::getRadarData(irr::u32 number, irr::core::vector3df scannerPosition) const
//Get data for Buoy (number) relative to scannerPosition
{
    RadarData radarData;

    if (number<=buoys.size()) {
        radarData = buoys[number-1].getRadarData(scannerPosition);
    }

    return radarData;
}

irr::u32 Buoys::getNumber() const
{
    return buoys.size();
}

irr::core::vector3df Buoys::getPosition(int number) const
{
    if (number < buoys.size()) {
        return buoys.at(number).getPosition();
    } else {
        return irr::core::vector3df(0,0,0);
    }

}

void Buoys::moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ)
{
    for(std::vector<Buoy>::iterator it = buoys.begin(); it != buoys.end(); ++it) {
        it->moveNode(deltaX,deltaY,deltaZ);
    }

    for(std::vector<NavLight>::iterator it = buoysLights.begin(); it != buoysLights.end(); ++it) {
        it->moveNode(deltaX,deltaY,deltaZ);
    }
}
