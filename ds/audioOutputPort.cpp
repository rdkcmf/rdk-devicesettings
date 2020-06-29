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


#include "audioEncoding.hpp"
#include "audioCompression.hpp"
#include "audioStereoMode.hpp"
#include "audioOutputPort.hpp"
#include "audioOutputPortType.hpp"
#include "audioOutputPortConfig.hpp"
#include "videoOutputPortConfig.hpp"
#include "videoOutputPort.hpp"
#include "illegalArgumentException.hpp"
#include "list.hpp"
#include <sstream>
#include <string>
#include <list>
#include "dsAudio.h"
#include "dsError.h"
#include "dslogger.h"

/**
 * @file audioOutputPort.cpp
 * @brief AudioOutputPort objects are instantiated by the Device Settings module upon initialization.
 * Applications do not need to create any such objects on its own.
 * References to these objects can be retrieved by applications via the VideoOutputPort
 * connected to the AudioOutputPort: VideoOutputPort::getAudioOutputPort()
 */

extern dsError_t dsSetStereoMode(int handle, dsAudioStereoMode_t mode, bool isPersist);
extern dsError_t dsSetStereoAuto(int handle, int autoMode, bool isPersist);
extern dsError_t dsGetStereoMode(int handle, dsAudioStereoMode_t *stereoMode, bool isPersist);

