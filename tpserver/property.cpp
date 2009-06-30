/*  Property class
 *
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

#include <time.h>

#include "frame.h"

#include "property.h"

Property::Property() : propid(0), rank(0) {
}

void Property::packFrame(Frame* frame) const {
  frame->setType(ft03_Property);
  frame->packInt(propid);
  frame->packInt64(getModTime());
  frame->packInt(catids.size());
  for(std::set<uint32_t>::const_iterator idit = catids.begin(); idit != catids.end(); ++idit){
    frame->packInt(*idit);
  }
  frame->packInt(rank);
  frame->packString(name.c_str());
  frame->packString(display.c_str());
  frame->packString(description.c_str());
  frame->packString(tpcl_display.c_str());
  frame->packString(tpcl_requires.c_str());
}

uint32_t Property::getPropertyId() const {
  return propid;
}

std::set<uint32_t> Property::getCategoryIds() const {
  return catids;
}

bool Property::isInCategory(uint32_t id) const {
  return catids.count(id) != 0;
}

uint32_t Property::getRank() const {
  return rank;
}

std::string Property::getName() const {
  return name;
}

std::string Property::getDisplayName() const {
  return display;
}

std::string Property::getDescription() const {
  return description;
}

std::string Property::getTpclDisplayFunction() const {
  return tpcl_display;
}

std::string Property::getTpclRequirementsFunction() const {
  return tpcl_requires;
}

void Property::setPropertyId(uint32_t id) {
  propid = id;
}

void Property::addCategoryId(uint32_t id) {
  catids.insert(id);
}

void Property::setCategoryIds(const std::set<uint32_t>& ids) {
  catids = ids;
}

void Property::setRank(uint32_t r) {
  rank = r;
}

void Property::setName(const std::string& n) {
  name = n;
}

void Property::setDisplayName(const std::string& d) {
  display = d;
}

void Property::setDescription(const std::string& d) {
  description = d;
}

void Property::setTpclDisplayFunction(const std::string& d) {
  tpcl_display = d;
}

void Property::setTpclRequirementsFunction(const std::string& r) {
  tpcl_requires = r;
}

