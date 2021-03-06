#ifndef MOVE_H
#define MOVE_H
/*  The Move class defines a FleetOrder which is used by the leader
 *  fleets to occupy systems.  This may create an internal conflict.
 *
 *  Copyright (C) 2008  Dustin White and the Thousand Parsec Project
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "fleetorder.h"

class ObjectOrderParameter;

class Move : public FleetOrder
{
 public:
   Move();
   virtual ~Move();

   virtual Order* clone() const;

   virtual void createFrame(OutputFrame::Ptr f, int pos);
   virtual void inputFrame(InputFrame::Ptr f, uint32_t playerid);
   
   virtual bool doOrder(IGObject::Ptr obj);
   
   //Used to make sure the inputed system is bordering a science colony
   static bool isBorderingScienceColony(StarSystem* system);
};


#endif
