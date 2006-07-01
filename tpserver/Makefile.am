## Process this file with automake to produce Makefile.in

bin_PROGRAMS = tpserver-cpp

if GNUTLS
GNUTLSSOURCES = tlsmanager.cpp tlssocket.cpp \
		playertlsconn.cpp httpssocket.cpp
GNUTLSHEADERS = tlsmanager.h tlssocket.h playertlsconn.h httpssocket.h 
else
GNUTLSSOURCES =
GNUTLSHEADERS =
endif

tpserver_cpp_SOURCES = $(GNUTLSSOURCES) main.cpp connection.cpp console.cpp \
	frame.cpp game.cpp logging.cpp net.cpp settings.cpp player.cpp object.cpp \
	order.cpp objectdata.cpp universe.cpp ownedobject.cpp emptyobject.cpp nop.cpp \
	vector3d.cpp board.cpp message.cpp ordermanager.cpp objectdatamanager.cpp \
	combatstrategy.cpp playerconnection.cpp playertcpconn.cpp tcpsocket.cpp property.cpp \
	propertyvalue.cpp component.cpp design.cpp designstore.cpp tpscheme.cpp category.cpp \
	persistence.cpp objectmanager.cpp playermanager.cpp listensocket.cpp boardmanager.cpp \
	filelogger.cpp syslogger.cpp consolelogger.cpp resourcedescription.cpp \
	resourcemanager.cpp pluginmanager.cpp
include_HEADERS = connection.h console.h \
	frame.h game.h logging.h net.h settings.h \
	player.h object.h \
	order.h objectdata.h universe.h ownedobject.h \
	emptyobject.h nop.h \
	vector3d.h board.h message.h ordermanager.h \
	objectdatamanager.h combatstrategy.h \
	playerconnection.h \
	refsys.h property.h propertyvalue.h component.h \
	design.h designstore.h \
	ruleset.h tpscheme.h category.h persistence.h \
	objectmanager.h \
	playermanager.h listensocket.h boardmanager.h \
	resourcedescription.h resourcemanager.h
noinst_HEADERS = playertcpconn.h tcpsocket.h filelogger.h syslogger.h \
	consolelogger.h logsink.h $(GNUTLSHEADERS) pluginmanager.h
tpserver_cpp_LDADD = -ldl $(MODULE_LIBS) $(LIBGNUTLS_LIBS)
tpserver_cpp_LDFLAGS = -rdynamic