namespace device {

const uint32_t audioDelayMsMax = 300;
const uint32_t audioDelayOffsetMsMax = 200;

/**
 * @addtogroup dssettingsaudoutportapi
 * @{
 */
/**
 * @fn AudioOutputPort::getInstance(int id)
 * @brief This API is used to get the instance of the audio output port based on the port id returned by the getsupported audiooutput port.
 *
 * @param[in] id Port id
 *
 * @return Reference to the instance of the port id
 */
AudioOutputPort & AudioOutputPort::getInstance(int id)
{
	return AudioOutputPortConfig::getInstance().getPort(id);
}


/**
 * @fn AudioOutputPort::getInstance(const std::string &name)
 * @brief This API is used to get the instance of the audio output port based on the port name returned by the getsupported audiooutput port.
 *
 * @param[in] name Name of the port
 *
 * @return Reference to the instance of the name of the port
 */
AudioOutputPort & AudioOutputPort::getInstance(const std::string &name)
{
	return AudioOutputPortConfig::getInstance().getPort(name);

}


/**
 * @fn AudioOutputPort::AudioOutputPort(const int type, const int index, const int id)
 * @brief This function is a default constructor for AudioOutputPort. It initialises the data members
 * of AudioOutputPort instance with the parameters passed. It also gets the handle for the type of
 * Audio port requested and updates the audio compression technique, type of encoding, stereo mode,
 * audio gain, audio level, optimal audio level, maximum and minimum DB that is supported and current
 * audio DB.
 *
 * @param[in] type Type of audio output port [HDMI,SPDIF]
 * @param[in] index Index of audio output port (0,1,...)
 * @param[in] id Audiooutput port id.
 *
 * @return None
 */
AudioOutputPort::AudioOutputPort(const int type, const int index, const int id) :
								 _type(type), _index(index), _id(id),
								 _handle(-1), _encoding(AudioEncoding::kNone),
								  _stereoMode(AudioStereoMode::kStereo), _stereoAuto(false),
								 _gain(0.0), _db(0.0), _maxDb(0.0), _minDb(0.0), _optimalLevel(0.0),
								 _level(0.0), _loopThru(false), _muted(false), _audioDelayMs(0), _audioDelayOffsetMs(0)
{
	dsGetAudioPort((dsAudioPortType_t)_type, _index, &_handle);
	{
		/* Construct Port Name as "Type+Index", such as "HDMI0" */
		std::stringstream out;
		out << getType().getName() << _index;
		_name = out.str();
	}

	//dsGetAudioCompression	(_handle, (dsAudioCompression_t *)&_compression);
	dsGetAudioEncoding		(_handle, (dsAudioEncoding_t *)&_encoding);
	dsGetStereoMode			(_handle, (dsAudioStereoMode_t *)&_stereoMode, false);
	dsGetAudioGain			(_handle, &_gain);
	dsGetAudioLevel			(_handle, &_level);
	dsGetAudioOptimalLevel	(_handle, &_optimalLevel);
	dsGetAudioMaxDB			(_handle, &_maxDb);
	dsGetAudioMinDB			(_handle, &_minDb);
	dsGetAudioDB				(_handle, &_db);
	dsIsAudioLoopThru		(_handle, &_loopThru);
	dsIsAudioMute			(_handle, &_muted);
        dsGetAudioDelay                 (_handle, &_audioDelayMs);
        dsGetAudioDelayOffset      (_handle, &_audioDelayOffsetMs);
}


/**
 * @fn AudioOutputPort::~AudioOutputPort()
 * @brief This is a default destructor of class AudioOutputPort.
 *
 * @return None.
 */
AudioOutputPort::~AudioOutputPort()
{

}


/**
 * @fn const AudioOutputPortType & AudioOutputPort::getType() const
 * @brief This API is used to get the type of the audio output port. The type of audio output port represent the general capabilities of the port.
 *
 * @return An instance to the type of the audio output port
 */
const AudioOutputPortType & AudioOutputPort::getType() const
{
	return AudioOutputPortConfig::getInstance().getPortType(_type);
}


/**
 * @fn const List<AudioEncoding> AudioOutputPort::getSupportedEncodings() const
 * @brief This API is used to get the list of audio encodings supported by the port.
 *
 * @return A list of audio encodings supported
 */
const List<AudioEncoding> AudioOutputPort::getSupportedEncodings() const
{
	return AudioOutputPortType::getInstance(_type).getSupportedEncodings();
}


/**
 * @fn const List<AudioCompression> AudioOutputPort::getSupportedCompressions() const
 * @brief This API is used to get the list of audio compressions supported by the port.
 *
 * @return A list of audio compressions supported
 */
const List<AudioCompression> AudioOutputPort::getSupportedCompressions() const
{
	return AudioOutputPortType::getInstance(_type).getSupportedCompressions();
}


/**
 * @fn const List<AudioStereoMode> AudioOutputPort::getSupportedStereoModes() const
 * @breif This API is used to get the list of audio stereo modes supported by the port.
 *
 * @return A list of stereo modes supported
 */
const List<AudioStereoMode> AudioOutputPort::getSupportedStereoModes() const
{
	return AudioOutputPortType::getInstance(_type).getSupportedStereoModes();
}


/**
 * @fn const AudioEncoding & AudioOutputPort::getEncoding() const
 * @brief This API is used to get the current encoding of the output port.
 *
 * @return Current audio encoding
 */
const AudioEncoding & AudioOutputPort::getEncoding() const
{
	dsGetAudioEncoding(_handle, (dsAudioEncoding_t *)&_encoding);
	return AudioEncoding::getInstance(_encoding);
}


/**
 * @fn const AudioStereoMode & AudioOutputPort::getStereoMode()
 * @brief This API is used to get the current stereo mode of the output port.
 *
 * @return Current audio stereo mode
 */
const AudioStereoMode & AudioOutputPort::getStereoMode(bool usePersist)
{
    std::cout << "AudioOutputPort::getStereoMode from " << (usePersist ? "persistence" : "effective") << std::endl;
    int _localmode = 0;
    dsGetStereoMode(_handle, (dsAudioStereoMode_t *)&_localmode, usePersist);
    _stereoMode = _localmode;
    return AudioStereoMode::getInstance(_stereoMode);
}



/**
 * @fn AudioOutputPort::enable()
 * @brief This API is used to enable the Audio output port.
 *
 * @return None
 */
void AudioOutputPort::enable()
{
	dsError_t ret = dsEnableAudioPort(_handle, true);
	if (ret != dsERR_NONE) {
		throw Exception(ret);
	}
}


/**
 * @fn VideoOutputPort::disable()
 * @brief This API is used to disable the Audio output port.
 *
 * @return None
 */
void AudioOutputPort::disable()
{
	dsError_t ret = dsEnableAudioPort(_handle, false);
	if (ret != dsERR_NONE) {
		throw Exception(ret);
	}
}


/**
 * @fn const int AudioOutputPort::getStereoAuto()
 * @brief This API is used to get the current auto mode.
 *
 * @return Current audio stereo mode
 */
bool AudioOutputPort::getStereoAuto()
{
	int _localmode = 0;
	dsGetStereoAuto	(_handle, &_localmode);
	_stereoAuto = (_localmode);
	return _stereoAuto;
}

/**
 * @fn float AudioOutputPort::getGain() const
 * @brief This API will get the current Gain for the given audio output port.
 *
 * @return Current gain value in a given Audio output port
 */
float AudioOutputPort::getGain() const
{
	return _gain;
}


/**
 * @fn float AudioOutputPort::getDB() const
 * @brief This API will get the current Decibel value for the given Audio port.
 *
 * @return Current Decibel value in a given Audio port
 */
float AudioOutputPort::getDB() const
{
	return _db;
}


/**
 * @fn float AudioOutputPort::getMaxDB() const
 * @brief This API is used to get the current Maximum decibel that Audio output port can support.
 *
 * @return Current maximum decibel for the  given audio output port
 */
float AudioOutputPort::getMaxDB() const
{
	return _maxDb;
}


/**
 * @fn float AudioOutputPort::getMinDB() const
 * @brief This API is used to get the current minimum decibel that Audio output port can support.
 *
 * @return Current minimum decibel value for the given audio output port
 */
float AudioOutputPort::getMinDB() const
{
	return _minDb;
}


/**
 * @fn float AudioOutputPort::getOptimalLevel() const
 * @brief This API is used to get the current optimal level value for audio  output port.
 *
 * @return Current optimal level for the given audio output port
 */
float AudioOutputPort::getOptimalLevel() const
{
	return _optimalLevel;
}

/**
 * @fn bool AudioOutputPort::getAudioDelay(uint32_t& audioDelayMs) const
 * @brief This API is used to get the current audio delay in milliseconds for audio  output port.
 *
 * @return true if call succeded, false otherwise
 */
bool AudioOutputPort::getAudioDelay(uint32_t& audioDelayMs) const
{
	if (dsGetAudioDelay(_handle, &audioDelayMs) != dsERR_NONE)
	{
		return false;
	}

	return true;
}

/**
 * @fn bool AudioOutputPort::getAudioDelayOffset(uint32_t& audioDelayOffsetMs) const
 * @brief This API is used to get the current audio delay offset in milliseconds for audio  output port.
 *
 * @return true if call succeded, false otherwise
 */
bool AudioOutputPort::getAudioDelayOffset(uint32_t& audioDelayOffsetMs) const
{
	if (dsGetAudioDelayOffset(_handle, &audioDelayOffsetMs) != dsERR_NONE)
	{
		return false;
	}

	return true;
}


/**
 * @fn float AudioOutputPort::getLevel() const
 * @brief This API is used to get the current audio level for the given audio output port.
 *
 * @return Current Audio Level for the given audio output port
 */
float AudioOutputPort::getLevel() const{
	return _level;
}


/**
 * @fn bool AudioOutputPort::isLoopThru() const
 * @brief This API is used to check whether the given audio port is configured for loop thro'.
 *
 * @return TRUE or FALSE
 * @retval 1 when output is loop thru
 * @retval 0 When output is not loop thru
 */
bool AudioOutputPort::isLoopThru() const {
	return _loopThru;
}


/**
 * @fn bool AudioOutputPort::isMuted() const
 * @brief This API is used to check whether the audio is muted or not
 *
 * @return True or False
 * @retval 1 when output is muted
 * @retval 0 When output is not muted
 */
bool AudioOutputPort::isMuted() const
{
	return _muted;
}


/**
 * @fn bool AudioOutputPort::isEnabled() const
 * @brief This API is used to check whether the audio is enabled or not
 *
 * @return True or False
 * @retval 1 when output is enabled
 * @retval 0 When output is disbaled
 */
bool AudioOutputPort::isEnabled() const
{
	bool enabled = false;
	
	dsError_t ret = dsIsAudioPortEnabled(_handle, &enabled);
	if (ret != dsERR_NONE) {
		throw Exception(ret);
	}
	return enabled;
}

/**
 * @fn bool AudioOutputPort::isConnected() const
 * @brief This API is used to check whether the audio output 
 * port is connected to a sink device or not.
 *
 * Optical/SPDIF port or any analog port is considered 
 * "Always connected". HDMI port depends on actual connectivity.
 *
 * @return True or False
 * @retval 1 when output is connected 
 * @retval 0 When output is not connected
 */
bool AudioOutputPort::isConnected() const
{
    if (_type == dsAUDIOPORT_TYPE_HDMI) {
        return device::VideoOutputPortConfig::getInstance().getPort("HDMI0").isDisplayConnected();
    }
    else {
        return true;
    }
}

/**
 * @fn AudioOutputPort::setEncoding(const int newEncoding)
 * @brief This API is used to set the Encoding method in a given audio port.
 *
 * If return is not equal to dsERR_NONE, it will throw the ret to IllegalArgumentException Handler and
 * it will pass the message as "No message for this exception".
 *
 * @param[in] newEncoding Type of Encoding method for the given Audio Output port.
 *
 * @return None
 */
void AudioOutputPort::setEncoding(const int newEncoding)
{
	dsError_t ret = dsERR_NONE;

#if 0
	if ( (ret = dsSetAudioEncoding(_handle, (dsAudioEncoding_t)newEncoding)) == dsERR_NONE) {
		_encoding = (int)newEncoding;
	}

	if (ret != dsERR_NONE) throw Exception(ret);
#endif
       throw Exception("Operation not Supported");
}


/**
 * @fn AudioOutputPort::setEncoding(const std::string &newEncoding)
 * @brief This API is used to set the Encoding method in a given audio port.
 *
 * @param[in] newEncoding Type of Encoding method for the given Audio Output port.
 *
 * @return None
 */
void AudioOutputPort::setEncoding(const std::string &newEncoding)
{
	setEncoding(AudioEncoding::getInstance(newEncoding).getId());
}


/**
 * @fn AudioOutputPort::setCompression(const int newCompression)
 * @brief This API is used to set the compression mode in a given audio port.
 *
 * If return is not equal to dsERR_NONE, it will throw the ret to IllegalArgumentException Handler and
 * it will pass the message as "No message for this exception" with the value of "dsERR_INVALID_PARAM" from dsError type.
 *
 * @param[in] newCompression Type of Compression mode for the given audio Output port.
 *
 * @return None
 */
void AudioOutputPort::setCompression(const int newCompression)
{
	dsError_t ret = dsERR_NONE;

	if ( (ret = dsSetAudioCompression(_handle, newCompression)) == dsERR_NONE) {
	}

	if (ret != dsERR_NONE) throw Exception(ret);
}

/**
 * @fn AudioOutputPort::setDialogEnhancement(const int level)
 * @brief This API is used to set the compression mode in a given audio port.
 *
 * If return is not equal to dsERR_NONE, it will throw the ret to IllegalArgumentException Handler and
 * it will pass the message as "No message for this exception" with the value of "dsERR_INVALID_PARAM" from dsError type.
 *
 * @param[in] newCompression Type of Compression mode for the given audio Output port.
 *
 * @return None
 */
void AudioOutputPort::setDialogEnhancement(const int level)
{
	dsError_t ret = dsERR_NONE;

	if ( (ret = dsSetDialogEnhancement(_handle, level)) == dsERR_NONE) {
	}
	else
	{
	    throw Exception(ret);
	}
}

/**
 * @fn AudioOutputPort::setDolbyVolumeMode(const bool mode)
 * @brief This API is used to set the compression mode in a given audio port.
 *
 * If return is not equal to dsERR_NONE, it will throw the ret to IllegalArgumentException Handler and
 * it will pass the message as "No message for this exception" with the value of "dsERR_INVALID_PARAM" from dsError type.
 *
 * @param[in] newCompression Type of Compression mode for the given audio Output port.
 *
 * @return None
 */
void AudioOutputPort::setDolbyVolumeMode(const bool mode)
{
	dsError_t ret = dsERR_NONE;

	if ( (ret = dsSetDolbyVolumeMode(_handle, mode)) == dsERR_NONE) {
	}
	else
	{
	    throw Exception(ret);
	}
}

/**
 * @fn AudioOutputPort::setIntelligentEqualizerMode(const int level)
 * @brief This API is used to set the compression mode in a given audio port.
 *
 * If return is not equal to dsERR_NONE, it will throw the ret to IllegalArgumentException Handler and
 * it will pass the message as "No message for this exception" with the value of "dsERR_INVALID_PARAM" from dsError type.
 *
 * @param[in] newCompression Type of Compression mode for the given audio Output port.
 *
 * @return None
 */
void AudioOutputPort::setIntelligentEqualizerMode(const int mode)
{
	dsError_t ret = dsERR_NONE;

	if ( (ret = dsSetIntelligentEqualizerMode(_handle, mode)) == dsERR_NONE) {
	}
	else
	{
	    throw Exception(ret);
	}
}

/**
 * @fn const AudioCompression & AudioOutputPort::getCompression() const
 * @brief This API is used to get the current compression of the output port.
 *
 * @return Current audio compression
 */
int  AudioOutputPort::getCompression() const
{
	dsError_t ret = dsERR_NONE;
	int _compression = 0;
	if ( (ret = dsGetAudioCompression(_handle, &_compression)) == dsERR_NONE)
	{
            return _compression;
	}
	else
	{
	    throw Exception(ret);
	}
}

/**
 * @fn const int AudioOutputPort::getDialogEnhancement()
 * @brief This API is used to get the current auto mode.
 *
 * @return Current audio stereo mode
 */
int  AudioOutputPort::getDialogEnhancement() const
{
	dsError_t ret = dsERR_NONE;
	int _enhancerLevel = 0;
	if ((ret = dsGetDialogEnhancement(_handle, &_enhancerLevel)) == dsERR_NONE)
	{
            return _enhancerLevel;
	}
	else
	{
	    throw Exception(ret);
	}
}

/**
 * @fn  bool AudioOutputPort::getDolbyVolumeMode() 
 * @brief This API is used to get the current auto mode.
 *
 * @return Current audio stereo mode
 */
bool AudioOutputPort::getDolbyVolumeMode() const
{
	dsError_t ret = dsERR_NONE;
	bool _mode = 0;
	if ( (ret = dsGetDolbyVolumeMode(_handle, &_mode)) == dsERR_NONE)
	{
            return _mode;
	}
	else
	{
	    throw Exception(ret);
	}
}

/**
 * @fn const int AudioOutputPort::getDialogEnhancement()
 * @brief This API is used to get the current auto mode.
 *
 * @return Current audio stereo mode
 */
int AudioOutputPort::getIntelligentEqualizerMode() const
{
	dsError_t ret = dsERR_NONE;
	int _mode = 0;
	if ((ret = dsGetIntelligentEqualizerMode(_handle, &_mode)) == dsERR_NONE)
	{
        return _mode;
	}
	else
	{
	    throw Exception(ret);
	}
}


/**
 * @fn AudioOutputPort::setStereoMode(const int newMode,const bool toPersist)
 * @brief This API is used to set the stereo mode to be used in a given audio port.If toPersist is true, the setting will persist after reboots.
 *
 * If return is not equal to dsERR_NONE, it will throw the ret to IllegalArgumentException Handler and
 * it will pass the message as "No message for this exception" with the value of "dsERR_INVALID_PARAM" from dsError type.
 *
 * @param[in] newMode Type of stereo mode to be used for the given audio Output port.
 *
 * @return None
 */
void AudioOutputPort::setStereoMode(const int newMode, const bool toPersist)
{
	dsError_t ret = dsERR_GENERAL;

	if (AudioOutputPortType::getInstance(_type).isModeSupported(newMode)) 
	{	
		if ( (ret = dsSetStereoMode(_handle, (dsAudioStereoMode_t)newMode,toPersist)) == dsERR_NONE) {
			_stereoMode = (int)newMode;
		}
	}
		
	if (ret != dsERR_NONE) 
		throw Exception(ret);

}

/**
 * @fn AudioOutputPort::setAudioDelay(const uint32_t audioDelayMs)
 * @brief This API is used to set audio delay in milliseconds
 *
 * @param[in] audioDelayMs Number of milliseconds to delay the audio (0 to +250)
 *
 * @return None
 */
void AudioOutputPort::setAudioDelay(const uint32_t audioDelayMs)
{
	dsError_t ret = dsERR_NONE;
	uint32_t ms = audioDelayMs;

	INFO("AudioOutputPort [%s], setting delay to [%lu] ms\n", _name.c_str(), audioDelayMs);

	if (ms > audioDelayMsMax)
	{
		ERROR("AudioOutputPort [%s], delay [%lu] ms, exceeds max [%lu]. Setting Max \n",
			_name.c_str(),
			audioDelayMs,
			audioDelayMsMax);

		ms = audioDelayMsMax;
	}

	ret = dsSetAudioDelay(_handle, ms);

	if (ret == dsERR_NONE)
	{
		_audioDelayMs = audioDelayMs;
	}
	else
	{
		throw Exception(ret);
	}
}

/**
 * @fn AudioOutputPort::setAudioDelayOffset(const uint32_t audioDelayOffsetMs)
 * @brief This API is used to set audio delay offset in milliseconds
 *
 * @param[in] audioDelayOffsetMs Number of milliseconds to offset(additional delay) of the audio delay(0 to +200)
 *
 * @return None
 */
void AudioOutputPort::setAudioDelayOffset(const uint32_t audioDelayOffsetMs)
{
	dsError_t ret = dsERR_NONE;
	uint32_t ms = audioDelayOffsetMs;

	if (ms > audioDelayOffsetMsMax)
	{
		ERROR("AudioOutputPort [%s], delay offset [%lu] ms, exceeds max [%lu]. Setting Max \n",
			_name.c_str(), audioDelayOffsetMs, audioDelayOffsetMsMax);
		ms = audioDelayOffsetMsMax;
	}

	ret = dsSetAudioDelayOffset(_handle, ms);

	if (ret == dsERR_NONE)
	{
		_audioDelayOffsetMs = audioDelayOffsetMs;
	}
	else
	{
		throw Exception(ret);
	}
}

/**
 * @fn AudioOutputPort::setAudioAtmosOutputMode(bool enable)
 * @brief
 *
 * @param[in] Enable/Disable always Atmos output mode
 *
 * @return None
 */
void AudioOutputPort::setAudioAtmosOutputMode(bool enable)
{
        dsError_t ret = dsERR_NONE;

        ret = dsSetAudioAtmosOutputMode(_handle,enable);

        if (ret != dsERR_NONE)
        {
                throw Exception(ret);
        }
}

/**
 * @fn AudioOutputPort::getSinkDeviceAtmosCapability(dsATMOSCapability_t & atmosCapability)
 * @brief
 *
 * @param[in/out] Sink device ATMOS capability
 *
 * @return None
 */
void AudioOutputPort::getSinkDeviceAtmosCapability(dsATMOSCapability_t & atmosCapability)
{
        dsError_t ret = dsERR_NONE;
        dsATMOSCapability_t capability;

        ret = dsGetSinkDeviceAtmosCapability(_handle, &capability);

        if (ret == dsERR_NONE)
        {
                atmosCapability = capability;
        }
        else
        {
                throw Exception(ret);
        }
}

/**
 * @fn AudioOutputPort::enableLEConfig(const bool enable)
 * @brief This API is used to enable Loudness Equivalence *
 * @param[in] enable true/false to control feature.
 *
 * @return None
 */
void AudioOutputPort::enableLEConfig(const bool enable)
{
    dsError_t ret = dsERR_GENERAL;

    ret = dsEnableLEConfig(_handle, enable);

    if (ret != dsERR_NONE)
        printf("enableLEConfig failed with ret:%d \n",ret);
}

/**
 * @fn AudioOutputPort::GetLEConfig()
 * @brief This API is used to check if  Loudness Equivalence is enabled or not*
 *
 * @return true if Loudness Equivalence is enabled
 */

bool AudioOutputPort::GetLEConfig()
{
    bool enable;
    dsError_t ret = dsERR_GENERAL;

    ret = dsGetLEConfig(_handle, &enable);
    if (ret != dsERR_NONE) throw Exception(ret);

    if (enable == true)
        return true;

    return false;
}

/**
 * @fn AudioOutputPort::enableMS12Config(const dsMS12FEATURE_t feature,const bool enable)
 * @brief This API is used to enable MS12 features such as DAPV2 adn DE *
 * @param[in] feature enums for feature name.
 * @param[in] enable true/false to control feature.
 *
 * @return None
 */
void AudioOutputPort::enableMS12Config(const dsMS12FEATURE_t feature,const bool enable)
{
    dsError_t ret = dsERR_GENERAL;

    ret = dsEnableMS12Config(_handle, feature, enable);

    if (ret != dsERR_NONE)
        printf("enableMS12Config failed with ret:%d \n",ret);
}

/**
 * @fn AudioOutputPort::setStereoAuto(const bool autoMode)
 * @brief This API is used to set the stereo mode to be auto; 
 * 
 * When in auto mode, the platform tries to set the audio mode
 * to the best available.  Use getStereoMode() to get the effecitve
 * mode set by platform.
 *
 * If return is not equal to dsERR_NONE, it will throw the ret to IllegalArgumentException Handler and
 * it will pass the message as "No message for this exception" with the value of "dsERR_INVALID_PARAM" from dsError type.
 *
 * @param[in] newMode Type of stereo mode to be used for the given audio Output port.
 *
 * @return None
 */
void AudioOutputPort::setStereoAuto(const bool autoMode, const bool toPersist)
{
	dsError_t ret = dsERR_NONE;

	if ( (ret = dsSetStereoAuto(_handle, autoMode ? 1 : 0, toPersist)) == dsERR_NONE) {
		_stereoAuto = (autoMode);
	}

	if (ret != dsERR_NONE) throw Exception(ret);

}
/**
 * @fn void AudioOutputPort::setStereoMode(const std::string &newMode,const bool toPersist)
 * @brief This function is used to set the stereo mode for the audio port.If toPersist is true, the setting will persist after reboots.
 *
 * @param[in] newMode Type of stereo mode to be used for the given audio Output port.
 *
 * @return None
 */
void AudioOutputPort::setStereoMode(const std::string &newMode,const bool toPersist)
{
	setStereoMode(AudioStereoMode::getInstance(newMode).getId(),toPersist);
}


/**
 * @fn void AudioOutputPort::setDB(const float newDb)
 * @brief This API is used to  set the audio DB value to be used in a given audio port
 *
 * If return is not equal to dsERR_NONE, it will throw the ret to IllegalArgumentException Handler and
 * it will pass the message as "No message for this exception" with the value of "dsERR_INVALID_PARAM" from dsError type.
 *
 * @param[in] newDb Decibel value to be used for the given output audio port
 *
 * @return None
 */
void AudioOutputPort::setDB(const float newDb)
{
	dsError_t ret = dsERR_NONE;

        if ((newDb < _minDb) || (newDb > _maxDb))
          ret = dsERR_INVALID_PARAM;

	else if (dsSetAudioDB(_handle, newDb) == dsERR_NONE)
	{
		_db = newDb;
	}

	if (ret != dsERR_NONE) throw Exception(ret);

}


/**
 * @fn void AudioOutputPort::setLevel(const float newLevel)
 * @brief This API is used to set the audio level to be used in a given audio port.
 *
 * If return is not equal to dsERR_NONE, it will throw the ret to IllegalArgumentException Handler and
 * it will pass the message as "No message for this exception" with the value of "dsERR_INVALID_PARAM" from dsError type.
 *
 * @param[in] newLevel New Audio level for a given audio output port
 *
 * @return None
 */
void AudioOutputPort::setLevel(const float newLevel)
{
	dsError_t ret = dsERR_NONE;

	if (newLevel < 0) {
		ret = dsERR_INVALID_PARAM;
    } else if ( (ret = dsSetAudioLevel(_handle, newLevel)) == dsERR_NONE) {
		_level = newLevel;
	}

	if (ret != dsERR_NONE) throw Exception(ret);

}

/**
 * @fn void AudioOutputPort::setAudioDuckingLevel(const float newLevel)
 * @brief This API is used to set the audio level to be used in a given audio port. If output mode is Passthrough/Expert this mutes the audio
 *
 * If return is not equal to dsERR_NONE, it will throw the ret to IllegalArgumentException Handler and
 * it will pass the message as "No message for this exception" with the value of "dsERR_INVALID_PARAM" from dsError type.
 *
 * @param[in] newLevel New Audio level for a given audio output port
 *
 * @return None
 */
void AudioOutputPort::setAudioDuckingLevel(const float newLevel)
{
    dsError_t ret = dsERR_NONE;

    if (newLevel < 0) {
        ret = dsERR_INVALID_PARAM;
    }
    else if ( (ret = dsSetAudioDuckingLevel(_handle, newLevel)) == dsERR_NONE)
    {
        _level = newLevel;
    }

    if (ret != dsERR_NONE) throw Exception(ret);
}


/**
 * @fn void AudioOutputPort::setLoopThru(const bool loopThru)
 * @brief This API is used to set the audio port to do loop thro.
 *
 * @param[in] loopThru True when output is loopthru. Otherwise False.
 *
 * @return None
 */
void AudioOutputPort::setLoopThru(const bool loopThru)
{
	dsError_t ret = dsERR_NONE;

	if  ( (ret = dsEnableLoopThru(_handle, loopThru)) == dsERR_NONE) {
		_loopThru = loopThru;
	}
}


/**
 * @fn void AudioOutputPort::setMuted(const bool mute)
 * @brief This API is used to mute/unmute the audio. It throws an IllegalArgumentException
 * if audio could not be muted/unmuted.
 *
 * @param[in] mute True if audio to be muted, false otherwise.
 *
 * @return None
 */
void AudioOutputPort::setMuted(const bool mute)
{
	dsError_t ret = dsERR_NONE;

	if ( (ret = dsSetAudioMute(_handle, mute)) == dsERR_NONE) {
		_muted = mute;
	}
	if (ret != dsERR_NONE) throw IllegalArgumentException();

}

/**
 * @fn bool AudioOutputPort::isAudioMSDecode() const
 * @brief This API is used to check whether the audio port supports Dolby MS11 Multistream Decode
 *
 * @return True or False
 * @retval True when Audio ports could be configured to support Mix PCM Audio with Surround
 * @retval Fals when Audio ports could not be configured to support Mix PCM Audio with Surround 
 */

bool AudioOutputPort::isAudioMSDecode() const
{
	bool HasMS11Decode = false;
	
	dsError_t ret = dsIsAudioMSDecode(_handle,&HasMS11Decode);
	if (ret != dsERR_NONE) {
		throw Exception(ret);
	}
	return HasMS11Decode;
}

/**
 * @fn bool AudioOutputPort::isAudioMSi12Decode() const
 * @brief This API is used to check whether the audio port supports Dolby MS12 Multistream Decode
 *
 * @return True or False
 * @retval True when Audio ports could be configured to support Mix PCM Audio with Surround
 * @retval Fals when Audio ports could not be configured to support Mix PCM Audio with Surround
 */

bool AudioOutputPort::isAudioMS12Decode() const
{
        bool HasMS12Decode = false;

        dsError_t ret = dsIsAudioMS12Decode(_handle,&HasMS12Decode);
        if (ret != dsERR_NONE) {
                throw Exception(ret);
        }
        return HasMS12Decode;
}

}

/** @} */

/** @} */
/** @} */
