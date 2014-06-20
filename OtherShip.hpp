#ifndef __OTHERSHIP_HPP_INCLUDED__
#define __OTHERSHIP_HPP_INCLUDED__

#include "irrlicht.h"

#include "NavLight.hpp"
#include "Leg.hpp"

#include <cmath>
#include <vector>
#include <string>

class OtherShip
{
    public:
        OtherShip (const std::string& name,const irr::core::vector3df& location, std::vector<Leg> legsLoaded, irr::scene::ISceneManager* smgr);
        virtual ~OtherShip();
        void setHeading(irr::f32 hdg);
        void setSpeed(irr::f32 spd);
        irr::f32 getHeading() const;
        irr::f32 getSpeed() const;
        irr::core::vector3df getPosition() const;
        irr::f32 getLength() const;
        irr::f32 getHeight() const;
        irr::f32 getRCS() const;
        void update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::core::vector3df viewPosition);

    protected:
    private:

        void setPosition(irr::core::vector3df position);
        void setRotation(irr::core::vector3df rotation);

        irr::scene::IMeshSceneNode* otherShip; //The scene node for the other ship.
        std::vector<Leg> legs;
        std::vector<NavLight> navLights;
        irr::f32 heading;
        irr::f32 xPos;
        irr::f32 yPos;
        irr::f32 zPos;
        irr::f32 speed;
        irr::f32 length; //For radar calculation
        irr::f32 height; //For radar
        irr::f32 rcs;
};

#endif
