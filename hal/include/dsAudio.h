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
 * @file dsAudio.h
 */



/**
* @defgroup devicesettings
* @{
* @defgroup hal
* @{
**/


#ifndef _DS_AUDIOOUTPORT_H_
#define _DS_AUDIOOUTPORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include "dsError.h"
#include "dsTypes.h"


/** @addtogroup DSHAL_AUDIO_API Device Settings HAL Audio Public API
 *  @ingroup devicesettingshalapi
 *
 *  Described herein are the DeviceSettings HAL types and functions that are part of the
 *  Audio subsystem. The Audio subsystem manages audio hardware operations.
 *
 *  @{
 */

/**
 * @brief Initialize the underlying Audio Port sub-system.
 * 
 * This function must initialize all the audio specific output ports. It must return
 * ::dsERR_OPERATION_NOT_SUPPORTED when no audio port is present in the device (e.g. a 
 * headless gateway device).
 *
 * @return Device Settings error code
 * @retval dsERR_NONE Indicates dsAudioPortInit API was successfully called using iarmbus call.
 * @retval dsERR_GENERAL Indicates error due to general failure.
 */
dsError_t  dsAudioPortInit();

/**
 * @brief Get the audio port handle.
 * 
 * This function returns the handle for the type of audio port requested. Must return
 * ::dsERR_OPERATION_NOT_SUPPORTED if an unavailable audio port is requested.
 *
 * @param[in] type      Indicates the type of audio port (HDMI, SPDIF and so on).
 * @param[in] index     Index of audio port (0, 1, ...).
 * @param[out] handle   Indicates pointer to hold the handle of the specified audio port.
 * @return Device Settings error code
 * @retval dsERR_NONE Indicates dsGetAudioPort API was successfully called using iarmbus call.
 * @retval dsERR_GENERAL Indicates error due to general failure.
 */
dsError_t  dsGetAudioPort(dsAudioPortType_t type, int index, int *handle);

/**
 * @brief Get the encoding type of an audio port.
 * 
 * This function returns the current audio encoding setting for the specified audio port.
 *
 * @param[in] handle Handle for the audio port.
 * @param[out] encoding Pointer to hold the encoding setting of the audio port.
 *
 * @return Device Settings error code
 * @retval dsERR_OPERATION_NOT_SUPPORTED Indicates this call is not supported.
 */
dsError_t  dsGetAudioEncoding(int handle, dsAudioEncoding_t *encoding);

/**
 * @brief Get the audio compressionof an audio port.
 * 
 * This function returns the audio compression setting used in the specified audio port.
 *
 * @param[in] handle Handle for the audio port.
 * @param[out] compression Pointer to hold the compression setting of the specified audio port.
 *
 * @return Device Settings error code
 * @retval dsERR_OPERATION_NOT_SUPPORTED Indicates this call is not supported.
 */
dsError_t  dsGetAudioCompression(int handle, dsAudioCompression_t *compression);

/**
 * @brief Get the stereo mode of an audio port.
 * 
 * This function gets the stereo mode setting for a specified audio port.
 *
 * @param[in] handle Handle for the audio port.
 * @param[out] stereoMode Pointer to hold the stereo mode setting of the
 * specified audio port.
 *
 * @return Device Settings error code
 * @retval dsERR_NONE Indicates dsGetStereoMode was successfully called using iarmbus.
 */
dsError_t  dsGetStereoMode(int handle, dsAudioStereoMode_t *stereoMode);

/**
 * @brief This function is used to get the current auto mode setting of the specified
 * audio port.
 *
 * @param[in] handle Handle for the audio port.
 * @param[out] autoMode Pointer to hold the auto mode setting of the
 * specified audio port.
 *
 * @return Device Settings error code
 * @retval dsERR_NONE Indicates dsGetStereoAuto was successfully called using iarmbus.
 */
dsError_t  dsGetStereoAuto(int handle, int *autoMode);

/**
 * @brief Get the audio gain of an audio port.
 * 
 * This function returns the current audio gain for the specified audio port.
 *
 * @param[in] handle Handle for the audio port.
 * @param[out] gain Pointer to hold the gain value of the specified audio port.
 *
 * @return Device Settings error code
 * @retval dsERR_OPERATION_NOT_SUPPORTED Indicates dsGetAudioGain call is not implemented.
 */
