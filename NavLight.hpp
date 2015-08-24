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

#ifndef __NAVLIGHT_HPP_INCLUDED__
#define __NAVLIGHT_HPP_INCLUDED__

#include <string>

#include "irrlicht.h"

class NavLight {

    public:
        NavLight(irr::scene::ISceneNode* parent, irr::scene::ISceneManager* smgr, irr::core::dimension2d<irr::f32> lightSize, irr::core::vector3df position, irr::video::SColor colour, irr::f32 lightStartAngle, irr::f32 lightEndAngle, irr::f32 lightRange, std::string lightSequence="");
        ~NavLight();
        void update(irr::f32 scenarioTime, irr::core::vector3df viewPosition, irr::u32 lightLevel);
        void moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ);

    private:
        irr::scene::IBillboardSceneNode* lightNode;
        irr::f32 startAngle;
        irr::f32 endAngle;
        irr::f32 range;
        std::string sequence;
        irr::f32 timeOffset;
        irr::u16 currentAlpha; //Note that this is u16 not u8 so we can indicate an initial implausible value.
        bool setAlpha(irr::u8 alpha, irr::video::ITexture* tex);
};

#endif
