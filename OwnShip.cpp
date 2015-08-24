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

//Extends from the general 'Ship' class
#include "OwnShip.hpp"

#include "Constants.hpp"
#include "SimulationModel.hpp"
#include "IniFile.hpp"
#include "Angles.hpp"

#include <cstdlib> //For rand()

using namespace irr;

void OwnShip::load(const std::string& scenarioName, irr::scene::ISceneManager* smgr, SimulationModel* model, Terrain* terrain)
{
    //Store reference to terrain
    this->terrain = terrain;

    //construct scenario ownship.ini filename
    std::string scenarioOwnShipFilename = scenarioName;
    scenarioOwnShipFilename.append("/ownship.ini");

    //Load from ownShip.ini file
    std::string ownShipName = IniFile::iniFileToString(scenarioOwnShipFilename,"ShipName");
    //Get initial position and heading, and set these
    spd = IniFile::iniFileTof32(scenarioOwnShipFilename,"InitialSpeed")*KTS_TO_MPS;
    xPos = model->longToX(IniFile::iniFileTof32(scenarioOwnShipFilename,"InitialLong"));
    yPos = 0;
    zPos = model->latToZ(IniFile::iniFileTof32(scenarioOwnShipFilename,"InitialLat"));
    hdg = IniFile::iniFileTof32(scenarioOwnShipFilename,"InitialBearing");

    //Load from boat.ini file if it exists
    std::string shipIniFilename = "Models/Ownship/";
    shipIniFilename.append(ownShipName);
    shipIniFilename.append("/boat.ini");

    //Construct the radar config file name, to be used later by the radar
    radarConfigFile = "Models/Ownship/";
    radarConfigFile.append(ownShipName);
    radarConfigFile.append("/radar.ini");

    //get the model file
    std::string ownShipFileName = IniFile::iniFileToString(shipIniFilename,"FileName");
    std::string ownShipFullPath = "Models/Ownship/";
                ownShipFullPath.append(ownShipName);
                ownShipFullPath.append("/");
                ownShipFullPath.append(ownShipFileName);

    //Load dynamics settings
    shipMass = IniFile::iniFileTof32(shipIniFilename,"Mass");
    inertia = IniFile::iniFileTof32(shipIniFilename,"Inertia");
    maxEngineRevs = IniFile::iniFileTof32(shipIniFilename,"MaxRevs");
    dynamicsSpeedA = IniFile::iniFileTof32(shipIniFilename,"DynamicsSpeedA");
    dynamicsSpeedB = IniFile::iniFileTof32(shipIniFilename,"DynamicsSpeedB");
    dynamicsTurnDragA = IniFile::iniFileTof32(shipIniFilename,"DynamicsTurnDragA");
    dynamicsTurnDragB = IniFile::iniFileTof32(shipIniFilename,"DynamicsTurnDragB");
    rudderA = IniFile::iniFileTof32(shipIniFilename,"RudderA");
    rudderB = IniFile::iniFileTof32(shipIniFilename,"RudderB");
    rudderBAstern = IniFile::iniFileTof32(shipIniFilename,"RudderBAstern");
    maxForce = IniFile::iniFileTof32(shipIniFilename,"Max_propulsion_force");
    propellorSpacing = IniFile::iniFileTof32(shipIniFilename,"PropSpace");
    asternEfficiency = IniFile::iniFileTof32(shipIniFilename,"AsternEfficiency");// (Optional, default 1)
    propWalkAhead = IniFile::iniFileTof32(shipIniFilename,"PropWalkAhead");// (Optional, default 0)
    propWalkAstern = IniFile::iniFileTof32(shipIniFilename,"PropWalkAstern");// (Optional, default 0)
    //Pitch and roll parameters: FIXME for hardcoding, and in future should be linked to the water's movements
    rollPeriod = 5; //Roll period (s)
    rollAngle = 2*IniFile::iniFileTof32(shipIniFilename,"Swell"); //Roll Angle (deg @weather=1)
    pitchPeriod = 6; //Roll period (s)
    pitchAngle = 0.5*IniFile::iniFileTof32(shipIniFilename,"Swell"); //Max pitch Angle (deg @weather=1)
    buffetPeriod = 8; //Yaw period (s)
    buffet = IniFile::iniFileTof32(shipIniFilename,"Buffet");

    //Set defaults for values that shouldn't be zero
    if (asternEfficiency == 0)
        {asternEfficiency = 1;}
    if (shipMass == 0)
        {shipMass = 10000;}
    if (inertia == 0)
        {inertia = 2000;}

    if (propellorSpacing==0) {
        singleEngine=true;
        maxForce *= 0.5; //Internally simulated with two equal engines, so halve the value
        std::cout << "Single engine" << std::endl;
    } else {
        singleEngine=false;
    }

    //Todo: Missing:
    //Number of engines
    //CentrifugalDriftEffect
    //PropWalkDriftEffect
    //Buffet
    //Swell
    //Windage
    //WindageTurnEffect
    //Also:
    //DeviationMaximum
    //DeviationMaximumHeading
    //?Depth
    //?AngleCorrection

    //Start in engine control mode
    controlMode = MODE_ENGINE;

    //calculate max speed from dynamics parameters
    maxSpeedAhead  = ((-1 * dynamicsSpeedB) + sqrt((dynamicsSpeedB*dynamicsSpeedB)-4*dynamicsSpeedA*-2*maxForce))/(2*dynamicsSpeedA);
	maxSpeedAstern = ((-1 * dynamicsSpeedB) + sqrt((dynamicsSpeedB*dynamicsSpeedB)-4*dynamicsSpeedA*-2*maxForce*asternEfficiency))/(2*dynamicsSpeedA);

    //Calculate engine speed required - the port and stbd engine speeds get send back to the GUI with updateGuiData.
    portEngine = requiredEngineProportion(spd);
    stbdEngine = requiredEngineProportion(spd);
    rudder=0;
    rateOfTurn=0;

    //Scale
    f32 scaleFactor = IniFile::iniFileTof32(shipIniFilename,"ScaleFactor");
    f32 yCorrection = IniFile::iniFileTof32(shipIniFilename,"YCorrection");

    //camera offset (in unscaled and uncorrected ship coords)
    irr::u32 numberOfViews = IniFile::iniFileTof32(shipIniFilename,"Views");
    if (numberOfViews==0) {
        //ToDo: Tell user that view positions couldn't be loaded
        exit(EXIT_FAILURE);
    }
    for(u32 i=1;i<=numberOfViews;i++) {
        f32 camOffsetX = IniFile::iniFileTof32(shipIniFilename,IniFile::enumerate1("ViewX",i));
        f32 camOffsetY = IniFile::iniFileTof32(shipIniFilename,IniFile::enumerate1("ViewY",i));
        f32 camOffsetZ = IniFile::iniFileTof32(shipIniFilename,IniFile::enumerate1("ViewZ",i));
        views.push_back(core::vector3df(scaleFactor*camOffsetX,scaleFactor*(camOffsetY+yCorrection),scaleFactor*camOffsetZ));
    }

    //Load the model
    scene::IMesh* shipMesh = smgr->getMesh(ownShipFullPath.c_str());

    //Translate and scale mesh here
    core::matrix4 transformMatrix;
    transformMatrix.setScale(core::vector3df(scaleFactor,scaleFactor,scaleFactor));
    transformMatrix.setTranslation(core::vector3df(0,yCorrection*scaleFactor,0));

    //Make mesh scene node
    if (shipMesh==0) {
        //Failed to load mesh - load with dummy and continue
        std::cout << "Failed to load own ship model " << ownShipFullPath << std::endl;
        ship = smgr->addCubeSceneNode(0.1);
    } else {
        smgr->getMeshManipulator()->transform(shipMesh,transformMatrix);
        ship = smgr->addMeshSceneNode(shipMesh,0,-1,core::vector3df(0,0,0));
    }

    ship->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true); //Normalise normals on scaled meshes, for correct lighting

    //Set lighting to use diffuse and ambient, so lighting of untextured models works
	if(ship->getMaterialCount()>0) {
        for(u32 mat=0;mat<ship->getMaterialCount();mat++) {
            ship->getMaterial(mat).ColorMaterial = video::ECM_DIFFUSE_AND_AMBIENT;
        }
    }

    length = ship->getBoundingBox().getExtent().Z; //Store length for basic collision calculation
    width = ship->getBoundingBox().getExtent().X; //Store length for basic collision calculation

    //set initial pitch and roll
    pitch = 0;
    roll = 0;
}