dsError_t  dsGetAudioGain(int handle, float *gain);

/**
 * @brief Get the current audio dB level of an audio port.
 * 
 * This function returns the current audio dB level for the specified audio port. 
 *
 * @param[in] handle Handle for the audio port.
 * @param[out] db Pointer to hold the dB value of the specified audio port.
 *
 * @return Device Settings error code
 * @retval dsERR_OPERATION_NOT_SUPPORTED Indicates dsGetAudioGain call is not implemented.
 */
dsError_t  dsGetAudioDB(int handle, float *db);

/**
 * @brief Get the current audio volume level of an audio port.
 * 
 * This function returns the current audio volume level for the specified audio port.
 *
 * @param[in] handle Handle for the audio port.
 * @param[out] level Pointer to hold the audio level value of the specified audio port.
 *
 * @return Device Settings error code
 * @retval dsERR_OPERATION_NOT_SUPPORTED Indicates dsGetAudioLevel call is not implemented.
 */
dsError_t  dsGetAudioLevel(int handle, float *level);

/**
 * @brief Get the maximum audio dB level of an audio port.
 * 
 * This function returns the maximum audio dB level supported by the specified audio port.
 *
 * @param[in] handle Handle for the audio port.
 * @param[out] maxDb Pointer to hold the maximum audio dB value supported by the specified audio port.
 *
 * @return Device Settings error code
 * @retval dsERR_NONE Indicates the call was successful.
 */
dsError_t  dsGetAudioMaxDB(int handle, float *maxDb);

/**
 * @brief Get the minimum audio dB level of an audio port.
 * 
 * This function returns the minimum audio dB level supported by the specified audio port.
 *
 * @param[in] handle Handle for the audio port.
 * @param[out] minDb Pointer to hold the minimum audio dB value supported by the specified audio port.
 *
 * @return Device Settings error code
 * @retval dsERR_NONE Indicates the call was successful.
 */
dsError_t  dsGetAudioMinDB(int handle, float *minDb);

/**
 * @brief Get the optimal audio level of an audio port.
 * 
 * This function returns the optimal audio level (dB) for the specified audio port.
 *
 * @param[in] handle Handle for the audio port.
 * @param[out] optimalLevel Pointer to hold the optimal level value of the specified audio port.
 *
 * @return Device Settings error code
 * @retval dsERR_OPERATION_NOT_SUPPORTED Indicates dsGetAudioOptimalLevel call is not implemented.
 */
dsError_t  dsGetAudioOptimalLevel(int handle, float *optimalLevel);

/**
 * @brief Get the audio delay in milliseconds
 *
 * This function will get the audio delay in milliseconds.
 *
 * @param [in] handle        Handle for the Output Audio port
 * @param [out] *audioDelayMs Audio delay in milliseconds
 * @return dsError_t Error code.
 */
dsError_t dsGetAudioDelay(int handle, uint32_t *audioDelayMs);

/**
 * @brief Get the audio delay offset in milliseconds
 *
 * This function will get the audio delay offset in milliseconds.
 *
 * @param [in] handle        Handle for the Output Audio port
 * @param [out] *audioDelayOffsetMs Audio delay offset in milliseconds
 * @return dsError_t Error code.
 */
dsError_t dsGetAudioDelayOffset(int handle, uint32_t *audioDelayOffsetMs);

/**
 * @brief Get the loop-through mode of an audio port.
 * 
 * This function is used to check if the audio port is configured for loop-through.
 *
 * @param[in] handle Handle for the audio port.
 * @param[out] loopThru Pointer to hold the loop-through setting of the specified audio.
 *
 * @return Device Settings error code
 * @retval dsERR_OPERATION_NOT_SUPPORTED Indicates dsIsAudioLoopThru call is not implemented.
 */
dsError_t  dsIsAudioLoopThru(int handle, bool *loopThru);

/**
 * @brief Get the audio mute status of an audio port.
 * 
 * This function is used to check whether the audio on a specified port is muted or not.
 *
 * @param[in] handle Handle for the audio port.
 * @param[out] muted Pointer to hold the mute setting of the specified audio port.
 *
 * @return Device Settings error code
 * @retval dsERR_OPERATION_NOT_SUPPORTED Indicates dsIsAudioMute call is not implemented.
 */
dsError_t  dsIsAudioMute(int handle, bool *muted);


