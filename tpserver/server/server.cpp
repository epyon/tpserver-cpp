/*  tpserver main server singleton class
 *
 *  Copyright (C) 2010 Kornel Kisielewicz and the Thousand Parsec Project
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

#include "server.h"

#include <boost/make_shared.hpp>
#include <boost/bind.hpp>

#include "settings.h"
#include "logging.h"
#include "playerconnection.h"
#include "adminconnection.h"
//#include "game.h"
//#include "pluginmanager.h"

#include "system.h"

Server::Server()
  : mStrand( mIOS )
{
}

void Server::startAdmin()
{
  Settings* settings = Settings::getSettings();
  
  if ( settings->get("admin_tcp") != "yes" ) 
  {
    LOG_INFO("Server : not configured to start admin TCP socket");
    return;
  }

  std::string addr = settings->get("admin_tcp_addr");
  std::string port = settings->get("admin_tcp_port");

  if ( addr.empty() ) addr = "localhost";
  if ( port.empty() ) port = "6925";

  LOG_INFO("Server : starting admin connection on %s:%s...",addr.c_str(),port.c_str());
  
  mAdminPort.reset( new Acceptor( 
        mIOS, addr, port,
        boost::bind( &Server::createConnection, this, Connection::ADMIN )
        ));

  LOG_INFO("Server : admin port is ready");
}

Connection::Ptr Server::createConnection( Connection::Type type )
{
  switch( type ) 
  {
    case Connection::PLAYER : return Connection::Ptr( new PlayerConnection( mIOS, mStrand ) );
    case Connection::ADMIN  : return Connection::Ptr( new AdminConnection( mIOS, mStrand ) );
  }
  return Connection::Ptr( (Connection*)NULL );  
}

void Server::stopAdmin()
{
  LOG_INFO("Server : stopping admin socket...");
  mAdminPort.reset();
  LOG_INFO("Server : admin socket stoped");
}

void Server::loadModules()
{
  LOG_INFO("Server : loading modules...");
    // get singletons (TODO : remove singletons ;))
    Settings *settings     = Settings::getSettings();
//    Game *game             = Game::getGame();
//    PluginManager* plugins = PluginManager::getPluginManager();

//    plugins->start();

    // load tpscheme
    std::string tpschemename = settings->get("tpscheme");
    if(tpschemename == "auto" || tpschemename == "")
    {
        //Temp. should be able to do better than this.
/*        if ( plugins->loadTpScheme("tpguile") ) 
        {
            LOG_INFO("Loaded TpScheme tpguile");
        }
        else
        {
            LOG_WARNING("Did not load TpScheme \"tpguile\", trying tpmzscheme");
            if ( plugins->loadTpScheme("tpmzscheme") ) 
            {
                LOG_INFO("Loaded TpScheme tpmzscheme");
            }
            else
            {
                LOG_WARNING("Did not load TpScheme \"tpmzscheme\"");
            }
        }
		*/
    }
    else
    {
        LOG_INFO("Loading TpScheme %s", tpschemename.c_str());
/*        if ( plugins->loadTpScheme(tpschemename) ) 
        {
            LOG_INFO("Loaded TpScheme %s", tpschemename.c_str());
        }
        else
        {
            LOG_WARNING("Did not load TpScheme \"%s\"", tpschemename.c_str());
        }
		*/
    }

    // load persistence
    std::string persistencename = settings->get("persistence");
    if ( persistencename != "" )
    {
        LOG_INFO("Loading persistence method %s", persistencename.c_str());
/*        if ( plugins->loadPersistence(persistencename) ) 
        {
            LOG_INFO("Loaded persistence method %s", persistencename.c_str());
        }
        else
        {
            LOG_WARNING("Did not load persistence method \"%s\"", persistencename.c_str());
        }*/
    }

    // load ruleset
    std::string rulesetname = settings->get("ruleset");
    if ( rulesetname != "" )
    {
        LOG_INFO( "Loading ruleset %s", rulesetname.c_str() );
/*        if ( plugins->loadRuleset(rulesetname) ) 
        {
            LOG_INFO("Loaded ruleset %s", rulesetname.c_str() );
        }
        else
        {
            LOG_WARNING("Did not load ruleset \"%s\"", rulesetname.c_str());
        }*/
    }


    if ( settings->get("game_load") == "yes" )
    {
//        game->load();
    }

    if ( settings->get("game_start") == "yes" )
    {
//        game->start();
    }
  
  LOG_INFO("Server : modules loaded");

}



void Server::stop()
{
    mIOS.stop();
}

void Server::run()
{
//    Game *game             = Game::getGame();
//    PluginManager* plugins = PluginManager::getPluginManager();
    Settings* settings     = Settings::getSettings();

    LOG_INFO("tpserver-cpp " VERSION " starting");
    LOG_INFO("This is GPL software, please see the COPYING file");

    try
    {
        loadModules();
        startAdmin();

        LOG_INFO("Server : opening TP protocol port...");
        mMainPort.reset( new Acceptor( 
            mIOS, 
            settings->get("tp_addr"), 
            settings->get("tp_port"), 
            boost::bind( &Server::createConnection, this, Connection::PLAYER ) 
        ) );
        LOG_INFO("Server : TP protocol port opened");

        if ( settings->get("http") == "yes" )
        {
/*            mHTTPPort.reset( new Acceptor( 
                mIOS, 
                settings->get("http_addr"), 
                settings->get("http_port"),
                boost::bind( &Server::acceptHttp, this, _1 )
            ) );
*/        }

        LOG_INFO("Server : entering asynchronous operation mode...");
        mIOS.run();
        LOG_INFO("Server : left asynchronous operation mode");

//        if ( game->isLoaded() ) 
//        {
//            game->saveAndClose();
//        }

        stopAdmin();
    }
    catch(const std::exception& e)
    {
        LOG_ERROR("Caught exception, exiting. Exception: %s", e.what());
    }

    LOG_INFO("TP-server exiting");
	Logger::getLogger()->flush();
}

Server::~Server()
{
	
}

