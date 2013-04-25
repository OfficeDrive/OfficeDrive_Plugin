/**********************************************************\

  Auto-generated OfficeDriveAPI.cpp

\**********************************************************/

#define BOOST_FILESYSTEM_VERSION 3

#include "JSObject.h"
#include "variant_list.h"
#include "DOM/Document.h"
#include "global/config.h"

// #include <boost/filesystem.hpp>
// #include <boost/filesystem/fstream.hpp>

#include "PluginCore.h"
#include "OfficeDriveAPI.h"

#include "DOM/Window.h"
#include "NPObjectAPI.h"
#include "NpapiBrowserHost.h"
#include "DialogManager.h"

// #include "Win/DialogManagerWin.h"
// #include "X11/DialogManagerX11.h"

// using namespace boost::filesystem;
// using namespace FB;
// using namespace FB::Npapi;


///////////////////////////////////////////////////////////////////////////////
/// @fn FB::variant OfficeDriveAPI::echo(const FB::variant& msg)
///
/// @brief  Echos whatever is passed from Javascript.
///         Go ahead and change it. See what happens!
///////////////////////////////////////////////////////////////////////////////
FB::variant OfficeDriveAPI::echo(const FB::variant& msg)
{
    //static int n(0);
    //fire_echo("So far, you clicked this many times: ", n++);
    return msg;
}

FB::variant OfficeDriveAPI::loadlibrary(const FB::variant& lib)
{
    void *handle;
#ifdef FB_X11
    handle = dlopen(lib, RTLD_LAZY);
#endif
    return handle;
}

void OfficeDriveAPI::call(const std::string& cmd)
{
    // send command to daemon
    //FB::DOM::WindowPtr window = m_host->getDOMWindow();
 
    // Check if the DOM Window has an the property console
    //if (window && window->getJSObject()->HasProperty("officedrive")) {
        // Create a reference to the browswer console object
      //  FB::JSObjectPtr obj = window->getProperty<FB::JSObjectPtr>("officedrive");
 
        // Invoke the "log" method on the console object
	    //obj->Invoke(cmd, FB::variant_list_of(params));
    //return "Ok";
			
			//FB::DOM::WindowPtr window = m_host->getDOMWindow();
			//FB::JSObjectPtr obj (window->getJSObject());
			//FB::JSObjectPtr obj = window->getProperty<FB::JSObjectPtr>("officedrive");
			
			OfficeDrivePtr plugin = getPlugin();
			FB::variant params = "";
			this->Invoke((L"%s", cmd), FB::variant_list_of(params));
			
}


void OfficeDriveAPI::connect2()
{
	FB::PluginEventSinkPtr eventSinkPtr;
	boost::optional<std::string> userId = getPlugin()->getParam("userId");
	std::string uid;
	uid = "None";
	if (userId) {
		uid = *userId;
	}

	m_host->createStream(FB::URI::url_decode("http://localhost:8881/&userId=" + uid),eventSinkPtr,0);

}
	
int OfficeDriveAPI::connect()
{
	bool result = FALSE;
	std::string postdata = NULL;

	boost::optional<std::string> userId = getPlugin()->getParam("userId");
	if (userId) {
		postdata = *userId;
	}
	result = FB::SimpleStreamHelper::AsyncPost(m_host, FB::URI::url_decode("http://localhost:8881/cmd"), 
												postdata, 
												boost::bind(&OfficeDriveAPI::connectCallback, this, _1,_2,_3,_4));
	return result;

}
void OfficeDriveAPI::connectCallback(const bool success, const FB::HeaderMap& headers, 
							  const boost::shared_array<uint8_t>& data, const size_t size) {
	
					
								  
		if (success) {
			m_host->htmlLog("Success");
			std::string dstr(reinterpret_cast<const char*>(data.get()), size);
			m_host->htmlLog(dstr);
			call(dstr);
			
	} else {
		m_host->htmlLog("Fail");
	}
}