void OwnShip::setRudder(irr::f32 rudder)
{
    controlMode = MODE_ENGINE; //Switch to engine and rudder mode
    //Set the rudder (-ve is port, +ve is stbd)
    this->rudder = rudder;
}

void OwnShip::setPortEngine(irr::f32 port)
{
    controlMode = MODE_ENGINE; //Switch to engine and rudder mode
    portEngine = port; //+-1
}

void OwnShip::setStbdEngine(irr::f32 stbd)
{
    controlMode = MODE_ENGINE; //Switch to engine and rudder mode
    stbdEngine = stbd; //+-1
}

irr::f32 OwnShip::getPortEngine() const
{
    return portEngine;
}

irr::f32 OwnShip::getStbdEngine() const
{
    return stbdEngine;
}

irr::f32 OwnShip::getPortEngineRPM() const
{
    return portEngine*maxEngineRevs;
}

irr::f32 OwnShip::getStbdEngineRPM() const
{
    return stbdEngine*maxEngineRevs;
}

irr::f32 OwnShip::getRudder() const
{
    return rudder;
}

irr::f32 OwnShip::getPitch() const
{
    return pitch;
}

irr::f32 OwnShip::getRoll() const
{
    return roll;
}

bool OwnShip::isSingleEngine() const
{
    return singleEngine;
}

