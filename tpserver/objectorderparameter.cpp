/*  ObjectOrderParameter baseclass
 *
 *  Copyright (C) 2007 Lee Begg and the Thousand Parsec Project
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

#include <stdlib.h>

#include "frame.h"

#include "objectorderparameter.h"

ObjectOrderParameter::ObjectOrderParameter() : OrderParameter(), object(0){
  type = opT_Object_ID;
}

ObjectOrderParameter::~ObjectOrderParameter(){

}


void ObjectOrderParameter::packOrderFrame(Frame * f){
  f->packInt(object);
}

bool ObjectOrderParameter::unpackFrame(Frame *f, uint32_t playerid){
  if(f->isEnoughRemaining(4)){
    object = f->unpackInt();
    return true;
  }else{
    return false;
  }
}

OrderParameter *ObjectOrderParameter::clone() const{
  return new ObjectOrderParameter();
}

uint32_t ObjectOrderParameter::getObjectId() const{
  return object;
}

void ObjectOrderParameter::setObjectId(uint32_t id){
  object = id;
}

