/*  Network abstraction for tpserver-cpp 
 *
 *  Copyright (C) 2003-2005, 2006, 2007, 2008  Lee Begg and the Thousand Parsec Project
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

#include <sys/select.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <cassert>
#include <boost/bind.hpp>
#include "algorithms.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "logging.h"
#include "settings.h"
#include "connection.h"
#include "playerconnection.h"
#include "adminconnection.h"
#include "tcpsocket.h"
#include "admintcpsocket.h"
#include "game.h"
#include "timercallback.h"
#include "asyncframe.h"

#include "net.h"


// Network Class methods

Network *Network::myInstance = NULL;


Network *Network::getNetwork()
{
  if (myInstance == NULL) {
    myInstance = new Network();
  }
  return myInstance;
}

void Network::addConnection(Connection::Ptr conn)
{
  DEBUG("Adding a file descriptor");
  connections[conn->getSocket()] = conn;
  FD_SET(conn->getSocket(), &master_set);
  if (max_fd < conn->getSocket()) {
    max_fd = conn->getSocket();
  }
}

void Network::removeConnection( int fd )
{
  DEBUG("Removing a file descriptor %d", fd);
  FD_CLR(fd, &master_set);
  connections.erase(connections.find(fd));
  if (max_fd == fd ) {
    DEBUG("Changing max_fd");
    max_fd = 0;
    // Map by int is sorted by int
    if ( !connections.empty() ) {
      max_fd = connections.rbegin()->first;
    }
  }
}

void Network::addToWriteQueue(Connection::Ptr conn){
  writequeue[conn->getSocket()] = conn;
}

void Network::addTimer(TimerCallback::Ptr callback){
  timers.push(callback);
}

void Network::start()
{
  if (active == true) {
    WARNING("Network already running");
    return;
  }

  if(Game::getGame()->isLoaded()){
    INFO("Starting Network");


    uint32_t numsocks = 0;
    TcpSocket::Ptr listensocket( new TcpSocket() );
    listensocket->openListen(Settings::getSettings()->get("tp_addr"), Settings::getSettings()->get("tp_port"));
    if(listensocket->getStatus() != Connection::DISCONNECTED){
      addConnection(listensocket);
      numsocks++;
    }else{
      WARNING("Could not listen on TP (tcp) socket");
    }
    if(numsocks != 0){
      INFO("Started network with %d listeners", numsocks);
      active = true;
    }

  }else{
    WARNING("Not starting network, game not yet loaded");
  }

}


void Network::stop()
{
  if (active) {
    INFO("Stopping Network");

    //Remove the advertisers first (might be managing connections)
    // replaced by while(!connections.empty()) loop
    while(!connections.empty()){
        removeConnection(connections.begin()->first);
    }

    active = false;
  } else {
    WARNING("Network already stopped");
  }
}

bool Network::isStarted() const{
  return active;
}

void Network::adminStart(){
  if(Settings::getSettings()->get("admin_tcp") == "yes"){
    AdminTcpSocket::Ptr admintcpsocket( new AdminTcpSocket() );
    admintcpsocket->openListen(Settings::getSettings()->get("admin_tcp_addr"), Settings::getSettings()->get("admin_tcp_port"));
    if(admintcpsocket->getStatus() != Connection::DISCONNECTED){
      addConnection(admintcpsocket);
    }else{
      WARNING("Could not listen on admin TCP socket");
    }
  }else{
    INFO("Not configured to start admin TCP socket");
  }
}

void Network::adminStop(){
  ConnMap::iterator itcurr = connections.begin();
  while (itcurr != connections.end()) {
    if ( itcurr->second->getType() == Connection::ADMIN || 
         itcurr->second->getType() == Connection::LISTENADMIN )
      removeConnection(itcurr->first);
    ++itcurr;
  }
}

void Network::sendToAll(AsyncFrame* aframe){
  ConnMap::iterator itcurr;
  for (itcurr = connections.begin(); itcurr != connections.end(); itcurr++) {
    PlayerConnection::Ptr currConn = boost::dynamic_pointer_cast<PlayerConnection>(itcurr->second);
    if(currConn != NULL && currConn->getStatus() == Connection::READY){
      OutputFrame::Ptr currFrame = currConn->createFrame( InputFrame::Ptr() );
      if(aframe->createFrame(currFrame)){
        currConn->sendFrame(currFrame);
      }
    }
  }
  delete aframe;
}

void Network::masterLoop()
{
  struct timeval tv;
  fd_set cur_set;
  halt = false;
  while (!halt) {

    //sleep(1);
    bool netstat = active;

    while(!timers.empty() && (timers.top()->getExpireTime() <= static_cast<uint64_t>(time(NULL)) ||
          !(timers.top()->isValid()))){
      TimerCallback::Ptr callback = timers.top();
      timers.pop();
      if(callback->isValid())
        callback->call();
    }
    if(timers.empty()){
      tv.tv_sec = 60;
      tv.tv_usec = 0;
    }else{
      tv.tv_sec = (timers.top()->getExpireTime() - time(NULL)) - 1;
      if(tv.tv_sec <= 0){
        tv.tv_sec = 0;
        tv.tv_usec = 200000;
      }else{
        tv.tv_usec = 0;
      }
    }
    fd_set write_set;
    FD_ZERO(&write_set);
    // Unsure what the macro FD_SET stands for, so I can't do for_each
    for(ConnMap::iterator itcurr = writequeue.begin();
        itcurr != writequeue.end(); ++itcurr){
      FD_SET(itcurr->first, &write_set);
    }

    cur_set = master_set;

    if (select(max_fd + 1, &cur_set, &write_set, NULL, &tv) > 0) {

      for(ConnMap::iterator itcurr = writequeue.begin();
          itcurr != writequeue.end(); ++itcurr){
        if(FD_ISSET(itcurr->first, &write_set)){
          Connection::Ptr conn = itcurr->second;
          writequeue.erase(itcurr);
          conn->processWrite();
          //use select again, don't check rest of list as it has changed.
          break;
        }
      }

      ConnMap::iterator itcurr;
      for (itcurr = connections.begin(); itcurr != connections.end(); itcurr++) {
        Connection::Ptr connection = itcurr->second;
        if (FD_ISSET(itcurr->first, &cur_set)) {
          connection->process();
        }
        if (connection->getStatus() == Connection::DISCONNECTED) {
          INFO("Closed connection");
          removeConnection(itcurr->first);
          //use select again, don't check rest of list as it has changed.
          break;
        }
      }

    }

    //advertiser->poll();

    if(netstat != active && active == false){
      for_each_key( connections.begin(), connections.end(), boost::bind( &Network::removeConnection, this, _1 ) );
      DEBUG("Network really stopped");
    }
  }
}

void Network::stopMainLoop(){
  halt = true;
}


Network::Network()
{

  max_fd = 0;
  FD_ZERO(&master_set);

  halt = false;
  active = false;

}


Network::~Network()
{
}


Network::Network(Network & rhs)
{
}


Network Network::operator=(Network & rhs)
{
  // please don't call me
  assert(0);
  return *this;
}

