/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/



/**
* @defgroup devicesettings
* @{
* @defgroup ds
* @{
**/


#include <iostream>
#include <algorithm>
#include <string.h>
#include "iarmProxy.hpp"
#include "audioOutputPortConfig.hpp"
#include "videoOutputPortConfig.hpp"
#include "list.hpp"
#include "host.hpp"
#include "videoDeviceConfig.hpp"
#include "dsVideoPort.h"
#include "dsVideoDevice.h"
#include "dsAudio.h"
#include "dsDisplay.h"
#include "dslogger.h"
#include "dsHost.h"
#include "dsTypes.h"
#include "unsupportedOperationException.hpp"
#include "hostEDID.hpp"


/**
 * @file host.cpp
 * @brief The host module is the central module of the Device Settings module.
 * Each devices establishes one and only one host instance that represents the entire host device.
 */
using namespace std;

namespace device 
{
   	
	const int Host::kPowerOn = dsPOWER_ON;
    const int Host::kPowerOff = dsPOWER_OFF;
    const int Host::kPowerStandby = dsPOWER_STANDBY;
 
	
	Host::Host() 
	{
		// TODO Auto-generated destructor stub
	}

    Host::~Host() {
    	if (true)
		{
			IARMProxy::getInstance().UnRegisterPowerEventHandler();
		}
    }


/**
 * @fn Host& Host::getInstance()
 * @brief This API is used to get a reference to the single instance of the Host object.
 *
 * @return reference to Host singleton instance
 */
    Host& Host::getInstance()
    {
        static Host instance;
        static bool isFirstTime = true;
        try {
			if (isFirstTime) {
				isFirstTime = false;
            }
            else {
            }
        }
        catch (...) {
    		cout << "Host Exception Thrown ...!\n";
        }
        return instance;
    }


/**
 * @fn void Host::addPowerModeListener(PowerModeChangeListener *l)
 * @brief This API is used to register listeners for Power Mode change event.
 * The listener object is created by application and should be released by the application once the listener is removed.
 * Listeners will be notified with the new mode via the listener's powerModeChanged() callback.
 *
 * @param[in] PowerModeChangeListener Pointer to Power Mode change listener
 *
 * @return None
 */
    void Host::addPowerModeListener(PowerModeChangeListener *l)
    {
        std::list < PowerModeChangeListener* > ::iterator it;
        
        it = find (powerEvntListeners.begin(),powerEvntListeners.end(), l);
        if (it == powerEvntListeners.end())
        {
            powerEvntListeners.push_back (l);
            cout << "Added Power Mode listener...!\n";
        }
        else
        {
            cout << "Already register for Power Mode Change\n";
        }
        return ;
    }


/**
 * @fn void Host::removePowerModeChangeListener(PowerModeChangeListener *l)
 * @brief This API is used to remove a listener from Power Mode change listener list.
 *
 * @param[in]PowerModeChangeListener The listener to remove.
 *
 * @return None
 */
    void Host::removePowerModeChangeListener(PowerModeChangeListener *l)
    {
        std::list < PowerModeChangeListener* > ::iterator it ;
        it = find (powerEvntListeners.begin(),powerEvntListeners.end(), l);
        if (it == powerEvntListeners.end())
        {
            cout << "Not Registered for Power Mode change yet...!\n";
        }
        else
        {
            powerEvntListeners.erase (it);
            cout << "Removed from Power Mode listener group..!\n";
        }
        return;
    }


/**
 * @fn void Host::addDisplayConnectionListener (DisplayConnectionChangeListener *l)
 * @brief This API is used to register listeners for Display connection change event.
 * The listener will be notified if Display device is connected/disconnected from the video output port.
 * The notification only carries the state change of the connection.
 * It does not carry any other system state change that may have been triggered by the connection.
 * The application is responsible to query the various parts of system to detect any such change.
 * For example, when a TV device is replaced, the application shall query the video output port again upon the connection
 * for the new resolution supported by the TV.
 * The listener object is created by application and should be released by the application once the listener is removed.
 *
 * @param[in] DisplayConnectionChangeListener Pointer to Display connection change listener
 *
 * @return None
 */
    void Host::addDisplayConnectionListener (DisplayConnectionChangeListener *l)
    {
        std::list < DisplayConnectionChangeListener* > ::iterator it;
        
        it = find (dispEvntListeners.begin(), dispEvntListeners.end(), l);
        if (it == dispEvntListeners.end())
        {
            dispEvntListeners.push_back (l);
            cout << "Added Display listener...!\n";
        }
        else
        {
            cout << "Already registered to the Display listener\n";
        }
        return ;
    }


/**
 * @fn void Host::removeDisplayConnectionListener (DisplayConnectionChangeListener *l)
 * @brief This API is used to remove listeners from the Display connection change event list.
 *
 * @param[in] DisplayConnectionChangeListener The listener to remove
 *
 * @return None
 */
    void Host::removeDisplayConnectionListener (DisplayConnectionChangeListener *l)
    {
        std::list < DisplayConnectionChangeListener* > ::iterator it ;
        it = find (dispEvntListeners.begin(), dispEvntListeners.end(), l);
        if (it == dispEvntListeners.end())
        {
            cout << "Not Registered to Display Listener yet...!\n";
        }
        else
        {
            dispEvntListeners.erase (it);
            cout << "Removed from the Display listener...!\n";
        }
        return;
    }


/**
 * @fn void Host::notifyPowerChange (const  int mode)
 * @brief This function is used to get the current power state.
 *
 * @param[in] mode Power mode of the decoder.
 * @return None.
 */
    void Host::notifyPowerChange (const  int mode)
    {
    	std::list < PowerModeChangeListener* > ::iterator it;
        for ( it = powerEvntListeners.begin() ; it != powerEvntListeners.end(); it++ )
        {
            (*it)->powerModeChanged (mode);
        }
    }

