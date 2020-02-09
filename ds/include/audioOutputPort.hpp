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


#ifndef _DS_AUDIOOUTPUTPORT_HPP_
#define _DS_AUDIOOUTPUTPORT_HPP_

#include "audioEncoding.hpp"
#include "audioCompression.hpp"
#include "audioStereoMode.hpp"
#include "list.hpp"
#include "enumerable.hpp"
#include "dsTypes.h"

#include <string>
#include <stdint.h>


/**
 * @file audioOutputPort.hpp
 * @brief It contain variables,stuctures,class and functions referenced by audiooutputport code.
 */
namespace device {

class AudioOutputPortType;
class AudioOutputPortConfig;


/**
 * @class  AudioOutputPort
 * @brief Class extending Enumerable to implement the audiooutputport interface.
 * @ingroup devicesettingsclass
 */
class AudioOutputPort  : public Enumerable {
	int _type; //!< Audiooutputporttype.
	int _index; //!< Index of the audio output port.
	int _id; //!< Port id.

	int _handle;
	std::string _name;

	int _encoding; //!< Audio Encoding Types.
	int _stereoMode; //!< Audio stereo mode Types.
	uint32_t _audioDelayMs; //!< Audio delays
	uint32_t _audioDelayOffsetMs; //!< Audio delay offset
	bool _stereoAuto; //!< Audio stereo mode Types.

	float _gain; //!< Audio gain value.
	float _db; //!< Audio decibel value.
	float _maxDb; //!< Maximum audio decibel value.
	float _minDb; //!< Minimum audio decibel value.
	float _optimalLevel; //!< Optimal audio level.
	float _level; //!< Audio level.

	bool  _loopThru; //!< Method variable to check whether audio is loop thru or not.
	bool  _muted; //!< Method variable to check whether audio is muted or not.


public:

	static AudioOutputPort & getInstance(int id);
	static AudioOutputPort & getInstance(const std::string &name);

	AudioOutputPort(const int type, const int index, const int id);
	virtual ~AudioOutputPort();

	const AudioOutputPortType & getType() const;
	int getId() const {return _id;};
	int getIndex() const {return _index; };


/**
 * @fn AudioOutputPort::getName() const
 * @brief This API gets the name of the AudioOutputPort.  The AudioOutputPort names can be
 * IDLR, HDMI and SPDIF which are created and used by the implementation to uniquely identify them.
 *
 * @return _name Indicates the name of the AudioOutputPort
 */
	const std::string &getName() const {return _name;};
	const List<AudioEncoding> getSupportedEncodings() const;
	const List<AudioCompression> getSupportedCompressions() const;
	const List<AudioStereoMode> getSupportedStereoModes() const;


	const AudioEncoding &getEncoding() const;
	int getCompression() const;
	int getDialogEnhancement() const;
	bool getDolbyVolumeMode() const;
	int getIntelligentEqualizerMode() const;
	const AudioStereoMode &getStereoMode(bool usePersist = false);
	
	void enable();
	void disable();

	bool getStereoAuto();

	float getGain() const;
	float getDB() const;
	float getLevel() const;
	float getMaxDB() const;
	float getMinDB() const;
	float getOptimalLevel() const;
	bool getAudioDelay(uint32_t& audioDelayMs) const;
	bool getAudioDelayOffset(uint32_t& audioDelayOffsetMs) const;
	bool isLoopThru() const;
	bool isMuted() const;
	bool isConnected() const;
	bool isEnabled() const;

	bool isAudioMSDecode() const;
	bool isAudioMS12Decode() const;
	
	void setEncoding(const int encoding);
	void setCompression(const int compression);
	void setDialogEnhancement(const int level);
	void setDolbyVolumeMode(const bool mode);
	void setIntelligentEqualizerMode(const int mode);

	void setStereoMode(const int mode, const bool toPersist = true);
	void setStereoAuto(const bool autoMode, const bool toPersist = true);

	void setEncoding(const std::string & encoding);
	void setStereoMode(const std::string & mode,const bool toPersist = true);
	void enableMS12Config(const dsMS12FEATURE_t feature,const bool enable);
	void enableLEConfig(const bool enable);
        bool GetLEConfig();
        void setAudioDelay(const uint32_t audioDelayMs);
        void setAudioDelayOffset(const uint32_t audioDelayOffsetMs);
        void getSinkDeviceAtmosCapability(dsATMOSCapability_t & atmosCapability);
        void setAudioAtmosOutputMode(bool enable);  

	void setDB(const float db);
	void setLevel(const float level);
	void setLoopThru(const bool loopThru);
	void setMuted(const bool mute);

};

}

#endif /* _DS_AUDIOOUTPUTPORT_HPP_ */


/** @} */
/** @} */