irr::f32 OwnShip::requiredEngineProportion(irr::f32 speed)
{
    irr::f32 proportion = 0;
    if (speed >= 0) {
        proportion=(dynamicsSpeedA*speed*speed + dynamicsSpeedB*speed)/(2*maxForce);
    } else {
        proportion=(-1*dynamicsSpeedA*speed*speed + dynamicsSpeedB*speed)/(2*maxForce*asternEfficiency);
    }
    return proportion;
}

void OwnShip::update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::f32 tideHeight, irr::f32 weather)
{
    //dynamics: hdg in degrees, spd in m/s. Internal units all SI
    if (controlMode == MODE_ENGINE) {
        //Update spd and hdg with rudder and engine controls - assume two engines: Fixme: Should also work with single engine
        portThrust = portEngine * maxForce;
        stbdThrust = stbdEngine * maxForce;
        if (singleEngine) {
            stbdThrust = portThrust; //Ignore stbd slider if single engine (internally modelled as 2 engines, each with half the max force)
        }
        if (portThrust<0) {portThrust*=asternEfficiency;}
        if (stbdThrust<0) {stbdThrust*=asternEfficiency;}
        if (spd<0) { //Compensate for loss of sign when squaring
            drag = -1*dynamicsSpeedA*spd*spd + dynamicsSpeedB*spd;
		} else {
			drag =    dynamicsSpeedA*spd*spd + dynamicsSpeedB*spd;
		}
		acceleration = (portThrust+stbdThrust-drag)/shipMass;
        spd += acceleration*deltaTime;

        //Turn dynamics
        //Rudder
        rudderTorque = rudder*spd*rudderA + rudder*(portThrust+stbdThrust)*rudderB; //Fixme: reduced rudder effect astern?
        //Engine
        engineTorque = (portThrust*propellorSpacing - stbdThrust*propellorSpacing)/2.0; //propspace is spacing between propellors, so halve to get moment arm
        //Prop walk
        irr::f32 propWalkTorquePort,propWalkTorqueStbd;
        if (portThrust > 0) {
            propWalkTorquePort=1*propWalkAhead*(portThrust/maxForce);//Had modification for 'invertspeed'
        } else {
			propWalkTorquePort=1*propWalkAstern*(portThrust/maxForce);
        }
		if (stbdThrust > 0) {
            propWalkTorqueStbd=-1*propWalkAhead*(stbdThrust/maxForce);//Had modification for 'invertspeed'
        } else {
			propWalkTorqueStbd=-1*propWalkAstern*(stbdThrust/maxForce);
        }
		propWalkTorque = propWalkTorquePort + propWalkTorqueStbd;
        //Turn drag
        if (rateOfTurn<0) {
            dragTorque=-1*dynamicsTurnDragA*rateOfTurn*rateOfTurn + dynamicsTurnDragB*rateOfTurn;
        } else {
            dragTorque=   dynamicsTurnDragA*rateOfTurn*rateOfTurn + dynamicsTurnDragB*rateOfTurn;
        }
        //Turn dynamics
        rateOfTurn += (rudderTorque + engineTorque + propWalkTorque - dragTorque)*deltaTime/inertia; //Rad/s

        //slow down if aground
        if (getDepth()<0) { //Todo: Have a separate groundingDepth(), that checks min depth at centre, and 3/4 ahead and astern of centre
            if (spd>0) {
                spd = fmin(0.1,spd); //currently hardcoded for 0.1 m/s, ~0.2kts
            }
            if (spd<0) {
                spd = fmax(-0.1,spd);
            }

            if (rateOfTurn>0) {
                rateOfTurn = fmin(0.01,rateOfTurn);//Rate of turn in rad/s, currently hardcoded for 0.01 rad/s
            }
            if (rateOfTurn<0) {
                rateOfTurn = fmax(-0.01,rateOfTurn);//Rate of turn in rad/s
            }
        }

        //Apply turn
        hdg += rateOfTurn*deltaTime*core::RADTODEG; //Deg

        //Apply buffeting from waves
        irr::f32 buffetAngle= buffet*weather*sin(scenarioTime*2*PI/buffetPeriod)*deltaTime;//Deg
        buffetAngle = buffetAngle * (irr::f32)std::rand()/RAND_MAX;

        hdg += buffetAngle;

    } //End of engine mode

    //Normalise heading
    if(hdg>=360) {hdg-=360;}
    if(hdg<0) {hdg+=360;}

    //move, according to heading and speed
    if (!positionManuallyUpdated && !this->sunk) { //If the position has already been updated, skip (for this loop only)
        xPos = xPos + sin(hdg*core::DEGTORAD)*spd*deltaTime;
        zPos = zPos + cos(hdg*core::DEGTORAD)*spd*deltaTime;
    } else if(!this->sunk) {
        positionManuallyUpdated = false;
    }


    //calculate pitch and roll - not linked to water/wave motion
    if (pitchPeriod>0)
        {pitch = weather*pitchAngle*sin(scenarioTime*2*PI/pitchPeriod);}
    if (rollPeriod>0)
        {roll = weather*rollAngle*sin(scenarioTime*2*PI/rollPeriod);}


    if(this->sunk){
        std::cout << "Sunk!!" << "\n";
        xPos = xPos;
        yPos = yPos;
        yPos = yPos - 10;
        pitch = 10;
    }
    else{
        yPos = tideHeight;
    }
    //Set position & angles
    ship->setPosition(core::vector3df(xPos,yPos,zPos));
    ship->setRotation(Angles::irrAnglesFromYawPitchRoll(hdg,pitch,roll));

}

irr::f32 OwnShip::getDepth()
{
    return -1*terrain->getHeight(xPos,zPos)+getPosition().Y;
}

std::vector<irr::core::vector3df> OwnShip::getCameraViews() const
{
    return views;
}

std::string OwnShip::getRadarConfigFile() const
{
    return radarConfigFile;
}