/**
 * @brief This function indicates whether the specified Audio port is enabled or not.
 *
 * @param[in]  handle      Handle of the Audio port.
 * @param[out] enabled    The address of a location to hold the Audio port enable state
 *                          on return (@a true when port is enabled, @a false otherwise).
 * @return Device Settings error code
 * @retval dsERR_NONE If sucessfully dsIsAudioPortEnabled api has been called using IARM support.
 * @retval dsERR_GENERAL General failure .
 */
dsError_t  dsIsAudioPortEnabled(int handle, bool *enabled);


/**
 * @brief This function is used to enable or disable the specified Audio port.
 *
 * @param[in] handle      Handle of the Audio port.
 * @param[in] enabled     Flag to control the Audio port state
 *                         (@a true to enable, @a false to disable)
 * @return Device Settings error code
 * @retval dsERR_NONE If sucessfully dsEnableAudioPort api has been called using IARM support.
 * @retval dsERR_GENERAL General failure.
 */
dsError_t  dsEnableAudioPort(int handle, bool enabled);

/**
 * @brief This function is used to enable or disable MS12 DAPV2 and DE feature.
 *
 * @param[in] handle      Handle of the Audio port.
 * @param[in] feature     Enums for MS12 features
 * @param[in] enable     Flag to control the MS12 features
 *                         (@a true to enable, @a false to disable)
 * @return Device Settings error code
 * @retval dsERR_NONE If API executed successfully.
 * @retval dsERR_GENERAL General failure.
 */
dsError_t  dsEnableMS12Config(int handle, dsMS12FEATURE_t feature,const bool enable);

/**
 * @brief This function is used to enable or disable Loudness Equivalence feature.
 *
 * @param[in] handle      Handle of the Audio port.
 * @param[in] enable     Flag to control the LE features
 *                         (@a true to enable, @a false to disable)
 * @return Device Settings error code
 * @retval dsERR_NONE If API executed successfully.
 * @retval dsERR_GENERAL General failure.
 */
dsError_t  dsEnableLEConfig(int handle, const bool enable);

/**
 * @brief To Get LE configuration
 *
 * This function is used to Get LE features
 *
 * @param [in] handle   Handle for the Output Audio port
 * @param [in] *enable  true if LE is enabled else False
 * @return dsError_t Error code.
 */
dsError_t dsGetLEConfig(int handle, bool *enable);

/**
 * @brief Set the encoding type of an audio port.
 * 
 * This function sets the audio encoding type to be used on the specified audio port.
 *
 * @param [in] handle     Handle for the audio output port.
 * @param [in] encoding   The encoding type to be used on the audio port.
 * @return Device Settings error code
 * @retval dsERR_NONE Indicates the call was successful.
 */
dsError_t  dsSetAudioEncoding(int handle, dsAudioEncoding_t encoding);

/**
 * @brief Set the audio compression of an audio port.
 * 
 * This function sets the audio compression type to be used on the specified audio port.
 *
 * @param[in] handle Handle for the audio port.
 * @param[in] compression Indicates the compression type to be used on the audio port.
 *
 * @return dsERR_NONE Indicates the call was successful.
 */
dsError_t  dsSetAudioCompression(int handle, dsAudioCompression_t compression);

/**
 * @brief Set the stereo mode of an audio port.
 * 
 * This function sets the stereo mode to be used on the specified audio port.
 *
 * @param[in] handle Handle for the audio output port.
 * @param[in] mode Indicates the stereo mode to be used on audio port.
 *
 * @return Device Settings error code
 * @retval dsERR_NONE Indicates the dsSetStereoMode API was successfully called using iarmbus call.
 * @retval dsERR_GENERAL Indicates error due to general failure.
 */
dsError_t  dsSetStereoMode(int handle, dsAudioStereoMode_t mode);

/**
 * @brief This function sets the auto mode to be used on the specified audio port.
 *
 * @param[in] handle Handle for the audio output port.
 * @param[in] autoMode Indicates the auto mode to be used on audio port.
 *
 * @return Device Settings error code
 * @retval dsERR_NONE Indicates the dsSetStereoAuto API was successfully called using iarmbus call.
 * @retval dsERR_GENERAL Indicates error due to general failure.
 */
dsError_t  dsSetStereoAuto(int handle, int autoMode);