/* int OfficeDriveAPI::startDaemon(const std::string& name, FB::VariantList& args)
{

	std::string name = "c:\\windows\\system32\\cmd.exe";
	FB::VariantList& args = {"cmd.exe", "/k", "java.exe -jar %LOCALAPPDATA%\\OfficeDrive\\client.jar", NULL};
	startDaemon(name, args);

}
*/

int OfficeDriveAPI::startDaemon()
{
    int ret; 
	int forkret;

#ifdef FB_X11
	const char *name[] = {"/full/path/to/your/executable"};
	char* const args[] = {"executablefilename", "first_argument", NULL};
    
    forkret = fork();
   
    if (forkret == 0) {
        ret = execv(name[0], args);
    }
#endif
#ifdef WIN32
	const char *localName[] = {"c:\\windows\\system32\\cmd.exe"};
	char* const localArgs[] = {"cmd.exe", "/k", "java.exe -jar %LOCALAPPDATA%\\OfficeDrive\\client.jar", NULL};
	ret = _spawnv(_P_NOWAIT, localName[0], localArgs);
    forkret = ret;
#endif
   daemonpid = forkret;
   return forkret;

}

FB::variant OfficeDriveAPI::stopDaemon()
{
    int ret = 0;
#ifdef FB_X11
    
	if (daemonpid > 0){
        ret = kill(daemonpid, 15);
     }
#endif
    if (ret != 0){
        // failed to kill daemon process
        echo("fail :-(");
    } else {
        daemonpid = -1;
    }
    return ret;
    // stop daemon?

}


///////////////////////////////////////////////////////////////////////////////
/// @fn OfficeDrivePtr OfficeDriveAPI::getPlugin()
///
/// @brief  Gets a reference to the plugin that was passed in when the object
///         was created.  If the plugin has already been released then this
///         will throw a FB::script_error that will be translated into a
///         javascript exception in the page.
///////////////////////////////////////////////////////////////////////////////
OfficeDrivePtr OfficeDriveAPI::getPlugin()
{
    OfficeDrivePtr plugin(m_plugin.lock());
    if (!plugin) {
        throw FB::script_error("The plugin is invalid");
    }
    return plugin;
}

// Read/Write property testString
std::string OfficeDriveAPI::get_testString()
{
    return m_testString;
}

void OfficeDriveAPI::set_testString(const std::string& val)
{
    m_testString = val;
}

// Read-only property version
std::string OfficeDriveAPI::get_version()
{
    return FBSTRING_PLUGIN_VERSION;
}

int OfficeDriveAPI::get_daemonpid()
{
    return daemonpid;
}

std::string OfficeDriveAPI::get_lastpath()

{
    return selectedPath;
}

void OfficeDriveAPI::testEvent()
{
    fire_test();
}
bool OfficeDriveAPI::callbackTest(FB::JSObjectPtr callback)
{
	callback->Invoke("", FB::variant_list_of(""));
	m_host->htmlLog("callbackTest success");
	return true;
	
}

void OfficeDriveAPI::launchFileSelect(int fileType, bool multiple, int fileOperation, FB::JSObjectPtr callback)
{
    DialogManager* dlgMgr = DialogManager::get();
    OfficeDrivePtr plugin = getPlugin();
	if ( fileType == 1) {
		dlgMgr->OpenFileDialog(m_host, plugin->GetWindow(), boost::bind(&OfficeDriveAPI::fileSelectCallback, this, _1, callback), fileType, multiple, fileOperation);
	} else {
		dlgMgr->OpenFolderDialog(m_host, plugin->GetWindow(), boost::bind(&OfficeDriveAPI::fileSelectCallback, this, _1, callback), fileType, multiple, fileOperation);
	    
	}
}
void OfficeDriveAPI::fileSelectCallback(const std::string &path, FB::JSObjectPtr callback)
{
    callback->Invoke("",FB::variant_list_of(path));
    selectedPath = path;
	m_host->htmlLog(selectedPath);
}

