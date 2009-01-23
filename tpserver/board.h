#ifndef BOARD_H
#define BOARD_H

/*  Messages boards
 *
 *  Copyright (C) 2004-2005  Lee Begg and the Thousand Parsec Project
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

#include <string>

class Message;
class Frame;

class Board{

 public:
  Board();
  ~Board();

  void setBoardID(int i);
  int getBoardID();

  void setName(const std::string & nname);
  std::string getName();

  void setDescription(const std::string & ndest);
  std::string getDescription();
  
  void addMessage(Message* msg, int pos);
  bool removeMessage(uint32_t pos);


  void packBoard(Frame * frame);
  void packMessage(Frame * frame, uint32_t msgnum);

        int64_t getModTime() const;

    // for persistence only
    uint32_t getNumMessages() const;
    void setNumMessages(uint32_t nnm);
    void setModTime(uint64_t nmt);

 private:
    uint32_t nummsg;

  int boardid;
  std::string name;
  std::string description;

  int64_t modtime;

};

#endif
