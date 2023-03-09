#include "BioDAQUtils.h"

#include <map>
#include <regex>

// pattern of the MAC address
static const std::regex _macAddressPattern(
    "^([0-9A-Fa-f]{16})$");

static std::map<std::string, OLE_COLOR> _makeColorMap()
{
    std::map<std::string, OLE_COLOR> colorMap;

    colorMap.insert({"BLACK", FreeEmg::ColorBlack});
    colorMap.insert({"BLUE",  FreeEmg::ColorBlue});
    colorMap.insert({"GREEN", FreeEmg::ColorGreen});
    colorMap.insert({"LIME",  FreeEmg::ColorLime});
    colorMap.insert({"RED",   FreeEmg::ColorRed});
    colorMap.insert({"YELLOW", FreeEmg::ColorYellow});
    
    return colorMap;
}

static const std::map<std::string, OLE_COLOR> _BIODAQ_NAME_TO_COLOR = _makeColorMap();

bool FreeEmg::colorFromName(std::string name, OLE_COLOR& color){

    // clean name string from spaces and make it uppercase
    auto it = std::remove_if(name.begin(), name.end(), [](auto x){return x == ' ';});
    name.erase(it, name.end());
    for(int i=0; i<name.size(); i++) name[i] = std::toupper(name[i]);

    // look for the color in the supported table
    auto mapIt = _BIODAQ_NAME_TO_COLOR.find(name); 
    if( mapIt ==_BIODAQ_NAME_TO_COLOR.end()){
        color = FreeEmg::ColorBlue; //default value
        return false;
    }

    color = mapIt->second;

    return true;
}

bool FreeEmg::validateMacAddress(const std::string& macAddress)
{
    return (!macAddress.empty() && std::regex_match(macAddress, _macAddressPattern) );
}


//[ -----------------------------------------------------------------------------
//[ hex_to_int
//[
//! Convert hex value to integer
//!
//! @param c hex value
//!
//! @return  Integer value
//!
static unsigned char hex2int(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return 0;
}

//[ -----------------------------------------------------------------------------
//[ hex_to_ascii
//[
//! Convert 2 hexadecimal digits to integer
//!
//! @param c hex value
//! @param d hex value
//!
//! @return  Integer value
//!
static unsigned char hex2ascii( char c, char d )
{
  unsigned char high = hex2int(c) * 16;
  unsigned char low = hex2int(d);
  return high+low;
}

bool FreeEmg::createReversedMacAddress(const std::string& macAddressString, unsigned char reversedAddress[MAC_ADDRESS_BYTES])
{
    if(!validateMacAddress(macAddressString)){
        return false;
    }

    int j = 0;
    for(int i = 1; i < macAddressString.size(); i = i+2){
        reversedAddress[MAC_ADDRESS_BYTES-1 -j] = hex2ascii(macAddressString[i-1], macAddressString[i]);
        j++;
    }

    return true;
}

SAFEARRAY* FreeEmg::createMacAddressSafeArray(const std::string& macAddress)
{
    // reversing MAC address (mandatory for COM interop marshalling function parameters)
    // and ignoring separators
    unsigned char binAddrR[MAC_ADDRESS_BYTES];
    if(!FreeEmg::createReversedMacAddress(macAddress,binAddrR)){
        return false;
    }

    SAFEARRAYBOUND bound[1] = {MAC_ADDRESS_BYTES, 0}; 
    SAFEARRAY * address = SafeArrayCreate(VT_UI1, 1, bound); 
    void * pvData;
    SafeArrayAccessData(address, &pvData);
    memcpy(pvData, binAddrR, MAC_ADDRESS_BYTES);
    SafeArrayUnaccessData(address);

    return address;
}
