/*  PlayerManager
 *
 *  Copyright (C) 2005  Lee Begg and the Thousand Parsec Project
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

#include "player.h"
#include "playerview.h"
#include "game.h"
#include "persistence.h"
#include "ruleset.h"
#include "objectmanager.h"
#include "logging.h"
#include "boardmanager.h"
#include "board.h"
#include "message.h"
#include "designstore.h"
#include "design.h"

#include "playermanager.h"

PlayerManager::PlayerManager(){
    nextid = 1;
}

PlayerManager::~PlayerManager(){

}

void PlayerManager::init(){
    Persistence* persist = Game::getGame()->getPersistence();
    nextid = persist->getMaxPlayerId() + 1;
    clear(persist->getPlayerIds());
}

Player* PlayerManager::createNewPlayer(const std::string &name, const std::string &pass){
    Player *rtn = findPlayer(name);

    if(rtn != NULL){
        //Player's name exists already
        rtn = NULL;
    }else{
      //Player's name doesn't exist, create the new player.
      rtn = new Player();
      rtn->setId(nextid++);
      rtn->setName(name.c_str());
      rtn->setPass(pass.c_str());

      if(Game::getGame()->getRuleset()->onAddPlayer(rtn)){
        // player can be added
        
        //setup board and add to player
        Board::Ptr board = Game::getGame()->getBoardManager()->createNewBoard("Personal board", 
                "Messages from the System and personal notices board");
        rtn->setBoardId(board->getId());

        //add welcome message to player's board
        Message::Ptr msg( new Message() );
        msg->setSubject("Welcome");
        msg->setBody("Welcome to Thousand Parsec!\nThis server is running on tpserver-cpp.  Please report any problems and enjoy the game.");
        msg->addReference(rst_Special, rssv_System);
        msg->addReference(rst_Player, rtn->getID());
        board->addMessage(msg, -1);
        
        map[rtn->getID()] = (rtn);
        Game::getGame()->getPersistence()->savePlayer(rtn);
        
        Game::getGame()->getRuleset()->onPlayerAdded(rtn);
        
        //tell the other players about it
        msg.reset( new Message() );
        msg->setSubject("New Player");
        msg->setBody("New player " + name + " has joined the game.");
        msg->addReference(rst_Special, rssv_System);
        msg->addReference(rst_Action_Player, rspav_Joined);
        msg->addReference(rst_Player, rtn->getID());
        
        for(std::map<uint32_t, Player*>::const_iterator itid = map.begin();
                itid != map.end(); ++itid){
            Player* op = itid->second;
            if(op == NULL){
                op = Game::getGame()->getPersistence()->retrievePlayer(itid->first);
                map[itid->first] = op;
            }
            if(op != NULL && op != rtn){
                op->postToBoard(new Message(*msg));
                //to update the difflist, etc
                op->getPlayerView()->doOnceATurn();
                Game::getGame()->getPersistence()->updatePlayer(op);
            }
        }

        rtn->getPlayerView()->doOnceATurn();

      }else{
          // player can not be added
          delete rtn;
          rtn = NULL;
          nextid--;
      }
    
    }
    return rtn;
}

Player* PlayerManager::getPlayer(uint32_t id){
    Player* rtn = NULL;
    std::map<uint32_t, Player*>::iterator pl = map.find(id);
    if(pl != map.end()){
        rtn = (*pl).second;
    }else{
        //player does not exist
        return NULL;
    }
    if(rtn == NULL){
        rtn = Game::getGame()->getPersistence()->retrievePlayer(id);
        if(rtn != NULL){
          map[id] = rtn;
        }
    }
    return rtn;
}

Player* PlayerManager::findPlayer(const std::string &name){
    Logger::getLogger()->debug("finding player");

    //look for current/known players
    Player *rtn = NULL;

    std::map<uint32_t, Player*>::iterator itcurr;

    for (itcurr = map.begin(); itcurr != map.end(); ++itcurr) {
        Player* p = (*itcurr).second;
        if(p == NULL){
            p = Game::getGame()->getPersistence()->retrievePlayer(itcurr->first);
            itcurr->second = p;
        }
        if(p != NULL){
            std::string itname = p->getName();
            if (name == itname) {
                rtn = p;
                break;
            }
        }
    }

    return rtn;
}

void PlayerManager::updateAll(){
    for(std::map<uint32_t, Player*>::iterator itcurr = map.begin(); itcurr != map.end(); ++itcurr){
        Game::getGame()->getPersistence()->updatePlayer(itcurr->second);
    }
}

void PlayerManager::updatePlayer(uint32_t id){
    Game::getGame()->getPersistence()->updatePlayer(map[id]);
}

uint32_t PlayerManager::getNumPlayers() const{
  return size();
}
