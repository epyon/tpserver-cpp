#ifndef RISK_H
#define RISK_H
/*  Risk rulesset class
 *
 *  Copyright (C) 2008  Ryan Neufeld and the Thousand Parsec Project
 *  Copyright (C) 2005, 2007  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/ruleset.h> 
#include <graph.h>
#include <vector>
#include <tpserver/vector3d.h>
#include <tpserver/player.h>

class IGObject;
class Random;

namespace RiskRuleset {
	
class Risk : public Ruleset {
   public:
      Risk();
      virtual ~Risk();

      std::string getName(); 
      std::string getVersion();
      void initGame(); 
      void createGame(); 
      void startGame(); 
      bool onAddPlayer(Player::Ptr player); 
      void onPlayerAdded(Player::Ptr player);
      
      Graph* getGraph();
      uint32_t getPlayerReinforcements(uint32_t owner);
      void setPlayerReinforcements(uint32_t owner, uint32_t units);
      
      //onAddPlayer methods
      bool isBoardClaimed() const;
      void increaseNumPlanets();      
      
   private:
       //initGame methods
      void setObjectTypes() const;
      void setOrderTypes() const;

       //createGame methods
      void createResources();

      void createUniverse();
      void createTestSystems(IGObject& universe);
      
      void setDefaults();
      
      void randomlyAssignPlanets(Player::Ptr player);
      void randomlyGiveOnePlanet(Player::Ptr player);
      void randomlyPickPlanets(Player::Ptr player, uint32_t numPlanets);
      
      //The number of planets to be on the board
      uint32_t num_planets;
      std::map<uint32_t, uint32_t> reinforcements;

      Graph graph;
      
      uint32_t rfc_number;
      
      Random* random;

};// class Risk : public Ruleset
	
} // namespace RiskRuleset
#endif