    void Host::notifyDisplayConnectionChange (int portHandle, bool newConnectionStatus)
    {
        std::list < DisplayConnectionChangeListener* > ::iterator it;
        for ( it = dispEvntListeners.begin() ; it != dispEvntListeners.end(); it++ )
        {
            (*it)->displayConnectionChanged(getVideoOutputPort(portHandle), newConnectionStatus);
            getVideoOutputPort(portHandle).setDisplayConnected(newConnectionStatus);
        }
    }


/**
 * @fn bool Host::setPowerMode(int mode)
 * @brief This API is used to change the power mode of the device.
 * This function will set the power mode to active or standby and turn off all the ouput ports.
 * The accepted modes are defined as constants in host module: Host::kPowerOn, Host::kPowerOff, Host::kPowerStandby.
 * Upon the return of this API, the power mode listeners will be notified of the new power mode.
 * This function is currently not supported.
 *
 * @param[in] mode New Power Mode.
 *
 * @return None
 */
    bool Host::setPowerMode(int mode)
    {
        throw UnsupportedOperationException();
    }


/**
 * @fn int Host::getPowerMode()
 * @brief This API is used to get the current power mode of the device.
 * This function is currently not supported.
 *
 * @return  Current power mode
 */
    int Host::getPowerMode()
    {
      throw UnsupportedOperationException();
    }


/**
 * @fn List<VideoOutputPort>  Host::getVideoOutputPorts()
 * @brief This API is used to get the list of the video output ports supported on the device.
 * A complete list of ports that are physically present on the platform will be returned, regardless the state of each port.
 * If no Video Output Port is available on the device, an empty List will be returned.
 *
 * @param  None
 *
 * @return A list of videooutputport instances
 */
    List<VideoOutputPort>  Host::getVideoOutputPorts()
    {
    	List<VideoOutputPort> rPorts;

        List<VideoOutputPortType> vTypes =  VideoOutputPortConfig::getInstance().getSupportedTypes();
        for (size_t i = 0; i < vTypes.size(); i++) {
        	List<VideoOutputPort> vPorts = vTypes.at(i).getPorts();
            for (size_t j = 0; j < vPorts.size(); j++) {
            	rPorts.push_back(vPorts.at(j));
            }
        }

    	return rPorts;
    }


/**
 * @fn List<AudioOutputPort>  Host::getAudioOutputPorts()
 * @brief This API is used to get the list of the audio output ports supported on the device.
 * A complete list of ports that are physically present on the platform will be returned, regardless the state of each port.
 * If no Audio Output Port is available on the device, an empty List will be returned.
 *
 * @return A list of audiooutputport instances
 */
    List<AudioOutputPort>  Host::getAudioOutputPorts()
    {
    	List<AudioOutputPort> rPorts;

        List<AudioOutputPortType> aTypes = AudioOutputPortConfig::getInstance().getSupportedTypes();
        for (size_t i = 0; i < aTypes.size(); i++) {
        	List<AudioOutputPort> aPorts = aTypes.at(i).getPorts();
            for (size_t j = 0; j < aPorts.size(); j++) {
            	rPorts.push_back(aPorts.at(j));
            }
        }

    	return rPorts;
    }


/**
 * @fn List<VideoDevice>  Host::getVideoDevices()
 * @brief This API is used to get the list of the video devices (i.e. Decoders) supported on the device.
 * If no Video Device is available on the device, an empty List will be returned.
 *
 * @return A list of videodevice instances
 */
    List<VideoDevice>  Host::getVideoDevices()
    {
        return VideoDeviceConfig::getInstance().getDevices();
    }


/**
 * @fn VideoOutputPort &Host::getVideoOutputPort(const std::string &name)
 * @brief This API is used to get the reference to the video output port by its name.
 * The name of the port must match one of those returned by Host::getVideoOutputPorts.
 * Application shall not develop code that depends on the actual naming conventions of the video output port.
 * Such code as Host::getVideoOutputPort("HDMI0") is discouraged as the name of the left-most
 * HDMI output port may not necessarily be "HDMI0" from all implementations.
 * IllegalArgumentException will be thrown if the "name" does not match any of the ports.
 *
 * @param[in] name Name of the port
 *
 * @return Reference to its object
 */
    VideoOutputPort &Host::getVideoOutputPort(const std::string &name)
    {
        return VideoOutputPortConfig::getInstance().getPort(name);
    }


/**
 * @fn  VideoOutputPort &Host::getVideoOutputPort(int id)
 * @brief This API is used to get a reference to the video output port by its id.
 * The name of the port must match one of those returned by Host::getVideoOutputPorts.
 * Application shall not develop code that depends on the actual naming conventions of the video output port.
 * Such code as Host::getVideoOutputPort("HDMI0") is discouraged as the name of the left-most
 * HDMI output port may not necessarily be "HDMI0" from all implementations.
 * IllegalArgumentException will be thrown if the "name" does not match any of the ports.
 *
 * @param[in] id port id
 *
 * @return Reference to its object
 */
    VideoOutputPort &Host::getVideoOutputPort(int id)
    {
        return VideoOutputPortConfig::getInstance().getPort(id);
    }


/**
 * @fn AudioOutputPort &Host::getAudioOutputPort(const std::string &name)
 * @brief This API is used to get the reference to the audio output port by its name.
 * The name of the port must match one of those returned by Host::getAudioOutputPorts.
 * Application shall not develop code that depends on the actual naming conventions of the Audio output port.
 * Such code as Host::getAudioOutputPort("SPDIF0") is discouraged as the name of the left-most
 * SPDIF output port may not necessarily be "SPDIF0" from all implementations.
 * IllegalArgumentException will be thrown if the "name" does not match any of the ports.
 *
 * @param[in] name Name of the port
 *
 * @return Reference to its object
 */
    AudioOutputPort &Host::getAudioOutputPort(const std::string &name)
    {
        return AudioOutputPortConfig::getInstance().getPort(name);
    }


/**
 * @fn  AudioOutputPort &Host::getAudioOutputPort(int id)
 * @brief This API is used to get a reference to the Audio output port by its id.
 * The name of the port must match one of those returned by Host::getAudioOutputPorts.
 * Application shall not develop code that depends on the actual naming conventions of the Audio output port.
 * Such code as Host::getAudioOutputPort("SPDIF0") is discouraged as the name of the left-most
 * SPDIF output port may not necessarily be "SPDIF0" from all implementations.
 * IllegalArgumentException will be thrown if the "name" does not match any of the ports.
 *
 * @param[in] id port id
 *
 * @return Reference to its object
 */
    AudioOutputPort &Host::getAudioOutputPort(int id)
    {
        return AudioOutputPortConfig::getInstance().getPort(id);
    }


/**
 * @fn Host::getPreferredSleepMode()
 * @brief This API is used to gets the Preferred sleep Mode and implements the low power mode for RDK platform.
 * dsGetPreferredSleepMode(&mode) function returns preferred sleep mode which is persisted.
 *
 * @param None
 *
 * @return A list of sleepmode instances
 */
    SleepMode Host::getPreferredSleepMode()
    {
        dsSleepMode_t mode;
        dsGetPreferredSleepMode(&mode);
        return SleepMode::getInstance(mode);
    }


/**
 * @fn Host::setPreferredSleepMode(const SleepMode mode)
 * @brief This API is used to set the  Preferred sleep Mode and implements the low power mode for RDK platform.
 *
 * @param[in] mode  sleep mode that is expected to be persisted
 *
 * @return ID value of the sleepmode instance
 */
    int Host::setPreferredSleepMode(const SleepMode mode)
    {
        return dsSetPreferredSleepMode((dsSleepMode_t)mode.getId());
    }


/**
 * @fn  List<SleepMode> Host::getAvailableSleepModes()
 * @brief This function is used to get the available sleep modes.
 *
 * @return A list of sleepmode instance
 */
    List<SleepMode> Host::getAvailableSleepModes()
    {
        SleepMode mode = SleepMode::getInstance(1);
        return mode.getSleepModes();
    }


/**
 * @fn Host::getCPUTemperature()
 * @brief This API gets CPU temperature.
 *
 * @return CPU temperature in default float value
 * @retval temperature Indicates CPU temerature in centigrade.
 */
    float Host::getCPUTemperature()
    {
        float temperature = 45.0;
        dsGetCPUTemperature(&temperature);
        return temperature;
    }

