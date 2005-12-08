/*  Mysql persistence class
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

#include <my_global.h>
#include <my_sys.h>
#include <mysql.h>

#include <sstream>

#include "logging.h"
#include "settings.h"
#include "game.h"
#include "ordermanager.h"

#include "object.h"
#include "order.h"
#include "board.h"
#include "message.h"
#include "player.h"
#include "category.h"
#include "design.h"
#include "component.h"
#include "property.h"
#include "mysqlobjecttype.h"
#include "mysqlordertype.h"

#include "mysqlpersistence.h"

MysqlPersistence::MysqlPersistence() : conn(NULL){
}

MysqlPersistence::~MysqlPersistence(){
    if(conn != NULL)
        shutdown();
}

bool MysqlPersistence::init(){
    lock();
    if(conn != NULL){
        Logger::getLogger()->warning("Mysql Persistence already running");
        unlock();
        return false;
    }
    conn = mysql_init(NULL);
    if(conn == NULL){
        Logger::getLogger()->error("Could not init mysql");
        unlock();
        return false;
    }
    Settings* conf = Settings::getSettings();
    
    const char* host = NULL;
    std::string shost = conf->get("mysql_host");
    if(shost.length() != 0)
        host = shost.c_str();
    const char* user = NULL;
    std::string suser = conf->get("mysql_user");
    if(suser.length() != 0)
        user = suser.c_str();
    const char* pass = NULL;
    std::string spass = conf->get("mysql_pass");
    if(spass.length() != 0)
        pass = spass.c_str();
    const char* db = NULL;
    std::string sdb = conf->get("mysql_db");
    if(sdb.length() != 0)
        db = sdb.c_str();
    else{
        Logger::getLogger()->error("mysql database name not specified");
        mysql_close(conn);
        conn = NULL;
        unlock();
        return false;
    }
    const char* sock = NULL;
    std::string ssock = conf->get("mysql_socket");
    if(ssock.length() != 0)
        sock = ssock.c_str();
    int port = atoi(conf->get("mysql_port").c_str());
    if(mysql_real_connect(conn, host, user, pass, db, port, sock, CLIENT_FOUND_ROWS) == NULL){
        Logger::getLogger()->error("Could not connect to mysql server - %s", mysql_error(conn));
        mysql_close(conn);
        conn = NULL;
        unlock();
        return false;
    }
    
    // check for tables, create if necessary
    
    if(mysql_query(conn, "SELECT * FROM tableversion;") != 0){
        // create tables
        Logger::getLogger()->info("Creating database tables");
        try{
            if(mysql_query(conn, "CREATE TABLE tableversion ( tableid INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, "
               "name VARCHAR(50) NOT NULL UNIQUE, version INT UNSIGNED NOT NULL);") != 0){
                throw std::exception();
            }
            if(mysql_query(conn, "INSERT INTO tableversion VALUES (NULL, 'tableversion', 0), (NULL, 'object', 0), "
                    "(NULL, 'ordertype', 0), (NULL, 'orderslot', 0), (NULL, 'board', 0), (NULL, 'message', 0), (NULL, 'messageslot', 0), (NULL, 'player', 0), "
                    "(NULL, 'playerdesignvisible', 0), (NULL, 'playerdesignusable', 0), (NULL, 'playercomponentvisible', 0), "
                    "(NULL, 'playercomponentusable', 0), (NULL, 'playerobjectvisible', 0), "
                    "(NULL, 'category', 0), (NULL, 'design',0), (NULL, 'component', 0), (NULL, 'property', 0);") != 0){
                throw std::exception();
            }
            if(mysql_query(conn, "CREATE TABLE object (objectid INT UNSIGNED NOT NULL PRIMARY KEY, type INT UNSIGNED NOT NULL, " 
                    "name TEXT NOT NULL, parentid INT UNSIGNED NOT NULL, size BIGINT UNSIGNED NOT NULL, posx BIGINT NOT NULL, "
                    "posy BIGINT NOT NULL, posz BIGINT NOT NULL, velx BIGINT NOT NULL, vely BIGINT NOT NULL, velz BIGINT NOT NULL, "
                    "orders INT UNSIGNED NOT NULL, modtime BIGINT UNSIGNED NOT NULL);") != 0){
                throw std::exception();
            }
            if(mysql_query(conn, "CREATE TABLE ordertype (orderid INT UNSIGNED NOT NULL PRIMARY KEY, type INT UNSIGNED NOT NULL);") != 0){
                throw std::exception();
            }
            if(mysql_query(conn, "CREATE TABLE orderslot (objectid INT UNSIGNED NOT NULL, slot INT UNSIGNED NOT NULL, "
                        "orderid INT UNSIGNED NOT NULL UNIQUE, PRIMARY KEY (objectid, slot));") != 0){
                throw std::exception();
            }

            if(mysql_query(conn, "CREATE TABLE player (playerid INT UNSIGNED NOT NULL PRIMARY KEY, name TEXT NOT NULL, "
                    "password TEXT NOT NULL, boardid INT UNSIGNED NOT NULL, UNIQUE (name(255)));") != 0){
                throw std::exception();
            }
            if(mysql_query(conn, "CREATE TABLE playerdesignvisible (playerid INT UNSIGNED NOT NULL, designid INT UNSIGNED NOT NULL, "
                    "PRIMARY KEY (playerid, designid));") != 0){
                throw std::exception();
            }
            if(mysql_query(conn, "CREATE TABLE playerdesignusable (playerid INT UNSIGNED NOT NULL, designid INT UNSIGNED NOT NULL, "
               "PRIMARY KEY (playerid, designid));") != 0){
                   throw std::exception();
            }
            if(mysql_query(conn, "CREATE TABLE playercomponentvisible (playerid INT UNSIGNED NOT NULL, componentid INT UNSIGNED NOT NULL, "
                  "PRIMARY KEY (playerid, componentid));") != 0){
                throw std::exception();
            }
            if(mysql_query(conn, "CREATE TABLE playercomponentusable (playerid INT UNSIGNED NOT NULL, componentid INT UNSIGNED NOT NULL, "
                    "PRIMARY KEY (playerid, componentid));") != 0){
                throw std::exception();
            }
            if(mysql_query(conn, "CREATE TABLE playerobjectvisible (playerid INT UNSIGNED NOT NULL, objectid INT UNSIGNED NOT NULL, "
                    "PRIMARY KEY (playerid, objectid));") != 0){
                throw std::exception();
            }
               
        }catch(std::exception e){
            Logger::getLogger()->error("Mysql creating tables: %s", mysql_error(conn));
            Logger::getLogger()->error("You may need to delete the tables and start again");
            mysql_close(conn);
            conn = NULL;
            unlock();
            return false;
        }
    }else{
        // check for tables to be updated.
        MYSQL_RES *tableversions = mysql_store_result(conn);
        if(tableversions == NULL){
            Logger::getLogger()->error("Mysql: table versions query result error: %s", mysql_error(conn));
            Logger::getLogger()->error("You may need to delete the tables and start again");
            mysql_close(conn);
            conn = NULL;
            unlock();
            return false;
        }
        // Since this is the first release, there are no tables to update.
        mysql_free_result(tableversions);
    }
    
    unlock();
    
    if(mysql_thread_safe()){
        Logger::getLogger()->debug("Mysql is thread safe");
    }else{
        Logger::getLogger()->debug("Mysql is NOT thread safe");
    }
    
    return true;
}

void MysqlPersistence::shutdown(){
    lock();
    // TEMP HACK
    mysql_query(conn, "DELETE FROM object;");
    mysql_query(conn, "DELETE FROM universe;");
    mysql_query(conn, "DELETE FROM planet;");
    mysql_query(conn, "DELETE FROM fleet;");
    mysql_query(conn, "DELETE FROM fleetship;");
    mysql_query(conn, "DELETE FROM ordertype;");
    mysql_query(conn, "DELETE FROM orderslot;");
    mysql_query(conn, "DELETE FROM player;");
    mysql_query(conn, "DELETE FROM playerdesignvisible;");
    mysql_query(conn, "DELETE FROM playerdesignusable;");
    mysql_query(conn, "DELETE FROM playercomponentvisible;");
    mysql_query(conn, "DELETE FROM playercomponentusable;");
    mysql_query(conn, "DELETE FROM playerobjectvisible;");
    // end TEMP HACK
    if(conn != NULL){
        mysql_close(conn);
        conn = NULL;
    }
    unlock();
}

bool MysqlPersistence::saveObject(IGObject* ob){
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO object VALUES (" << ob->getID() << ", " << ob->getType() << ", ";
    querybuilder << "'" << addslashes(ob->getName()) << "', " << ob->getParent() << ", ";
    querybuilder << ob->getSize() << ", " << ob->getPosition().getX() << ", " << ob->getPosition().getY() << ", ";
    querybuilder << ob->getPosition().getZ() << ", " << ob->getVelocity().getX() << ", ";
    querybuilder << ob->getVelocity().getY() << ", " << ob->getVelocity().getZ() << ", " << ob->getNumOrders(-1);
    querybuilder << ", " << ob->getModTime() << ");";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store object %d - %s", ob->getID(), mysql_error(conn));
        unlock();
        return false;
    }
    bool rtv;
    //store type-specific information
    MysqlObjectType* obtype = objecttypes[ob->getType()];
    if(obtype != NULL){
        rtv = obtype->save(this, conn, ob);
    }else{
        Logger::getLogger()->error("Mysql: Object type %d not registered", ob->getType());
        rtv = false;
    }
    unlock();
    return rtv;
}

//#include <iostream>

bool MysqlPersistence::updateObject(IGObject* ob){
    std::ostringstream querybuilder;
    querybuilder << "UPDATE object SET type=" << ob->getType() << ", name='";
    querybuilder << addslashes(ob->getName()) << "', parentid=" << ob->getParent() << ", size=";
    querybuilder << ob->getSize() << ", posx=" << ob->getPosition().getX() << ", posy=" << ob->getPosition().getY() << ", posz=";
    querybuilder << ob->getPosition().getZ() << ", velx=" << ob->getVelocity().getX() << ", vely=";
    querybuilder << ob->getVelocity().getY() << ", velz=" << ob->getVelocity().getZ() << ", orders=" << ob->getNumOrders(-1);
    querybuilder << ", modtime=" << ob->getModTime() << " WHERE objectid=" << ob->getID() << ";";
    lock();
    std::string query = querybuilder.str();
    //std::cout << "Query: " << query << std::endl;
    if(mysql_query(conn, query.c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not update object %d - %s", ob->getID(), mysql_error(conn));
        unlock();
        return false;
    }
    bool rtv;
    //store type-specific information
    MysqlObjectType* obtype = objecttypes[ob->getType()];
    if(obtype != NULL){
        rtv = obtype->update(this, conn, ob);
    }else{
        Logger::getLogger()->error("Mysql: Object type %d not registered", ob->getType());
        rtv = false;
    }
    unlock();
    return rtv;
}

IGObject* MysqlPersistence::retrieveObject(uint32_t obid){
    std::ostringstream querybuilder;
    querybuilder << "SELECT * FROM object WHERE objectid = " << obid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve object %d - %s", obid, mysql_error(conn));
        unlock();
        return NULL;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve object: Could not store result - %s", mysql_error(conn));
        unlock();
        return NULL;
    }
    
    //children object ids
    querybuilder.str("");
    querybuilder << "SELECT objectid FROM object WHERE parentid = " << obid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve object %d - %s", obid, mysql_error(conn));
        unlock();
        mysql_free_result(obresult);
        return NULL;
    }
    MYSQL_RES *childres = mysql_store_result(conn);

    if(childres == NULL){
        Logger::getLogger()->error("Mysql: retrieve object: Could not store result - %s", mysql_error(conn));
        unlock();
        mysql_free_result(obresult);
        return NULL;
    }
    
    unlock(); // finished with mysql for a bit
    
    MYSQL_ROW row = mysql_fetch_row(obresult);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such object %d", obid);
        mysql_free_result(obresult);
        mysql_free_result(childres);
        return NULL;
    }
    IGObject* object = new IGObject();
    object->setID(obid);
    object->setType(atoi(row[1]));
    object->setName(row[2]);
    object->setParent(atoi(row[3]));
    object->setSize(strtoull(row[4], NULL, 10));
    Vector3d vec;
    vec.setAll(atoll(row[5]), atoll(row[6]), atoll(row[7]));
    object->setPosition(vec);
    vec.setAll(atoll(row[8]), atoll(row[9]), atoll(row[10]));
    object->setVelocity(vec);
    object->setNumOrders(atoi(row[11]));
    
    MYSQL_ROW children;
    while((children = mysql_fetch_row(childres)) != NULL){
        uint32_t childid = atoi(children[0]);
        Logger::getLogger()->debug("childid: %d", childid);
        if(childid != object->getID())
            object->addContainedObject(childid);
    }
    Logger::getLogger()->debug("num children: %d", object->getContainedObjects().size());
    mysql_free_result(childres);

    // fetch type-specific information
    MysqlObjectType* obtype = objecttypes[object->getType()];
    if(obtype != NULL){
        lock();
        bool sucessful = obtype->retrieve(conn, object);
        unlock();
        object->setModTime(strtoull(row[12], NULL, 10));
        if(!sucessful){
            Logger::getLogger()->error("Mysql: Could not retrieve object type specific data");
            delete object;
            object = NULL;
        }
    }else{
        Logger::getLogger()->error("Mysql: Object type %d not registered", object->getType());
        delete object;
        object = NULL;
    }
    mysql_free_result(obresult);
    return object;
}

bool MysqlPersistence::removeObject(uint32_t obid){
    std::ostringstream querybuilder;
    querybuilder << "SELECT type FROM object WHERE objectid=" << obid <<";";
    uint32_t objecttype;
    lock();
    try{
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: remove object query error: %s", mysql_error(conn));
            throw std::exception();
        }else{
            MYSQL_RES *objtyperes = mysql_store_result(conn);
            if(objtyperes == NULL){
                Logger::getLogger()->error("Mysql: remove object query result error: %s", mysql_error(conn));
                throw std::exception();
            }
            unlock();
    
            MYSQL_ROW row = mysql_fetch_row(objtyperes);
            if(row == NULL || row[0] == NULL){ 
                Logger::getLogger()->warning("Mysql remove object: object not found");
                throw std::exception();
            }
            objecttype = atoi(row[0]);
            mysql_free_result(objtyperes);
        }
    }catch(std::exception e){
        unlock();
        return false;
    }
    querybuilder.str("");
    querybuilder << "DELETE FROM object WHERE objectid=" << obid << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove object %d - %s", obid, mysql_error(conn));
        unlock();
        return false;
    }
    bool rtv;
    //store type-specific information
    MysqlObjectType* obtype = objecttypes[objecttype];
    if(obtype != NULL){
        rtv = obtype->remove(conn, obid);
    }else{
        Logger::getLogger()->error("Mysql: Object type %d not registered", objecttype);
        rtv = false;
    }
    unlock();
    return rtv;
    
}

uint32_t MysqlPersistence::getMaxObjectId(){
    lock();
    if(mysql_query(conn, "SELECT MAX(objectid) FROM object;") != 0){
        Logger::getLogger()->error("Mysql: Could not query max object id - %s", mysql_error(conn));
        unlock();
        return 0;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    unlock();
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: get max objectid: Could not store result - %s", mysql_error(conn));
        return 0;
    }
    MYSQL_ROW max = mysql_fetch_row(obresult);
    uint32_t maxid = 0;
    if(max[0] != NULL){
        maxid = atoi(max[0]);
    }
    mysql_free_result(obresult);
    return maxid;
}

std::set<uint32_t> MysqlPersistence::getObjectIds(){
    lock();
    if(mysql_query(conn, "SELECT objectid FROM object;") != 0){
        Logger::getLogger()->error("Mysql: Could not query object ids - %s", mysql_error(conn));
        unlock();
        return std::set<uint32_t>();
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    unlock();
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: get objectids: Could not store result - %s", mysql_error(conn));
        return std::set<uint32_t>();
    }
    MYSQL_ROW max;
    std::set<uint32_t> vis;
    while((max = mysql_fetch_row(obresult)) != NULL){
        vis.insert(atoi(max[0]));
    }
    mysql_free_result(obresult);
    return vis;
}

bool MysqlPersistence::saveOrder(uint32_t ordid, Order* ord){
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO ordertype VALUES (" << ordid << ", " << ord->getType() << ");";
     lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store order %d - %s", ordid, mysql_error(conn));
        unlock();
        return false;
    }
    bool rtv;
    //store type-specific information
    MysqlOrderType* ordtype = ordertypes[ord->getType()];
    if(ordtype != NULL){
        rtv = ordtype->save(this, conn, ordid, ord);
    }else{
        Logger::getLogger()->error("Mysql: Order type %d not registered", ord->getType());
        rtv = false;
    }
    unlock();
    return rtv;
}

bool MysqlPersistence::updateOrder(uint32_t ordid, Order* ord){
    std::ostringstream querybuilder;
    lock();
    bool rtv;
    //update type-specific information
    MysqlOrderType* ordtype = ordertypes[ord->getType()];
    if(ordtype != NULL){
        rtv = ordtype->update(this, conn, ordid, ord);
    }else{
        Logger::getLogger()->error("Mysql: Order type %d not registered", ord->getType());
        rtv = false;
    }
    unlock();
    return rtv;
}

Order* MysqlPersistence::retrieveOrder(uint32_t ordid){
    std::ostringstream querybuilder;
    querybuilder << "SELECT type FROM ordertype WHERE orderid = " << ordid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve order %d - %s", ordid, mysql_error(conn));
        unlock();
        return NULL;
    }
    MYSQL_RES *ordresult = mysql_store_result(conn);
    if(ordresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve order: Could not store result - %s", mysql_error(conn));
        unlock();
        return NULL;
    }
    unlock();
    MYSQL_ROW row = mysql_fetch_row(ordresult);
    uint32_t ordertype;
    if(row != NULL && row[0] != NULL){
        ordertype = atoi(row[0]);
    }else{
        mysql_free_result(ordresult);
        Logger::getLogger()->error("Mysql: retrieve order: no such order %d - %s", ordid, mysql_error(conn));
        return NULL;
   }
    mysql_free_result(ordresult);
    Order* order = Game::getGame()->getOrderManager()->createOrder(ordertype);
    // fetch type-specific information
    MysqlOrderType* ordtype = ordertypes[order->getType()];
    if(ordtype != NULL){
        lock();
        bool sucessful = ordtype->retrieve(conn, ordid, order);
        unlock();
        if(!sucessful){
            Logger::getLogger()->error("Mysql: Could not retrieve order type specific data");
            delete order;
            order = NULL;
        }
    }else{
        Logger::getLogger()->error("Mysql: Order type %d not registered", order->getType());
        delete order;
        order = NULL;
    }
    return order;
}

bool MysqlPersistence::removeOrder(uint32_t ordid){
    std::ostringstream querybuilder;
    querybuilder << "SELECT type FROM ordertype WHERE orderid=" << ordid <<";";
    uint32_t ordertype;
    lock();
    try{
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: remove order query error: %s", mysql_error(conn));
            throw std::exception();
        }else{
            MYSQL_RES *ordtyperes = mysql_store_result(conn);
            if(ordtyperes == NULL){
                Logger::getLogger()->error("Mysql: remove order query result error: %s", mysql_error(conn));
                throw std::exception();
            }
    
            MYSQL_ROW row = mysql_fetch_row(ordtyperes);
            if(row == NULL || row[0] == NULL){ 
                Logger::getLogger()->warning("Mysql remove order: order not found");
                throw std::exception();
            }
            ordertype = atoi(row[0]);
            mysql_free_result(ordtyperes);
        }
    }catch(std::exception e){
        unlock();
        return false;
    }
    querybuilder.str("");
    querybuilder << "DELETE FROM ordertype WHERE orderid=" << ordid << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove order %d - %s", ordid, mysql_error(conn));
        unlock();
        return false;
    }
    bool rtv;
    //store type-specific information
    MysqlOrderType* ordtype = ordertypes[ordertype];
    if(ordtype != NULL){
        rtv = ordtype->remove(conn, ordid);
    }else{
        Logger::getLogger()->error("Mysql: Order type %d not registered", ordertype);
        rtv = false;
    }
    unlock();
    return rtv;
}

bool MysqlPersistence::saveOrderList(uint32_t obid, std::list<uint32_t> list){
    std::ostringstream querybuilder;
    querybuilder << "DELETE FROM orderslot WHERE objectid=" << obid <<";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove orderslots for object %d - %s", obid, mysql_error(conn));
        unlock();
        return false;
    }
    if(!list.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO orderslot VALUES ";
        uint32_t slotnum = 0;
        for(std::list<uint32_t>::iterator itcurr = list.begin(); itcurr != list.end(); ++itcurr){
            if(itcurr != list.begin()){
                querybuilder << ", ";
            }
            querybuilder << "(" << obid << ", " << slotnum << ", " << (*itcurr) << ")";
            slotnum++;
        }
        querybuilder << ";";
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not store orderslots - %s", mysql_error(conn));
            return false;
        }
    }
    return true;
}

std::list<uint32_t> MysqlPersistence::retrieveOrderList(uint32_t obid){
    std::ostringstream querybuilder;
    querybuilder << "SELECT orderid FROM orderslot WHERE objectid=" << obid <<" ORDER BY slot;";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not query order list - %s", mysql_error(conn));
        unlock();
        return std::list<uint32_t>();
    }
    MYSQL_RES *ordresult = mysql_store_result(conn);
    unlock();
    if(ordresult == NULL){
        Logger::getLogger()->error("Mysql: get order list: Could not store result - %s", mysql_error(conn));
        return std::list<uint32_t>();
    }
    MYSQL_ROW max;
    std::list<uint32_t> oolist;
    while((max = mysql_fetch_row(ordresult)) != NULL){
        oolist.push_back(atoi(max[0]));
    }
    mysql_free_result(ordresult);

    return oolist;
}

std::set<uint32_t> MysqlPersistence::retrieveObjectsWithOrders(){
    lock();
    if(mysql_query(conn, "SELECT DISTINCT objectid FROM orderslot;") != 0){
        Logger::getLogger()->error("Mysql: Could not query objects with orders - %s", mysql_error(conn));
        unlock();
        return std::set<uint32_t>();
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    unlock();
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: get objects with orders: Could not store result - %s", mysql_error(conn));
        return std::set<uint32_t>();
    }
    MYSQL_ROW max;
    std::set<uint32_t> obset;
    while((max = mysql_fetch_row(obresult)) != NULL){
        obset.insert(atoi(max[0]));
    }
    mysql_free_result(obresult);

    return obset;
}

uint32_t MysqlPersistence::getMaxOrderId(){
    lock();
    if(mysql_query(conn, "SELECT MAX(orderid) FROM ordertype;") != 0){
        Logger::getLogger()->error("Mysql: Could not query max order id - %s", mysql_error(conn));
        unlock();
        return 0;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    unlock();
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: get max orderid: Could not store result - %s", mysql_error(conn));
        return 0;
    }
    MYSQL_ROW max = mysql_fetch_row(obresult);
    uint32_t maxid = 0;
    if(max[0] != NULL){
        maxid = atoi(max[0]);
    }
    mysql_free_result(obresult);
    return maxid;
}

bool MysqlPersistence::savePlayer(Player* player){
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO player VALUES (" << player->getID() << ", '" << addslashes(player->getName()) << "', '";
    querybuilder << addslashes(player->getPass()) << "', " << player->getBoardId() << ");";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store player %d - %s", player->getID(), mysql_error(conn));
        unlock();
        return false;
    }
    unlock();
    std::set<uint32_t> idset = player->getVisibleDesigns();
    if(!idset.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO playerdesignvisible VALUES ";
        for(std::set<uint32_t>::iterator itcurr = idset.begin(); itcurr != idset.end(); ++itcurr){
            if(itcurr != idset.begin())
                querybuilder << ", ";
            querybuilder << "(" << player->getID() << ", " << (*itcurr) << ")";
        }
        querybuilder << ";";
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not store player visible designs %d - %s", player->getID(), mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }
    idset = player->getUsableDesigns();
    if(!idset.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO playerdesignusable VALUES ";
        for(std::set<uint32_t>::iterator itcurr = idset.begin(); itcurr != idset.end(); ++itcurr){
            if(itcurr != idset.begin())
                querybuilder << ", ";
            querybuilder << "(" << player->getID() << ", " << (*itcurr) << ")";
        }
        querybuilder << ";";
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not store player usable designs %d - %s", player->getID(), mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }
    idset = player->getVisibleComponents();
    if(!idset.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO playercomponentusable VALUES ";
        for(std::set<uint32_t>::iterator itcurr = idset.begin(); itcurr != idset.end(); ++itcurr){
            if(itcurr != idset.begin())
                querybuilder << ", ";
            querybuilder << "(" << player->getID() << ", " << (*itcurr) << ")";
        }
        querybuilder << ";";
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not store player visible components %d - %s", player->getID(), mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }
    idset = player->getUsableComponents();
    if(!idset.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO playercomponentusable VALUES ";
        for(std::set<uint32_t>::iterator itcurr = idset.begin(); itcurr != idset.end(); ++itcurr){
            if(itcurr != idset.begin())
                querybuilder << ", ";
            querybuilder << "(" << player->getID() << ", " << (*itcurr) << ")";
        }
        querybuilder << ";";
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not store player usable components %d - %s", player->getID(), mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }
    idset = player->getVisibleObjects();
    if(!idset.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO playerobjectvisible VALUES ";
        for(std::set<uint32_t>::iterator itcurr = idset.begin(); itcurr != idset.end(); ++itcurr){
            if(itcurr != idset.begin())
                querybuilder << ", ";
            querybuilder << "(" << player->getID() << ", " << (*itcurr) << ")";
        }
        querybuilder << ";";
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not store player visible objects %d - %s", player->getID(), mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }
    return true;
}

bool MysqlPersistence::updatePlayer(Player* player){
    std::ostringstream querybuilder;
    querybuilder << "UPDATE player SET name='" << addslashes(player->getName()) << "', password='" << addslashes(player->getPass());
    querybuilder << "', boardid=" << player->getBoardId() << " WHERE playerid=" << player->getID() << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not update player %d - %s", player->getID(), mysql_error(conn));
        unlock();
        return false;
    }
    querybuilder.str("");
    querybuilder << "DELETE FROM playerdesignvisible WHERE playerid=" << player->getID() << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove player visible designs %d - %s", player->getID(), mysql_error(conn));
        unlock();
        return false;
    }
    querybuilder.str("");
    querybuilder << "DELETE FROM playerdesignusable WHERE playerid=" << player->getID() << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove player usable designs %d - %s", player->getID(), mysql_error(conn));
        unlock();
        return false;
    }
    querybuilder.str("");
    querybuilder << "DELETE FROM playercomponentvisible WHERE playerid=" << player->getID() << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove player visible components %d - %s", player->getID(), mysql_error(conn));
        unlock();
        return false;
    }
    querybuilder.str("");
    querybuilder << "DELETE FROM playercomponentusable WHERE playerid=" << player->getID() << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove player usable components %d - %s", player->getID(), mysql_error(conn));
        unlock();
        return false;
    }
    querybuilder.str("");
    querybuilder << "DELETE FROM playerobjectvisible WHERE playerid=" << player->getID() << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove player visible objects %d - %s", player->getID(), mysql_error(conn));
        unlock();
        return false;
    }
    unlock();
    std::set<uint32_t> idset = player->getVisibleDesigns();
    if(!idset.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO playerdesignvisible VALUES ";
        for(std::set<uint32_t>::iterator itcurr = idset.begin(); itcurr != idset.end(); ++itcurr){
            if(itcurr != idset.begin())
                querybuilder << ", ";
            querybuilder << "(" << player->getID() << ", " << (*itcurr) << ")";
        }
        querybuilder << ";";
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not update player visible designs %d - %s", player->getID(), mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }
    idset = player->getUsableDesigns();
    if(!idset.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO playerdesignusable VALUES ";
        for(std::set<uint32_t>::iterator itcurr = idset.begin(); itcurr != idset.end(); ++itcurr){
            if(itcurr != idset.begin())
                querybuilder << ", ";
            querybuilder << "(" << player->getID() << ", " << (*itcurr) << ")";
        }
        querybuilder << ";";
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not update player usable designs %d - %s", player->getID(), mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }
    idset = player->getVisibleComponents();
    if(!idset.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO playercomponentvisible VALUES ";
        for(std::set<uint32_t>::iterator itcurr = idset.begin(); itcurr != idset.end(); ++itcurr){
            if(itcurr != idset.begin())
                querybuilder << ", ";
            querybuilder << "(" << player->getID() << ", " << (*itcurr) << ")";
        }
        querybuilder << ";";
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not update player visible components %d - %s", player->getID(), mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }
    idset = player->getUsableComponents();
    if(!idset.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO playercomponentusable VALUES ";
        for(std::set<uint32_t>::iterator itcurr = idset.begin(); itcurr != idset.end(); ++itcurr){
            if(itcurr != idset.begin())
                querybuilder << ", ";
            querybuilder << "(" << player->getID() << ", " << (*itcurr) << ")";
        }
        querybuilder << ";";
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not update player usable components %d - %s", player->getID(), mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }
    idset = player->getVisibleObjects();
    if(!idset.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO playerobjectvisible VALUES ";
        for(std::set<uint32_t>::iterator itcurr = idset.begin(); itcurr != idset.end(); ++itcurr){
            if(itcurr != idset.begin())
                querybuilder << ", ";
            querybuilder << "(" << player->getID() << ", " << (*itcurr) << ")";
        }
        querybuilder << ";";
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not update player visible objects %d - %s", player->getID(), mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }
    return true;
}

Player* MysqlPersistence::retrievePlayer(uint32_t playerid){
    std::ostringstream querybuilder;
    querybuilder << "SELECT * FROM player WHERE playerid = " << playerid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve player %d - %s", playerid, mysql_error(conn));
        unlock();
        return NULL;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve player: Could not store result - %s", mysql_error(conn));
        unlock();
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    MYSQL_ROW row = mysql_fetch_row(obresult);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such player %d", playerid);
        mysql_free_result(obresult);
        return NULL;
    }
    Player* player = new Player();
    player->setId(playerid);
    player->setName(row[1]);
    player->setPass(row[2]);
    //Board atoi(row[3]);
    mysql_free_result(obresult);
    
    querybuilder.str("");
    querybuilder << "SELECT designid FROM playerdesignvisible WHERE playerid = " << playerid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve player visible designs %d - %s", playerid, mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    MYSQL_RES *res = mysql_store_result(conn);
    if(res == NULL){
        Logger::getLogger()->error("Mysql: retrieve player visible designs: Could not store result - %s", mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    while((row = mysql_fetch_row(res)) != NULL){
        player->addVisibleDesign(atoi(row[0]));
    }
    mysql_free_result(res);
    
    querybuilder.str("");
    querybuilder << "SELECT designid FROM playerdesignusable WHERE playerid = " << playerid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve player usable designs %d - %s", playerid, mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    res = mysql_store_result(conn);
    if(res == NULL){
        Logger::getLogger()->error("Mysql: retrieve player usable designs: Could not store result - %s", mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    while((row = mysql_fetch_row(res)) != NULL){
        player->addUsableDesign(atoi(row[0]));
    }
    mysql_free_result(res);
    
    querybuilder.str("");
    querybuilder << "SELECT componentid FROM playercomponentvisible WHERE playerid = " << playerid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve player visible components %d - %s", playerid, mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    res = mysql_store_result(conn);
    if(res == NULL){
        Logger::getLogger()->error("Mysql: retrieve player visible components: Could not store result - %s", mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    while((row = mysql_fetch_row(res)) != NULL){
        player->addVisibleComponent(atoi(row[0]));
    }
    mysql_free_result(res);
    
    querybuilder.str("");
    querybuilder << "SELECT componentid FROM playercomponentusable WHERE playerid = " << playerid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve player usable components %d - %s", playerid, mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    res = mysql_store_result(conn);
    if(res == NULL){
        Logger::getLogger()->error("Mysql: retrieve player usable components: Could not store result - %s", mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    while((row = mysql_fetch_row(res)) != NULL){
        player->addUsableComponent(atoi(row[0]));
    }
    mysql_free_result(res);
    
    querybuilder.str("");
    querybuilder << "SELECT objectid FROM playerobjectvisible WHERE playerid = " << playerid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve player visible objects %d - %s", playerid, mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    res = mysql_store_result(conn);
    if(res == NULL){
        Logger::getLogger()->error("Mysql: retrieve player visible objects: Could not store result - %s", mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    std::set<uint32_t> obids;
    while((row = mysql_fetch_row(res)) != NULL){
        obids.insert(atoi(row[0]));
    }
    mysql_free_result(res);
    player->setVisibleObjects(obids);
    
    return player;
}

uint32_t MysqlPersistence::getMaxPlayerId(){
    lock();
    if(mysql_query(conn, "SELECT MAX(playerid) FROM player;") != 0){
        Logger::getLogger()->error("Mysql: Could not query max player id - %s", mysql_error(conn));
        unlock();
        return 0;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    unlock();
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: get max playerid: Could not store result - %s", mysql_error(conn));
        return 0;
    }
    MYSQL_ROW max = mysql_fetch_row(obresult);
    uint32_t maxid = 0;
    if(max[0] != NULL){
        maxid = atoi(max[0]);
    }
    mysql_free_result(obresult);
    return maxid;
}

std::set<uint32_t> MysqlPersistence::getPlayerIds(){
    lock();
    if(mysql_query(conn, "SELECT playerid FROM player;") != 0){
        Logger::getLogger()->error("Mysql: Could not query player ids - %s", mysql_error(conn));
        unlock();
        return std::set<uint32_t>();
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    unlock();
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: get playerids: Could not store result - %s", mysql_error(conn));
        return std::set<uint32_t>();
    }
    MYSQL_ROW max;
    std::set<uint32_t> vis;
    while((max = mysql_fetch_row(obresult)) != NULL){
        vis.insert(atoi(max[0]));
    }
    mysql_free_result(obresult);
    return vis;
}

std::string MysqlPersistence::addslashes(const std::string& in) const{
    char* buf = new char[in.length() * 2 + 1];
    uint len = mysql_real_escape_string(conn, buf, in.c_str(), in.length());
    std::string rtv(buf, len);
    delete[] buf;
    return rtv;
}

uint32_t MysqlPersistence::getTableVersion(const std::string& name){
    if(mysql_query(conn, (std::string("SELECT version FROM tableversion WHERE name='") + addslashes(name) + "';").c_str()) != 0){
        Logger::getLogger()->error("Mysql: table version query error: %s", mysql_error(conn));
        throw std::exception();
    }else{
        MYSQL_RES *tableversion = mysql_store_result(conn);
        if(tableversion == NULL){
            Logger::getLogger()->error("Mysql: table versions query result error: %s", mysql_error(conn));
            throw std::exception();
        }

        MYSQL_ROW row = mysql_fetch_row(tableversion);
        if(row == NULL || row[0] == NULL){ 
            Logger::getLogger()->warning("Mysql: table version not found");
            throw std::exception();
        }
        uint32_t version = atoi(row[0]);
        mysql_free_result(tableversion);
        return version;
    }
}

void MysqlPersistence::addObjectType(MysqlObjectType* ot){
    objecttypes[ot->getType()] = ot;
    lock();
    ot->initialise(this, conn);
    unlock();
}

void MysqlPersistence::addOrderType(MysqlOrderType* ot){
    ordertypes[ot->getType()] = ot;
    lock();
    ot->initialise(this, conn);
    unlock();
}

void MysqlPersistence::lock(){
}

void MysqlPersistence::unlock(){
}
