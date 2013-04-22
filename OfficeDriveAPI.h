/**********************************************************\

  Auto-generated OfficeDriveAPI.h

\**********************************************************/

#include <string>

#include <sstream>
#include <boost/weak_ptr.hpp>
#include "JSAPIAuto.h"
#include "JSObject.h"
#include "BrowserHost.h"
//#include "PluginCore.h"
#include "OfficeDrive.h"

#include <stdlib.h>

#include <SimpleStreamHelper.h>
#include <BrowserStreamRequest.h>

#ifdef FB_X11
#include <dlfcn.h>
#include <signal.h>
#include <unistd.h>
#endif

#ifdef WIN32
#include <process.h>
#endif

#include <string.h>


#ifndef H_OfficeDriveAPI
#define H_OfficeDriveAPI

FB_FORWARD_PTR(OfficeDriveAPI)
class OfficeDriveAPI : public FB::JSAPIAuto
{
public:
    ////////////////////////////////////////////////////////////////////////////
    /// @fn OfficeDriveAPI::OfficeDriveAPI(const OfficeDrivePtr& plugin, const FB::BrowserHostPtr host)
    ///
    /// @brief  Constructor for your JSAPI object.
    ///         You should register your methods, properties, and events
    ///         that should be accessible to Javascript from here.
    ///
    /// @see FB::JSAPIAuto::registerMethod
    /// @see FB::JSAPIAuto::registerProperty
    /// @see FB::JSAPIAuto::registerEvent
    ////////////////////////////////////////////////////////////////////////////
    OfficeDriveAPI(const OfficeDrivePtr& plugin, const FB::BrowserHostPtr& host) :
        m_plugin(plugin), m_host(host)
    {
        registerMethod("echo",      make_method(this, &OfficeDriveAPI::echo));
        registerMethod("call",      make_method(this, &OfficeDriveAPI::call));
		registerMethod("callbackTest",      make_method(this, &OfficeDriveAPI::callbackTest));
		registerMethod("onload",        make_method(this, &OfficeDriveAPI::startDaemon));
        registerMethod("startDaemon",        make_method(this, &OfficeDriveAPI::startDaemon));
        registerMethod("stopDaemon",        make_method(this, &OfficeDriveAPI::stopDaemon));
        registerMethod("loadlibrary",      make_method(this, &OfficeDriveAPI::loadlibrary));
        registerMethod("testEvent", make_method(this, &OfficeDriveAPI::testEvent));
       // registerMethod("getWindow", make_method(this,&PluginWindow:: GetWindow));
        registerMethod("launchFileSelect",     make_method(this, &OfficeDriveAPI::launchFileSelect));
        registerMethod("connectToDaemon",     make_method(this, &OfficeDriveAPI::connect));
		registerMethod("connect",     make_method(this, &OfficeDriveAPI::connect2));
        // Read-write property
        registerProperty("testString",
                         make_property(this,
                                       &OfficeDriveAPI::get_testString,
                                       &OfficeDriveAPI::set_testString));
        
        // Read-only property
        registerProperty("version",
                         make_property(this,
                                       &OfficeDriveAPI::get_version));
        registerProperty("daemonpid",
                         make_property(this,
                                       &OfficeDriveAPI::get_daemonpid));
        registerProperty("lastpath",
                         make_property(this,
                                       &OfficeDriveAPI::get_lastpath));
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    /// @fn OfficeDriveAPI::~OfficeDriveAPI()
    ///
    /// @brief  Destructor.  Remember that this object will not be released until
    ///         the browser is done with it; this will almost definitely be after
    ///         the plugin is released.
    ///////////////////////////////////////////////////////////////////////////////
    virtual ~OfficeDriveAPI() {};

    OfficeDrivePtr getPlugin();
    
	std::string selectedPath;
    int daemonpid;
    
    FB::JSObjectPtr callback;

    // Read/Write property ${PROPERTY.ident}
    std::string get_testString();
    void set_testString(const std::string& val);

    // Read-only property ${PROPERTY.ident}
    std::string get_version();
    
    std::string get_lastpath();
    
    int get_daemonpid();
    
    // Method echo
    FB::variant echo(const FB::variant& msg);
    
    // Method call
	void call(const std::string& cmd);
    
	int connect();
	void connect2();

	void connectCallback(const bool success, const FB::HeaderMap& headers, 
						 const boost::shared_array<uint8_t>& data, const size_t size);

	int startDaemon();
	//int startDaemon(const char * name, FB::VariantList& args);
	//int startDaemon(const std::string& name, FB::VariantList& args);
	
    FB::variant stopDaemon();
    
    // Method loadlibrary
    FB::variant loadlibrary(const FB::variant& lib);
    
    // Event helpers
    FB_JSAPI_EVENT(test, 0, ());
    FB_JSAPI_EVENT(echo, 2, (const FB::variant&, const int));

    // Method test-event
    void testEvent();
	bool callbackTest(FB::JSObjectPtr callback); 
    void launchFileSelect(int fileType, bool multiple, int fileOperation, FB::JSObjectPtr callback); 
    
private:
    OfficeDriveWeakPtr m_plugin;
    FB::BrowserHostPtr m_host;
    
    std::string m_testString;
	void fileSelectCallback(const std::string &path, FB::JSObjectPtr callback);
	
    
};

#endif // H_OfficeDriveAPI

