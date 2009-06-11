/*  BoardManager
 *
 *  Copyright (C) 2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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

#include "board.h"
#include "game.h"
#include "persistence.h"
#include "message.h"
#include "logging.h"
#include "frame.h"

#include "boardmanager.h"

BoardManager::BoardManager() 
  : nextbid(1), nextmid(1) {
}


void BoardManager::init() {
  Persistence* persist = Game::getGame()->getPersistence();
  nextbid = persist->getMaxBoardId() + 1;
  nextmid = persist->getMaxMessageId() + 1;
  std::set<uint32_t> bidset(persist->getBoardIds());

  for(std::set<uint32_t>::iterator itcurr = bidset.begin(); itcurr != bidset.end(); ++itcurr) {
    boards[*itcurr] = NULL;
    boardmessages[*itcurr] = IdList();
  }
}

Board* BoardManager::createNewBoard(const std::string &name, const std::string &desc) {
  Board *rtn = new Board();

  rtn->setBoardID(nextbid++);
  rtn->setName(name);
  rtn->setDescription(desc);

  boards[rtn->getBoardID()] = (rtn);
  boardmessages[rtn->getBoardID()] = IdList();
  Game::getGame()->getPersistence()->saveBoard(rtn);

  return rtn;
}

Board* BoardManager::getBoard(uint32_t id) {
  Board* rtn = NULL;
  BoardMap::iterator pl = boards.find(id);
  if (pl != boards.end()) {
    rtn = (*pl).second;
  }
  if (rtn == NULL) {
    rtn = Game::getGame()->getPersistence()->retrieveBoard(id);
    boards[id] = rtn;
  }
  return rtn;
}

void BoardManager::updateBoard(uint32_t id) {
  Game::getGame()->getPersistence()->updateBoard(boards[id]);
}

bool BoardManager::addMessage(Message* msg, Board* board, uint32_t pos){
  uint32_t msgid = nextmid++;
  messagecache[msgid] = msg;
  IdList bmlist = boardmessages[board->getBoardID()];

  if (bmlist.empty()) {
    bmlist = Game::getGame()->getPersistence()->retrieveMessageList(board->getBoardID());
    boardmessages[board->getBoardID()] = bmlist;
  }

  if (pos == UINT32_NEG_ONE) {
    bmlist.push_back(msgid);
  } else {
    IdList::iterator inspos = bmlist.begin();
    advance(inspos, pos);
    bmlist.insert(inspos, msgid);
  }

  boardmessages[board->getBoardID()] = bmlist;

  bool result = true;
  result = result && Game::getGame()->getPersistence()->saveMessage(msgid, msg);
  result = result && Game::getGame()->getPersistence()->saveMessageList(board->getBoardID(), bmlist);
  board->setNumMessages(bmlist.size());
  return result;
}

bool BoardManager::removeMessage(Board* board, uint32_t pos){
  IdList bmlist = boardmessages[board->getBoardID()];
  if(bmlist.empty()){
    bmlist = Game::getGame()->getPersistence()->retrieveMessageList(board->getBoardID());
    boardmessages[board->getBoardID()] = bmlist;
  }
  if (pos >= bmlist.size()) {
    return false;
  }

  IdList::iterator itpos = bmlist.begin();
  advance(itpos, pos);
  uint32_t msgid = *itpos;
  Message* msg = messagecache[msgid];
  if(msg == NULL){
    msg = Game::getGame()->getPersistence()->retrieveMessage(msgid);
    messagecache[msgid] = msg;
  }
  delete msg;
  bmlist.erase(itpos);
  boardmessages[board->getBoardID()] = bmlist;

  bool result = true;
  result = result && Game::getGame()->getPersistence()->removeMessage(msgid);
  result = result && Game::getGame()->getPersistence()->saveMessageList(board->getBoardID(), bmlist);
  board->setNumMessages(bmlist.size());
  return result;
}

Message* BoardManager::getMessage(Board* board, uint32_t pos){
  IdList bmlist = boardmessages[board->getBoardID()];
  if(bmlist.empty()){
    bmlist = Game::getGame()->getPersistence()->retrieveMessageList(board->getBoardID());
    boardmessages[board->getBoardID()] = bmlist;
  }
  if (pos >= bmlist.size()) {
    return NULL;
  }

  IdList::iterator itpos = bmlist.begin();
  advance(itpos, pos);
  uint32_t msgerid = *itpos;
  Message* msg = messagecache[msgerid];
  if(msg == NULL){
    msg = Game::getGame()->getPersistence()->retrieveMessage(msgerid);
    messagecache[msgerid] = msg;
  }
  return msg;
}

