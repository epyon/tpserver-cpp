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
#include "game.h"
#include "pluginmanager.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define VERSION "0.0.0"
#endif


Server::Server()
{
  
}

void Server::startAdmin()
{
    Settings* settings = Settings::getSettings();
    Logger *logger     = Logger::getLogger();

    if ( settings->get("admin_tcp") == "yes" ) 
    {
        mAdminPort.reset( new Acceptor(
            mIOS, 
            settings->get("admin_tcp")
            "22", 
            boost::bind( &Server::acceptAdmin, this )
        ));
    }
    else
    {
        logger->info("Not configured to start admin TCP socket");
    }
}

void Server::acceptAdmin( const boost::asio::ip::tcp::socket& socket )
{
    
}

void Server::acceptTp( const boost::asio::ip::tcp::socket& socket )
{

}

void Server::acceptHttp( const boost::asio::ip::tcp::socket& socket )
{

}


void Server::stopAdmin()
{
    mAdminPort.reset();
}

void Server::loadModules()
{
    // get singletons (TODO : remove singletons ;))
    Settings *settings     = Settings::getSettings();
    Logger *logger         = Logger::getLogger();
    Game *game             = Game::getGame();
    PluginManager* plugins = PluginManager::getPluginManager();

    plugins->start();

    // load tpscheme
    std::string tpschemename = settings->get("tpscheme");
    if(tpschemename == "auto" || tpschemename == "")
    {
        //Temp. should be able to do better than this.
        if ( plugins->loadTpScheme("tpguile") ) 
        {
            logger->info("Loaded TpScheme tpguile");
        }
        else
        {
            logger->warning("Did not load TpScheme \"tpguile\", trying tpmzscheme");
            if ( plugins->loadTpScheme("tpmzscheme") ) 
            {
                logger->info("Loaded TpScheme tpmzscheme");
            }
            else
            {
                logger->warning("Did not load TpScheme \"tpmzscheme\"");
            }
        }
    }
    else
    {
        logger->info("Loading TpScheme %s", tpschemename.c_str());
        if ( plugins->loadTpScheme(tpschemename) ) 
        {
            logger->info("Loaded TpScheme %s", tpschemename.c_str());
        }
        else
        {
            logger->warning("Did not load TpScheme \"%s\"", tpschemename.c_str());
        }
    }

    // load persistence
    std::string persistencename = settings->get("persistence");
    if ( persistencename != "" )
    {
        logger->info("Loading persistence method %s", persistencename.c_str());
        if ( plugins->loadPersistence(persistencename) ) 
        {
            logger->info("Loaded persistence method %s", persistencename.c_str());
        }
        else
        {
            logger->warning("Did not load persistence method \"%s\"", persistencename.c_str());
        }
    }

    // load ruleset
    std::string rulesetname = settings->get("ruleset");
    if ( rulesetname != "" )
    {
        logger->info( "Loading ruleset %s", rulesetname.c_str() );
        if ( plugins->loadRuleset(rulesetname) ) 
        {
            logger->info("Loaded ruleset %s", rulesetname.c_str() );
        }
        else
        {
            logger->warning("Did not load ruleset \"%s\"", rulesetname.c_str());
        }
    }


    if ( settings->get("game_load") == "yes" )
    {
        game->load();
    }

    if ( settings->get("game_start") == "yes" )
    {
        game->start();
    }

}



void Server::stop()
{
    mIOS.stop();
}

void Server::run()
{
    Logger *logger         = Logger::getLogger();
    Game *game             = Game::getGame();
    PluginManager* plugins = PluginManager::getPluginManager();
    Settings* settings     = Settings::getSettings();

    logger->info("tpserver-cpp " VERSION " starting");
    logger->info("This is GPL software, please see the COPYING file");

    try
    {
        loadModules();
        startAdmin();

        mMainPort.reset( new Acceptor( 
            mIOS, 
            settings->get("tp_addr"), 
            settings->get("tp_port") ), 
            boost::bind( &Server::acceptTp, this ) 
        );

        if ( settings->get("http") == "yes" )
        {
            mHTTPPort.reset( new Acceptor( 
                mIOS, 
                settings->get("http_addr"), 
                settings->get("http_port") ), 
                boost::bind( &Server::acceptHttp, this 
            );
        }

        mIOS.run();

        if ( game->isLoaded() ) 
        {
            game->saveAndClose();
        }

        stopAdmin();
    }
    catch(const std::exception& e)
    {
        logger->error("Caught exception, exiting. Exception: %s", e.what());
    }

    logger->info("TP-server exiting");
    logger->flush();
}

Server::~Server()
{
}

