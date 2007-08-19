/*  ownedobject
 *
 *  Copyright (C) 2007  Tyler Shaub and the Thousand Parsec Project
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

#include <tpserver/game.h>
#include <tpserver/object.h>
#include <tpserver/objectdata.h>
#include <tpserver/objectmanager.h>
#include <tpserver/player.h>
#include <tpserver/playerview.h>
#include <tpserver/playermanager.h>

#include "ownedobject.h"

namespace RFTS_ {

using std::set;

void exploreStarSys(IGObject* obj) {
   // set the planets in this star sys to visible for my owner
   OwnedObject *objData = dynamic_cast<OwnedObject*>(obj->getObjectData());

   Game *game = Game::getGame();
   ObjectManager *om = game->getObjectManager();

   IGObject *starSys = om->getObject(obj->getParent());
   PlayerView *pview = game->getPlayerManager()->getPlayer(objData->getOwner())->getPlayerView();

   pview->addVisibleObject(obj->getID());

   if(starSys->getID() == 0) return; // don't explore the universe

   set<uint32_t> planets = starSys->getContainedObjects();
   for(set<uint32_t>::const_iterator i = planets.begin(); i != planets.end(); ++i)
      pview->addVisibleObject(*i);
}

}