    /**
    * @fn Host::getVersion()
    * @brief Get DS HAL API Version
    *
    * @return Returns 4 byte Versionnumber  
    * @retval In 4 byte VersionNumber, Two Most significant Bytes are Major number 
    * and Two Least Significant Bytes are minor number.
    */
    uint32_t  Host::getVersion(void) 
    {
        uint32_t versionNumber;     
        dsGetVersion(&versionNumber);
        return versionNumber;
    }

    /**
    * @fn Host::SetVersion()
    * @brief Allows the application to set the runtime version of the dsHAL 
    *
    * @param[out] New 4 Bytes versionNumber of DS HAL
    * @return None. 
    */
    void Host::setVersion(uint32_t versionNumber)
    {
        dsSetVersion(versionNumber);
    }

/**
 * @fn  void Host::getHostEDID(std::vector<uint8_t> &edid) const
 * @brief This function is used to get the EDID information of the host device.
 *
 * @param[out] edid EDID info of the device.
 * @return None.
 */
    void Host::getHostEDID(std::vector<uint8_t> &edid) const
    {
        if (sizeof(m_hostEDID) != 0) {
            edid.insert(edid.begin(), &m_hostEDID[0], &m_hostEDID[sizeof(m_hostEDID)]);
        }
        else {
            edid.clear();
        }
    }
}



/** @} */
/** @} */
