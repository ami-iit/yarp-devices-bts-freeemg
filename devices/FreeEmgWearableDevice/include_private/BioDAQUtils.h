#ifndef FREE_EMG_BIODAQ_UTILS_H
#define FREE_EMG_BIODAQ_UTILS_H

#include <string>
#include <olectl.h>

namespace FreeEmg
{

const OLE_COLOR ColorBlue = static_cast<OLE_COLOR>(RGB(0,0,255));       // blue
const OLE_COLOR ColorRed = static_cast<OLE_COLOR>(RGB(255,0,0));        // red
const OLE_COLOR ColorYellow = static_cast<OLE_COLOR>(RGB(255,255,0));   // yellow
const OLE_COLOR ColorGreen = static_cast<OLE_COLOR>(RGB(0,255,0));      // green 
const OLE_COLOR ColorBlack = static_cast<OLE_COLOR>(RGB(0,0,0));        // black
const OLE_COLOR ColorLime = static_cast<OLE_COLOR>(RGB(0,128,0));       // lime (C# interprets as green color)

bool colorFromName(std::string name, OLE_COLOR& color);


const long BIODAQ_EMG_SENSOR_CHANNEL_IDX = 0;
const int BIODAQ_EMG_SENSOR_MIN_LABEL = 1;
const int BIODAQ_EMG_SENSOR_MAX_LABEL = 255;

static const int MAC_ADDRESS_BYTES = 8;
const std::string EMG_SENSOR_MAC_PREFIX = std::string("02B75E0005");

/**
 * @brief Validates a MAC address without delimiters
 * 
 * @param macAddress the MAC address without 
 * @return true if macAddress is a valid MAC address
 * @return false otherwise
 */
bool validateMacAddress(const std::string& macAddress);

/**
 * @brief Create the reversed binary MAC address
 * 
 * @param macAddressString the MAC address
 * @param MAC_ADDRESS_BYTES the output reversed MAC address 
 * @return true on success
 * @return false on failure
 */
bool createReversedMacAddress(const std::string& macAddressString, unsigned char MAC_ADDRESS_BYTES[MAC_ADDRESS_BYTES]);

/**
 * @brief Create the reversed binary MAC address as SAFEARRAY
 * 
 * @param macAddressString the MAC address
 * @return a pointer to the SAFEARRAY containing the reversed binary MAC address
 */
SAFEARRAY* createMacAddressSafeArray(const std::string& macAddress);

} // namespace FreeEmg

#endif // FREE_EMG_BIODAQ_UTILS_H
