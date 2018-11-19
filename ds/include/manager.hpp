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
 * @defgroup devicesettings Device Settings Module
 * @defgroup devicesettingsclass Device Settings Classes
 * @ingroup devicesettings
 */


/**
 * @defgroup devicesettingsapi Device Settings API list
 *
 * <b> Following public APIs are available in class Manager </b>
 * <ol>
 * <li> device::Manager::Initialize() <br>API to initialize the device settings module.
 * <li> device::Manager::DeInitialize() <br>API to deinitialize the device settings module.
 * </ol>
 *
 * <b> Following public APIs are available in class Host </b>
 * <ol>
 * <li> device::Host::setPowerMode(int mode) <br>API to Change the power mode of the device.
 * <li> device::Host::getPowerMode() <br>API to get the current power mode of the device.
 * <li> device::Host::getPreferredSleepMode() <br>This API is used to gets the Preferred sleep Mode.
 * <li> device::Host::setPreferredSleepMode(const SleepMode mode) <br>API is used to set the  Preferred sleep Mode.
 * <li> device::Host::addPowerModeListener(PowerModeChangeListener *l) <br>API to register listeners for Power Mode change event.
 * <li> device::Host::removePowerModeChangeListener(PowerModeChangeListener *l) <br>API to remove listeners for Power Mode change event.
 * <li> device::Host::addDisplayConnectionListener (DisplayConnectionChangeListener *l) <br>API to register listeners for Display connection change event.
 * <li> device::Host::removeDisplayConnectionListener (DisplayConnectionChangeListener *l) <br>API to remove listeners for Display connection change event.
 * <li> device::Host::getInstance() <br>API to get a reference of the Host module.
 * <li> device::Host::getVideoOutputPorts() <br>API to get a Device:::List of the Video Output Ports supported on this device.
 * <li> device::Host::getAudioOutputPorts() <br>API to get a Device:::List of the Audio Output Ports supported on this device.
 * <li> device::Host::getVideoDevices() <br>API to get a Device:::List of the Video Devices (i.e. Decoders) supported on this device.
 * <li> device::Host::getVideoOutputPort(const std::string &name) <br>API to get a reference to the Video Output Port by its name.
 * <li> device::Host::getVideoOutputPort(int id) <br>API is used to get a reference to the Video Output Port by its id.
 * <li> device::Host::getCPUTemperature() <br>API gets CPU temperature.
 * </ol>
 *
 * <b> Following public APIs are available in class VideoDevice </b>
 * <ol>
 * <li> device::VideoDevice::setDFC(const std::string & dfc) <br>API to set the zoom setting by its name or Id.
 * <li> device::VideoDevice::setDFC(int dfc) <br>API to set the zoom settings by ID.
 * <li> device::VideoDevice::setPlatformDFC() <br>API to set the zoom setting to the default one supported by the platform.
 * <li> device::VideoDevice::getDFC() <br>API to get the current zoom setting.
 * <li> device::VideoDevice::getSupportedDFCs() <br>API to get a list of supported DFC (i.e. Zoom Settings) of this decoder.
 * </ol>
 *
 * <b> Following Public APIs are available in class VideoOutputPort </b>
 * <ol>
 * <li> device::VideoOutputPort::getInstance(int id) <br>API to get the instance of the video output port based on the port id.
 * <li> device::VideoOutputPort::getInstance(const std::string &name) <br>API to get the instance of the video output port based on the port name.
 * <li> device::VideoOutputPort::getType() <br>API to get the type of this output port.
 * <li> device::VideoOutputPort::getName() <br>API to get the name of the VideoOutputPort.
 * <li> device::VideoOutputPort::getAudioOutputPort() <br>API to get the audio output port connected to this video output port.
 * <li> device::VideoOutputPort::getResolution() <br>API to get the current Resolution output from this port.
 * <li> device::VideoOutputPort::setResolution(const std::string &resolutionName) <br>API to set the resolution of the port by its Name.
 * <li> device::VideoOutputPort::getDefaultResolution() <br>API to get the default resolution supported by the port.
 * <li> device::VideoOutputPort::getDisplay() <br>API to get the display device currently connected to the output port.
 * <li> device::VideoOutputPort::isDisplayConnected() <br>API  to Check if the port is currently connected to any display device.
 * <li> device::VideoOutputPort::isContentProtected() <br>API to Check if the port or the content output on the port has DTCP or HDCP in use.
 * <li> device::VideoOutputPort::isEnabled() const <br>API to check whether this Video output port is enabled or not.
 * <li> device::VideoOutputPort::isDynamicResolutionSupported() const <br>API to check whether the video output port supports the dynamic resolution or not.
 * <li> device::VideoOutputPort::setResolution(const std::string &resolutionName) <br>API to set the output resolution of the port by ID or its Name.
 * <li> device::VideoOutputPort::setDisplayConnected(const bool connected) <br>API to set the video output port display to be connected.
 * <li> device::VideoOutputPort::enable() <br>API  to enable the video output port.
 * <li> device::VideoOutputPort::disable() <br>API to disable the video output port.
 * </ol>
 *
 * <b> Following Public APIs are available in class VideoResolution </b>
 * <ol>
 * <li> device::VideoResolution::getPixelResolution() <br>API is used to get the pixel resolution of the given video output port.
 * <li> device::VideoResolution::getAspectRatio() <br>API is used to get the current Aspect Ratio setting of the Display Device.
 * <li> device::VideoResolution::getStereoscopicMode() <br>API is used to get the stereoscopic mode of the given video output port.
 * <li> device::VideoResolution::getFrameRate() <br>API is used to get the frame rate of the given video output port.
 * <li> device::VideoResolution::isInterlaced() const <br>API to check the video is interlaced or not.
 * <li> device::VideoResolution::isEnabled() const <br>API to check the video resolution is enabled or not.
 * </ol>
 *
 * <b> Following public APIs are available in class VideoOutputportType </b>
 * <ol>
 * <li> device::VideoOutputPortType::getInstance(int id) <br>API to get the instance of the video output port type based on the port id.
 * <li> device::VideoOutputPortType::isDTCPSupported() const <br>API to query if DTCP is supported by this port type.
 * <li> device::VideoOutputPortType::isHDCPSupported() <br>API to query if HDCP is supported by this port type.
 * <li> device::VideoOutputPortType::getSupportedResolutions() <br>API to get a list of supported Video Resolutions by this port type.
 * </ol>
 *
 * <b> Following public APIs are available in class audioOutputPort </b>
 * <ol>
 * <li> device::AudioOutputPort::getInstance(int id) <br> API is used to get the instance of the audio output port
 * <li> device::AudioOutputPort::getInstance(const std::string &name) <br>This API is used to get the instance of the audio output port.
 * <li> device::AudioOutputPort::getType() <br>API to get the type of this output port.
 * <li> device::AudioOutputPort::getName() <br>API to get the name of the AudioOutputPort.
 * <li> device::AudioOutputPort::getSupportedEncodings() <br>API to get a list of audio encodings supported by this port.
 * <li> device::AudioOutputPort::getSupportedCompressions() <br>API to get a list of audio compressions supported by this port.
 * <li> device::AudioOutputPort::getSupportedStereoModes() <br>API to get a list of audio stereo modes supported by this port.
 * <li> device::AudioOutputPort::getEncoding() <br>API to get the current encoding of the output port.
 * <li> device::AudioOutputPort::getCompression() <br>API to get the current compression of the output port.
 * <li> device::AudioOutputPort::getStereoMode() <br>API to get the current stereo mode of the output port.
 * <li> device::AudioOutputPort::getGain() <br>API to get the current Gain in a given  Audio output port.
 * <li> device::AudioOutputPort::getDB() <br>API to get the current Decibel value in a given Audio port.
 * <li> device::AudioOutputPort::getLevel() <br>API to get the current audio level in a given audio output port.
 * <li> device::AudioOutputPort::getMaxDB() <br>API to get the current Maximum decibel that Audio output port can support.
 * <li> device::AudioOutputPort::getMinDB() <br>API to get the current minimum decibel that Audio output port can support.
 * <li> device::AudioOutputPort::getOptimalLevel() <br>API to get the current optimal level value for audio  output port.
 * <li> device::AudioOutputPort::isLoopThru() <br>API to check whether the given audio port is configured for loop thro.
 * <li> device::AudioOutputPort::isMuted() <br>API to check whether the audio is muted or not.
 * <li> device::AudioOutputPort::setCompression(const int newCompression) <br>API is used to set the compression mode in a given audio port.
 * <li> device::AudioOutputPort::setStereoMode(const int newMode) <br>API is used to set the stereo mode to be used in a given audio port.
 * <li> device::AudioOutputPort::setEncoding(const std::string &newEncoding) <br>This API is used to set the Encoding method in a given audio port.
 * <li> device::AudioOutputPort::setCompression(const std::string &newCompression) <br>This API is used to set the compression mode in a given audio port.
 * <li> device::AudioOutputPort::setDB(const float newDb) <br>API to set the audio DB value to be used in a given audio port.
 * <li> device::AudioOutputPort::setLevel(const float newLevel) <br>API to set the audio level to be used in a given audio port.
 * <li> device::AudioOutputPort::setLoopThru(const bool loopThru) <br>API to set the audio port to do loop thro.
 * <li> device::AudioOutputPort::setMuted(const bool mute) <br>API to set the audio to mute.
 * </ol>
 *
 * <b> Following public APIs are available in class audioOutputPorttypes </b>
 * <ol>
 * <li> device::AudioOutputPortType::getSupportedEncodings() const <br>API to get the list of audio encodings supported by this port.
 * <li> device::AudioOutputPortType::getSupportedCompressions() const <br>API to get the list of audio compressions supported by this port.
 * <li> device::AudioOutputPortType::getSupportedStereoModes() const <br>API to get the list of audio stereo modes supported by this port.
 * </ol>
 *
 * <b> Following public APIs are available in class frontPanelIndicator </b>
 * <ol>
 * <li> device::FrontPanelIndicator::getInstance(const std::string &name) <br> API gets the FrontPanelIndicator instance corresponding to the name parameter.
 * <li> device::FrontPanelIndicator::setState(const bool &enable) <br> This API is used to enable or disable the front panel indicator
 * <li> device::FrontPanelIndicator::getBrightness() <br>API to set the brightness of the FrontPanel Indicators.
 * <li> device::FrontPanelIndicator::Blink::Blink(int interval, int iteration) <br>API is a parameterized constructor for the nested class Blink.
 * <li> device::FrontPanelIndicator::getBlink() <br>API to get the blink parameters of the FrontPanel display.
 * <li> device::FrontPanelIndicator::getColor() <br>API to get the color of the FrontPanel indicator/LED.
 * <li> device::FrontPanelIndicator::getMaxCycleRate() <br>API to get the rate at which the LED is rotating/glowing during scrolling.
 * <li> device::FrontPanelIndicator::setBrightness(const int &brightness,bool toPersist) <br>API to set the brightness of the FrontPanel Indicators.
 * <li> device::FrontPanelIndicator::setBlink(const Blink &blink) <br>API to set the blink iteration and blink interval for the LED.
 * <li> device::FrontPanelIndicator::setColor(const FrontPanelIndicator::Color & color,bool toPersist) <br>API to set the color of the Front Panel Indicator.
 * <li> device::FrontPanelIndicator::setColor(uint32_t color,bool toPersist) <br>API to set the color of the Front Panel Indicator.
 * <li> device::FrontPanelIndicator::getSupportedColors() <br>API gets the list of supported colors for front panel indicator.
 * </ol>
 *
 * <b> Following public APIs are available in class frontPanelTextDisplay </b>
 * <ol>
 * <li> device::FrontPanelTextDisplay::getInstance(const std::string &name) <br>API gets the FrontPanelTextDisplay instance corresponding to the name parameter.
 * <li> device::FrontPanelTextDisplay::getScroll() <br>API to get the scroll parameters for FrontPanel display.
 * <li> device::FrontPanelTextDisplay::getCurrentTimeFormat () <br>API to get the currently set timeformat for FrontPanel display.
 * <li> device::FrontPanelTextDisplay::setScroll(const Scroll & scroll) <br>API to set the scroll parameters for FrontPanel text display.
 * <li> device::FrontPanelTextDisplay::setText(const std::string &text) <br>API to set the front panel display text.
 * <li> device::FrontPanelTextDisplay::setTimeFormat(const int iTimeFormat) <br>API to set the TimeFormat of the front panel display.
 * <li> device::FrontPanelTextDisplay::setTime(const int uiHours, const int uiMinutes) <br>API to set the time of the front panel display.
 * </ol>
 *
 * <b> Following public APIs are available in class frontPanelConfig </b>
 * <ol>
 * <li> device::FrontPanelConfig::getInstance() <br>API to get the instance of the FrontPanelConfig.
 * <li> device::FrontPanelConfig::getColors() <br>API to get the list of colors supported by FrontPanel Indicators.
 * <li> device::FrontPanelConfig::getIndicators () <br>API to get the list of indicators on the front panel.
 * <li> device::FrontPanelConfig::getTextDisplays () <br>API to get the list of text display supported by the front panels.
 * </ol>
 *
 * @ingroup devicesettings
 */




/**
* @defgroup devicesettings
* @{
* @defgroup ds
* @{
**/


#ifndef _DS_MANAGER_HPP_
#define _DS_MANAGER_HPP_


/**
 * @file manager.hpp
 * @brief It contains class referenced by manager.cpp file
 */
namespace device {


/**
 * @class Manager
 * @brief Class to implement the manager interface.
 * @ingroup devicesettingsclass
 */
class Manager {
	Manager();
	virtual ~Manager();
public:
	static void Initialize();
	static void DeInitialize();
	static bool IsInitialized;   //!< Indicates the application has initialized with devicettings modules.
};

}

#endif /* _DS_MANAGER_HPP_ */


/** @} */
/** @} */
