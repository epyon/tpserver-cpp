/*  Main method for tpserver-cpp
 *
 *  Copyright (C) 2010  Kornel Kisielewicz and the Thousand Parsec Project
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

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

#include "settings.h"
#include "server.h"
#include "logging.h"


#if !defined(WIN32)

#include <pthread.h>
#include <signal.h>

#else // defined(WIN32)

// callback for shutting down the server from windows console control
boost::function< void(void) > gServerShutdownCallback;

// windows console control handler
BOOL WINAPI consoleHandler(DWORD ctrl_type)
{
    switch (ctrl_type)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        gServerShutdownCallback();
        return TRUE;
    default:
        return FALSE;
    }
}

#endif // defined(WIN32)

int main(int argc, char* argv[])
{
    try
    {
        Settings *settings = Settings::getSettings();
        settings->readArgs(argc, argv);
        if ( settings->get("NEVER_START") == "!" ) return 0;

        if ( !settings->readConfFile() ) 
        {
            std::string savedloglevel = settings->get("log_level");
            settings->set("log_level", "3");
            LOG_ERROR("Could not read config file");
            settings->set("log_level", savedloglevel);
        }

#if !defined(WIN32)

        // block unwanted signals
        sigset_t sig_block;
        sigset_t sig_wait;
        sigset_t sig_old;

        sigfillset( &sig_block );
        pthread_sigmask( SIG_BLOCK, &sig_block, &sig_old );

        // run server in background
        Server server_instance;
        boost::thread this_thread(
            boost::bind(
            &Server::run, 
            &server_instance 
            ) 
            );

        // restore signals
        pthread_sigmask( SIG_SETMASK, &sig_old, 0 );

        // wait for shut down signal 
        sigemptyset( &sig_wait );
        sigaddset( &sig_wait, SIGINT );
        sigaddset( &sig_wait, SIGQUIT );
        sigaddset( &sig_wait, SIGTERM );
        pthread_sigmask( SIG_BLOCK, &sig_wait, 0 );

        int signal = 0;
        sigwait( &sig_wait, &signal );

        // stop the server.
        server_instance.stop();
        this_thread.join();

#else // defined(WIN32)

        // create the server in this thread
        Server server_instance;

        // set console control handler to allow server to be stopped.
        //gServerShutdownCallback = boost::bind(
        //    &Server::stop, 
        //    &server_instance
        //    );

        // set the console handler via WINAPI
        SetConsoleCtrlHandler( consoleHandler, TRUE );

        // run the server until stopped.
        server_instance.run();

#endif

    }
    catch (std::exception& e)
    {
        LOG_ERROR("Caught exception, exiting. Exception: %s", e.what());
    }

    return 0;
}

