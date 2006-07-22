#ifndef PLANET_H
#define PLANET_H
/*  Planet ObjectData class
 *
 *  Copyright (C) 2004  Lee Begg and the Thousand Parsec Project
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

#include <map>

#include <tpserver/ownedobject.h>

class Planet:public OwnedObject {
      public:
	Planet();

	void packExtraData(Frame * frame);

	void doOnceATurn(IGObject * obj);

	void packAllowedOrders(Frame * frame, int playerid);

	bool checkAllowedOrder(int ot, int playerid);

	int getContainerType();
	
	ObjectData* clone();

    std::map<uint32_t, std::pair<uint32_t, uint32_t> > getResources();
    
    void setResources(std::map<uint32_t, std::pair<uint32_t, uint32_t> > ress);
    void addResource(uint32_t restype, uint32_t amount);
    bool removeResource(uint32_t restype, uint32_t amount);

      private:
    std::map<uint32_t, std::pair<uint32_t, uint32_t> > resources;

};

#endif