/**
 * @brief Set the audio gain of an audio port.
 * 
 * This function sets the gain to be used on the specified audio port.
 *
 * @param[in] handle Handle for the audio output port.
 * @param[in] gain The gain to be used on the audio port.
 *
 * @return Device Settings error code
 * @retval dsERR_NONE Indicates the call was successful.
 */
dsError_t  dsSetAudioGain(int handle, float gain);

/**
 * @brief This function sets the dB level to be used on the specified audio port.
 *
 * @param [in] handle  Handle for the audio output port.
 * @param [in] db      The dB level to be used on the audio port.
 * @return    Error Code.
 * @retval    ::dsError_t
 */
dsError_t  dsSetAudioDB(int handle, float db);

/**
 * @brief This function sets the audio volume level to be used on the specified audio port.
 *
 * @param [in] handle   Handle for the audio output port.
 * @param [in] level    The volume level to be used on the audio port.
 * @return    Error Code.
 * @retval    ::dsError_t
 */
dsError_t  dsSetAudioLevel(int handle, float level);

/**
 * @brief Set loop-through mode of an audio port.
 * 
 * This function enables/disables audio loop-through on the specified audio port.
 *
 * @param[in] handle Handle for the audio output port.
 * @param[in] loopThru Boolean flag to enable/disable loop-through.
 *
 * @return Device Settings error code
 * @retval dsERR_NONE Indicates the call was successful.
 */
dsError_t  dsEnableLoopThru(int handle, bool loopThru);

/**
 * @brief Mute or un-mute an audio port.
 * 
 * This function mutes or unmutes the specified audio output port.
 *
 * @param[in] handle Handle for the audio output port.
 * @param[in] mute Boolean flag to mute/un-mute the audio port.
 *
 * @return Device Settings error code
 * @retval dsERR_NONE Indicates dsAudioPortInit API was successfully called using iarmbus call.
 * @retval dsERR_GENERAL Indicates error due to general failure.
 */
dsError_t  dsSetAudioMute(int handle, bool mute);

/**
 * @brief This function is used to check whether the audio port supports Dolby MS11 Multistream Decode
 *
 * @param[in] handle Handle for the audio port.
 * @param[out] Address to hold the MS11 Multistream Decode setting of the specified audio port.
 *
 * @return Device Settings error code
 * @retval dsERR_OPERATION_NOT_SUPPORTED Indicates dsIsAudioMSDecode call is not implemented.
 */
dsError_t  dsIsAudioMSDecode(int handle, bool *HasMS11Decode);

/**
 * @brief This function is used to check whether the audio port supports Dolby MS12 Multistream Decode
 *
 * @param[out] Address to hold the MS12 Multistream Decode setting of the specified audio port.
 *
 * @return Device Settings error code
 * @retval dsERR_OPERATION_NOT_SUPPORTED Indicates dsIsAudioMS12Decode call is not implemented.
 */
dsError_t  dsIsAudioMS12Decode(bool *HasMS12Decode);

/**
 * @brief Set the audio delay in milliseconds
 * 
 * This function will set the audio delay in milliseconds
 *
 * @param [in] handle      Handle for the Output Audio port
 * @param [in] audioDelayMs  Amount of milliseconds of delay
 * @return dsError_t Error code.
 */
dsError_t dsSetAudioDelay(int handle, const uint32_t audioDelayMs);

/**
 * @brief Set the audio delay offset in milliseconds
 * 
 * This function will set the audio delay offset in milliseconds
 *
 * @param [in] handle      Handle for the Output Audio port
 * @param [in] audioDelayOffsetMs  Amount of milliseconds of delay offset
 * @return dsError_t Error code.
 */
dsError_t dsSetAudioDelayOffset(int handle, const uint32_t audioDelayOffsetMs);

/**
 * @brief Terminate the Audio Port sub-system.
 * 
 * This function will terminate the usage of audio output ports by resetting the data
 * structures used within this module and release the audio port specific handles.
 *
 * @return Device Settings error code
 * @retval dsERR_NONE Indicates dsAudioPortInit API was successfully called using iarmbus call.
 * @retval dsERR_GENERAL Indicates error due to general failure.
 */
dsError_t  dsAudioPortTerm();

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif /* _DS_AUDIOOUTPORT_H_ */


/** @} */
/** @} */
