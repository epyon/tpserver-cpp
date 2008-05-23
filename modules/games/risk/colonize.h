#ifndef COLONIZE_H
#define COLONIZE_H
/*  Colonize class
 *
 *  Copyright (C) 2008  Ryan Neufeld and the Thousand Parsec Project
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
 
#include <tpserver/order.h>

class ObjectOrderParameter;    //Tyler's class had these, not sure the syntax/but I do get the purpose
class IGObject;
 
namespace RiskRuleset {

class Colonize : public::Order {
  
  public:
    
    Colonize();
    virtual ~Colonize();
    
    virtual Order* clone() const; //not sure what this does
    
    virtual bool doOrder(IGObject* obj);
  
  private:
    //I am assuming these are...parameters required for order 
    ObjectOrderParameter* units;    //# of units to colonize a target planet with.
    //ObjectOrderParameter* planet; //not sure about this one, or how colonize orders will be issued
};//class Colonize
} //namespace RiskRuleset
#endif