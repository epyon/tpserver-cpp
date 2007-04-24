/*  MTSecTurn object
 *
 *  Copyright (C) 2007  Lee Begg and the Thousand Parsec Project
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
#include <tpserver/ordermanager.h>
#include <tpserver/objectmanager.h>
#include <tpserver/playermanager.h>
#include <tpserver/order.h>
#include <tpserver/object.h>
#include <tpserver/ownedobject.h>
#include <tpserver/player.h>
#include <tpserver/objectdatamanager.h>
#include <tpserver/objectdata.h>
#include <tpserver/objectparameter.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/orderqueue.h>

#include "avacombat.h"

#include "mtsecturn.h"

MTSecTurn::MTSecTurn() : TurnProcess(){
  
}

MTSecTurn::~MTSecTurn(){
  
}

void MTSecTurn::doTurn(){
  std::set<uint32_t>::iterator itcurr;
  
  Game* game = Game::getGame();
  OrderManager* ordermanager = game->getOrderManager();
  ObjectManager* objectmanager = game->getObjectManager();
  AVACombat* combatstrategy = new AVACombat();
  PlayerManager* playermanager = game->getPlayerManager();

  //do orders
  std::set<uint32_t> objects = objectmanager->getAllIds();
  for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
    IGObject * ob = objectmanager->getObject(*itcurr);
    if(ob->getType() == planettype || ob->getType() == fleettype){
      OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*>(ob->getObjectData()->getParameterByType(obpT_Order_Queue));
      if(oqop != NULL){
        OrderQueue* orderqueue = ordermanager->getOrderQueue(oqop->getQueueId());
        Order * currOrder = orderqueue->getFirstOrder();
        if(currOrder != NULL){
            if(currOrder->doOrder(ob)){
            orderqueue->removeFirstOrder();
            }else{
                orderqueue->updateFirstOrder();
            }
        }
      }
    }
    objectmanager->doneWithObject(ob->getID());
  }

  objectmanager->clearRemovedObjects();

  // update positions and velocities
  objects = objectmanager->getAllIds();
  for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
      IGObject * ob = objectmanager->getObject(*itcurr);
      ob->updatePosition();
      objectmanager->doneWithObject(ob->getID());
  }

  for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
      IGObject * ob = objectmanager->getObject(*itcurr);
      //ob->updatePosition();
      std::set<unsigned int> cont = ob->getContainedObjects();
      for(std::set<uint32_t>::iterator ita = cont.begin(); ita != cont.end(); ++ita){
          IGObject* itaobj = objectmanager->getObject(*ita);
          if(itaobj->getType() == fleettype || (itaobj->getType() == planettype && ((OwnedObject*)(itaobj->getObjectData()))->getOwner() != 0)){
              for(std::set<unsigned int>::iterator itb = ita; itb != cont.end(); ++itb){
                  IGObject* itbobj = objectmanager->getObject(*itb);
                  if((*ita != *itb) && (itbobj->getType() == fleettype || (itbobj->getType() == planettype && ((OwnedObject*)(itbobj->getObjectData()))->getOwner() != 0))){
                      if(((OwnedObject*)(itaobj->getObjectData()))->getOwner() != ((OwnedObject*)(itbobj->getObjectData()))->getOwner()){
                          uint64_t diff = itaobj->getPosition().getDistance(itbobj->getPosition());
                          if(diff <= itaobj->getSize() / 2 + itbobj->getSize() / 2){
                              combatstrategy->setCombatants(itaobj, itbobj);
                              combatstrategy->doCombat();
                              if(!combatstrategy->isAliveCombatant1()){
                                  if(itaobj->getType() == planettype){
                                      ((OwnedObject*)(itaobj->getObjectData()))->setOwner(0);
                                  }else{
                                      objectmanager->scheduleRemoveObject(*ita);
                                  }
                              }
                              if(!combatstrategy->isAliveCombatant2()){
                                  if(itbobj->getType() == planettype){
                                      ((OwnedObject*)(itbobj->getObjectData()))->setOwner(0);
                                  }else{
                                      objectmanager->scheduleRemoveObject(*itb);
                                  }
                              }
                          }
                      }
                  }
                  objectmanager->doneWithObject(itbobj->getID());
              }
              
              //combat between object and container (ie planet)
              if(ob->getType() == fleettype || (ob->getType() == planettype && ((OwnedObject*)(ob->getObjectData()))->getOwner() != 0)){
                  if(((OwnedObject*)(itaobj->getObjectData()))->getOwner() != ((OwnedObject*)(ob->getObjectData()))->getOwner()){
                      combatstrategy->setCombatants(itaobj, ob);
                      combatstrategy->doCombat();
                      if(!combatstrategy->isAliveCombatant1()){
                          if(itaobj->getType() == planettype){
                              ((OwnedObject*)(itaobj->getObjectData()))->setOwner(0);
                          }else{
                              objectmanager->scheduleRemoveObject(*ita);
                          }
                      }
                      if(!combatstrategy->isAliveCombatant2()){
                          if(ob->getType() == planettype){
                              ((OwnedObject*)(ob->getObjectData()))->setOwner(0);
                          }else{
                              objectmanager->scheduleRemoveObject(*itcurr);
                          }
                      }
                  }
              }
          }
          objectmanager->doneWithObject(itaobj->getID());
      }
      objectmanager->doneWithObject(ob->getID());
  }

  objectmanager->clearRemovedObjects();
  
  // to once a turn (right at the end)
  objects = objectmanager->getAllIds();
  for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
    IGObject * ob = objectmanager->getObject(*itcurr);
    ob->getObjectData()->doOnceATurn(ob);
    objectmanager->doneWithObject(ob->getID());
  }

  // find the objects that are visible to each player
  std::set<uint32_t> vis = objectmanager->getAllIds();
  std::set<uint32_t> players = playermanager->getAllIds();
  for(std::set<uint32_t>::iterator itplayer = players.begin(); itplayer != players.end(); ++itplayer){
      playermanager->getPlayer(*itplayer)->setVisibleObjects(vis);
  }
  playermanager->updateAll();
  
  delete combatstrategy;
}

void MTSecTurn::setPlanetType(uint32_t pt){
  planettype = pt;
}

void MTSecTurn::setFleetType(uint32_t ft){
  fleettype = ft;
}