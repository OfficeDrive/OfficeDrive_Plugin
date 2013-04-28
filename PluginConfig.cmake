#/**********************************************************\ 
#
# Auto-Generated Plugin Configuration file
# for OfficeDrive
#
#\**********************************************************/

set(PLUGIN_NAME "OfficeDrive")
set(PLUGIN_PREFIX "NPOD")
set(COMPANY_NAME "1Site1")

# ActiveX constants:
set(FBTYPELIB_NAME OfficeDriveLib)
set(FBTYPELIB_DESC "OfficeDrive 1.0 Type Library")
set(IFBControl_DESC "OfficeDrive Control Interface")
set(FBControl_DESC "OfficeDrive Control Class")
set(IFBComJavascriptObject_DESC "OfficeDrive IComJavascriptObject Interface")
set(FBComJavascriptObject_DESC "OfficeDrive ComJavascriptObject Class")
set(IFBComEventSource_DESC "OfficeDrive IFBComEventSource Interface")
set(AXVERSION_NUM "1")

# NOTE: THESE GUIDS *MUST* BE UNIQUE TO YOUR PLUGIN/ACTIVEX CONTROL!  YES, ALL OF THEM!
set(FBTYPELIB_GUID 3ea27499-7c9b-50f6-942b-90a81f623848)
set(IFBControl_GUID 9737dc5b-a7aa-5d2a-9676-3af3a0abd803)
set(FBControl_GUID 46a38939-9024-546f-9aac-4bc84a281309)
set(IFBComJavascriptObject_GUID 4044f621-660f-5f92-849b-0e44ee7134eb)
set(FBComJavascriptObject_GUID 03ccb9de-1415-510d-83da-c2f4cc593afa)
set(IFBComEventSource_GUID f01d2aac-4a57-59f7-aede-7b0df382e157)
if ( FB_PLATFORM_ARCH_32 )
    set(FBControl_WixUpgradeCode_GUID 4ed692b9-660c-5575-a37b-32fece8dd379)
else ( FB_PLATFORM_ARCH_32 )
    set(FBControl_WixUpgradeCode_GUID 21ab4c88-95a7-573f-9f6a-f1a1880c023a)
endif ( FB_PLATFORM_ARCH_32 )

# these are the pieces that are relevant to using it from Javascript
set(ACTIVEX_PROGID "1Site1.OfficeDrive")
set(MOZILLA_PLUGINID "1Site1.com/OfficeDrive")

# strings
set(FBSTRING_CompanyName "1Site1.nl")
set(FBSTRING_PluginDescription "OfficeDrive")
set(FBSTRING_PLUGIN_VERSION "1.0.0.0")
set(FBSTRING_LegalCopyright "Copyright 2013 1Site1.nl")
set(FBSTRING_PluginFileName "npOfficeDrive.dll")
set(FBSTRING_ProductName "OfficeDrive")
set(FBSTRING_FileExtents "")
set(FBSTRING_PluginName "OfficeDrive")  # No 32bit postfix to maintain backward compatability.
set(FBSTRING_MIMEType "application/x-OfficeDrive")

# Uncomment this next line if you're not planning on your plugin doing
# any drawing:

set (FB_GUI_DISABLED 0)

# Mac plugin settings. If your plugin does not draw, set these all to 0
set(FBMAC_USE_QUICKDRAW 0)
set(FBMAC_USE_CARBON 0)
set(FBMAC_USE_COCOA 0)
set(FBMAC_USE_COREGRAPHICS 0)
set(FBMAC_USE_COREANIMATION 0)
set(FBMAC_USE_INVALIDATINGCOREANIMATION 0)

# If you want to register per-machine on Windows, uncomment this line
#set (FB_ATLREG_MACHINEWIDE 1)

set (BOOST_FILESYSTEM_V3 1)
add_boost_library(filesystem)
