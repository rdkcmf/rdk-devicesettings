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
* @defgroup rpc
* @{
**/


#include "dsAudio.h"

#include <sys/types.h>
#include <stdint.h>
#include "dsError.h"
#include "dsUtl.h"
#include "dsRpc.h"
#include "dsMgr.h"
#include "iarmUtil.h"
#include "libIARM.h"
#include "libIBus.h"
#include "dsTypes.h"
#include "dsclientlogger.h"
#include <string.h> 

#include "safec_lib.h"

dsError_t dsAudioPortInit()
{
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
	
	printf("<<<<< AOP is initialized in Multi-App Mode >>>>>>>>\r\n");

   	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsAudioPortInit,
                            NULL,
                            0);
  
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		return dsERR_NONE;
	}

	return dsERR_GENERAL;
}

dsError_t dsGetAudioPort(dsAudioPortType_t type, int index, int *handle)
{
     dsAudioGetHandleParam_t param;
	  IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
	
	_DEBUG_ENTER();
    _RETURN_IF_ERROR(dsAudioType_isValid(type), dsERR_INVALID_PARAM);
    _RETURN_IF_ERROR((handle) != NULL, dsERR_INVALID_PARAM);

	param.type = type;
    param.index = index;
    param.handle = NULL;

    rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsGetAudioPort,
                            (void *)&param,
                            sizeof(param));
	

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		*handle = param.handle;
		return dsERR_NONE;
	}
 
   return dsERR_GENERAL ;
}


dsError_t dsGetAudioEncoding(int handle, dsAudioEncoding_t *encoding)
{
	dsError_t ret = dsERR_NONE;
        dsAudioGetEncodingModeParam_t param;

	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;


	param.handle = handle;
	param.encoding = dsAUDIO_ENC_PCM; /* Default to stereo */

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
			(char *)IARM_BUS_DSMGR_API_dsGetEncoding,
			(void *)&param,
			sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		*encoding = param.encoding;
		return dsERR_NONE;
	}

	return dsERR_GENERAL;
}


dsError_t dsGetAudioFormat(int handle, dsAudioFormat_t *audioFormat)
{
    dsError_t ret = dsERR_GENERAL;
    _DEBUG_ENTER();

    dsAudioFormatParam_t param;
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

    param.handle = handle;
    param.audioFormat = dsAUDIO_FORMAT_NONE;

    rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                        (char *)IARM_BUS_DSMGR_API_dsGetAudioFormat,
                        (void *)&param,
                        sizeof(param));

    if (IARM_RESULT_SUCCESS == rpcRet)
    {
            *audioFormat = param.audioFormat;
            ret = dsERR_NONE;
    }

    return ret;
}


dsError_t dsGetStereoMode(int handle, dsAudioStereoMode_t *stereoMode, bool isPersist);
dsError_t dsGetStereoMode(int handle, dsAudioStereoMode_t *stereoMode, bool isPersist)
{
	dsError_t ret = dsERR_NONE;
	dsAudioSetStereoModeParam_t param;

	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

    param.handle = handle;
    param.mode = dsAUDIO_STEREO_STEREO; /* Default to stereo */
    param.toPersist = isPersist;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsGetStereoMode,
                            (void *)&param,
                            sizeof(param));
	
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		*stereoMode = param.mode;
		return dsERR_NONE;
	}

	return dsERR_NONE;
}

dsError_t dsGetStereoAuto(int handle, int *autoMode)
{
	dsError_t ret = dsERR_NONE;
	dsAudioSetStereoAutoParam_t param;

	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

    param.handle = handle;
    param.autoMode = 1; /* Default to auto */

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsGetStereoAuto,
                            (void *)&param,
                            sizeof(param));
	
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		*autoMode = param.autoMode;
		return dsERR_NONE;
	}

	return dsERR_NONE;
}

dsError_t dsGetAudioGain(int handle, float *gain)
{
    dsError_t ret = dsERR_GENERAL;
    _DEBUG_ENTER();

    dsAudioGainParam_t param;
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

    param.handle = handle;
    param.gain = 0;

    rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                        (char *)IARM_BUS_DSMGR_API_dsGetAudioGain,
                        (void *)&param,
                        sizeof(param));

    if (IARM_RESULT_SUCCESS == rpcRet)
    {
            *gain = param.gain;
            ret = dsERR_NONE;
    }

    return ret;
}

dsError_t dsGetAudioDB(int handle, float *db)
{
	dsError_t ret = dsERR_NONE;
    ret = dsERR_OPERATION_NOT_SUPPORTED;
	return ret;
}

dsError_t dsGetAudioLevel(int handle, float *level)
{
    dsError_t ret = dsERR_GENERAL;
    _DEBUG_ENTER();

    dsAudioSetLevelParam_t param;
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

    param.handle = handle;
    param.level = 0;

    rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                        (char *)IARM_BUS_DSMGR_API_dsGetAudioLevel,
                        (void *)&param,
                        sizeof(param));

    if (IARM_RESULT_SUCCESS == rpcRet)
    {
            *level = param.level;
            ret = dsERR_NONE;
    }

    return ret;

}

dsError_t dsGetAudioMaxDB(int handle, float *maxDb)
{
	dsError_t ret = dsERR_NONE;
	*maxDb = 180.0;
	return ret;
}

dsError_t dsGetAudioMinDB(int handle, float *minDb)
{
	dsError_t ret = dsERR_NONE;
	*minDb = -1450;
	return ret;
}

dsError_t dsGetAudioOptimalLevel(int handle, float *optimalLevel)
{
	dsError_t ret = dsERR_NONE;
    ret = dsERR_OPERATION_NOT_SUPPORTED;
	return ret;
}

dsError_t  dsIsAudioLoopThru(int handle, bool *loopThru)
{
	dsError_t ret = dsERR_NONE;
    ret = dsERR_OPERATION_NOT_SUPPORTED;
	return ret;

}

dsError_t dsIsAudioMute(int handle, bool *muted)
{
        dsAudioSetMutedParam_t param;
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

        param.handle = handle;
        param.mute = false;
        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsIsAudioMute,
                            (void *)&param,
                            sizeof(param));
        if (IARM_RESULT_SUCCESS == rpcRet)
        {
                *muted = param.mute;
                return dsERR_NONE;
        }
        return dsERR_GENERAL ;
}

dsError_t dsIsAudioPortEnabled(int handle, bool *enabled)
{
	dsAudioPortEnabledParam_t param;
	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	param.handle = handle;
	param.enabled = false;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsIsAudioPortEnabled,
                            (void *)&param,
                            sizeof(param));
	
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		*enabled = param.enabled;
		return dsERR_NONE;
	}

	return dsERR_NONE;
}

dsError_t  dsEnableAudioPort(int handle, bool enabled, const char* portName)
{
    _DEBUG_ENTER();

    dsAudioPortEnabledParam_t param;
    param.handle = handle;
    param.enabled = enabled;
    memset(param.portName, '\0', sizeof(param.portName));
    errno_t rc = -1;
    rc = strcpy_s (param.portName,sizeof(param.portName), portName);
    if(rc!=EOK)
    {
            ERR_CHK(rc);
    }
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
							(char *)IARM_BUS_DSMGR_API_dsEnableAudioPort,
							(void *)&param,
							sizeof(param));
  
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		return dsERR_NONE;
	}

	return dsERR_GENERAL ;
}

dsError_t dsGetEnablePersist(int handle, const char* portName, bool *enabled)
{
	dsAudioPortEnabledParam_t param;
	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	param.handle = handle;
    /*By default all port values are true*/
	param.enabled = true;
    memset(param.portName, '\0', sizeof(param.portName));
    errno_t rc = -1;
    rc = strcpy_s (param.portName,sizeof(param.portName), portName);
    if(rc!=EOK)
    {
            ERR_CHK(rc);
    }
	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsGetEnablePersist,
                            (void *)&param,
                            sizeof(param));
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		*enabled = param.enabled;
		return dsERR_NONE;
	}
    printf ("dsGetEnablePersist cli portName:%s rpcRet:%d param.enabled:%d enabled:%d\n", 
            portName, rpcRet, param.enabled, *enabled);	

	return dsERR_NONE;
}

dsError_t dsSetEnablePersist(int handle, const char* portName, bool enabled)
{
    _DEBUG_ENTER();

	dsAudioPortEnabledParam_t param;
    param.handle = handle;
    param.enabled = enabled;
    memset(param.portName, '\0', sizeof(param.portName));
    errno_t rc = -1;
    rc = strcpy_s (param.portName,sizeof(param.portName), portName);
    if(rc!=EOK)
    {
            ERR_CHK(rc);
    }
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
							(char *)IARM_BUS_DSMGR_API_dsSetEnablePersist,
							(void *)&param,
							sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		return dsERR_NONE;
	}

	return dsERR_GENERAL ;
}

dsError_t dsSetAudioEncoding(int handle, dsAudioEncoding_t encoding)
{
	dsError_t ret = dsERR_NONE;
	/* This is a empty operation in RNG150 */
	return ret;
}


dsError_t dsSetStereoMode(int handle, dsAudioStereoMode_t mode,bool isPersist);
dsError_t dsSetStereoMode(int handle, dsAudioStereoMode_t mode,bool isPersist)
{
    _DEBUG_ENTER();
    _RETURN_IF_ERROR(dsAudioStereoMode_isValid(mode), dsERR_INVALID_PARAM);

    dsAudioSetStereoModeParam_t param;

    param.handle = handle;
    param.mode = mode;
    param.rpcResult = dsERR_NONE;
    param.toPersist = isPersist;

    IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                 (char *)IARM_BUS_DSMGR_API_dsSetStereoMode,
                 (void *)&param,
                 sizeof(param));

    if (dsERR_NONE == param.rpcResult)
    {
        return dsERR_NONE;
    }

   return dsERR_GENERAL ;
}

dsError_t dsSetStereoAuto(int handle, int autoMode, bool isPersist);
dsError_t dsSetStereoAuto(int handle, int autoMode, bool isPersist)
{
    _DEBUG_ENTER();

	dsAudioSetStereoAutoParam_t param;
	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

    param.handle = handle;
    param.autoMode = autoMode;
    param.toPersist = isPersist;


	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsSetStereoAuto,
                            (void *)&param,
                            sizeof(param));
	

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		return dsERR_NONE;
	}
 
   return dsERR_GENERAL ;
}

dsError_t dsSetAudioGain(int handle, float gain)
{
    dsError_t ret = dsERR_GENERAL;
    _DEBUG_ENTER();

    dsAudioGainParam_t param;
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

    param.handle = handle;
    param.gain = gain;

    rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                        (char *)IARM_BUS_DSMGR_API_dsSetAudioGain,
                        (void *)&param,
                        sizeof(param));

    if (IARM_RESULT_SUCCESS == rpcRet)
    {
            ret = dsERR_NONE;
    }

    return ret;
}


dsError_t dsSetAudioDB(int handle, float db)
{
	dsError_t ret = dsERR_NONE;
	/* This is a empty operation in RNG150 */
	return ret;
}


dsError_t dsSetAudioLevel(int handle, float level)
{
    dsError_t ret = dsERR_GENERAL;
    _DEBUG_ENTER();

    dsAudioSetLevelParam_t param;
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

    param.handle = handle;
    param.level = level;

    rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                        (char *)IARM_BUS_DSMGR_API_dsSetAudioLevel,
                        (void *)&param,
                        sizeof(param));

    if (IARM_RESULT_SUCCESS == rpcRet)
    {
            ret = dsERR_NONE;
    }

    return ret;
}

dsError_t dsSetAudioDucking(int handle,dsAudioDuckingAction_t action, dsAudioDuckingType_t type, const unsigned char  level)
{
    dsError_t ret = dsERR_GENERAL;
    _DEBUG_ENTER();

    dsAudioSetDuckingParam_t param;
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

    param.handle = handle;
    param.action = action;
    param.type = type;
    param.level = level;


    rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                        (char *)IARM_BUS_DSMGR_API_dsSetAudioDucking,
                        (void *)&param,
                        sizeof(param));

    if (IARM_RESULT_SUCCESS == rpcRet)
    {
            ret = dsERR_NONE;
    }

    return ret;
}

dsError_t dsEnableLoopThru(int handle, bool loopThru)
{
	dsError_t ret = dsERR_NONE;
	/* This is a empty operation in RNG150 */
	return ret;
}

dsError_t dsSetAudioMute(int handle, bool mute)
{
   
	_DEBUG_ENTER();
 
	dsAudioSetMutedParam_t param;
	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

    param.handle = handle;
    param.mute = mute;



	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsSetAudioMute,
                            (void *)&param,
                            sizeof(param));
	
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		return dsERR_NONE;
	}

  return dsERR_GENERAL ;
}

dsError_t dsAudioPortTerm(void)
{
    _DEBUG_ENTER();
	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsAudioPortTerm,
                            NULL,
                            0);
	
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		return dsERR_NONE;
	}
	return dsERR_GENERAL ;
}

dsError_t dsIsAudioMSDecode(int handle, bool *HasMS11Decode)
{
	dsAudioGetMS11Param_t param;
	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	param.handle = handle;
	param.ms11Enabled = false;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsIsAudioMSDecode,
                            (void *)&param,
                            sizeof(param));
	
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		*HasMS11Decode = param.ms11Enabled;
		return dsERR_NONE;
	}

	return dsERR_NONE;
}

dsError_t dsIsAudioMS12Decode(int handle, bool *HasMS12Decode)
{
	dsAudioGetMS12Param_t param;
	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
	param.handle = handle;
	param.ms12Enabled = false;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsIsAudioMS12Decode,
                            (void *)&param,
                            sizeof(param));
	
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		*HasMS12Decode = param.ms12Enabled;
		return dsERR_NONE;
	}

	return dsERR_NONE;
}



dsError_t dsEnableMS12Config(int handle, dsMS12FEATURE_t feature,const bool enable)
{
	_dsMS12ConfigParam_t param;
	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	param.handle = handle;
	param.feature = feature;
	param.enable = enable;


	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsEnableMS12Config,
                            (void *)&param,
                            sizeof(param));

	if (IARM_RESULT_SUCCESS != rpcRet)
	{
		return dsERR_GENERAL;
	}

	return dsERR_NONE;
}

dsError_t dsSetAudioDelay(int handle, const uint32_t audioDelayMs)
{
	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
	dsSetAudioDelayParam_t param;
	param.handle = handle;
	param.audioDelayMs = audioDelayMs;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
							(char *)IARM_BUS_DSMGR_API_dsSetAudioDelay,
							(void *)&param,
							sizeof(param));

	if (IARM_RESULT_SUCCESS != rpcRet)
	{
		return dsERR_GENERAL;
	}

	return dsERR_NONE;

}

dsError_t dsSetAudioDelayOffset(int handle, const uint32_t audioDelayOffsetMs)
{
	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
	dsAudioDelayOffsetParam_t param;
	param.handle = handle;
	param.audioDelayOffsetMs = audioDelayOffsetMs;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
							(char *)IARM_BUS_DSMGR_API_dsSetAudioDelayOffset,
							(void *)&param,
							sizeof(param));

	if (IARM_RESULT_SUCCESS != rpcRet)
	{
		return dsERR_GENERAL;
	}

	return dsERR_NONE;

}


dsError_t  dsSetDialogEnhancement(int handle, int level)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsDialogEnhancementParam_t param;
        param.handle = handle;
        param.enhancerLevel = level;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                        (char *)IARM_BUS_DSMGR_API_dsSetDialogEnhancement,
                                                        (void *)&param,
                                                        sizeof(param));
        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                return dsERR_GENERAL;
        }
        return dsERR_NONE;
}

dsError_t dsSetAudioCompression(int handle, int compression)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsAudioCompressionParam_t param;
        param.handle = handle;
        param.compression = compression;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                        (char *)IARM_BUS_DSMGR_API_dsSetAudioCompression,
                                                        (void *)&param,
                                                        sizeof(param));
        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                return dsERR_GENERAL;
        }
        return dsERR_NONE;
}

dsError_t dsSetDolbyVolumeMode(int handle, bool mode)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsSetDolbyVolumeParam_t param;
        param.handle = handle;
        param.enable = mode;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                        (char *)IARM_BUS_DSMGR_API_dsSetDolbyVolumeMode,
                                                        (void *)&param,
                                                        sizeof(param));
        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                return dsERR_GENERAL;
        }
        return dsERR_NONE;
}

dsError_t dsSetIntelligentEqualizerMode(int handle, int mode)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsIntelligentEqualizerModeParam_t param;
        param.handle = handle;
        param.mode = mode;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                        (char *)IARM_BUS_DSMGR_API_dsSetIntelligentEqualizerMode,
                                                        (void *)&param,
                                                        sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                return dsERR_GENERAL;
        }
        return dsERR_NONE;
}

dsError_t dsGetAudioCompression(int handle, int *compression)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsAudioCompressionParam_t param;

        param.handle = handle;
        param.compression = 0;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                (char *)IARM_BUS_DSMGR_API_dsGetAudioCompression,
                                                (void *)&param,
                                                sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                printf("%s: AudioCompression (GET) GENERAL ERROR\n", __FUNCTION__);
                return dsERR_GENERAL;
        }

        *compression = param.compression;
        return dsERR_NONE;
}

dsError_t dsGetDialogEnhancement(int handle, int *level)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsDialogEnhancementParam_t param;

        param.handle = handle;
        param.enhancerLevel = 0;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                (char *)IARM_BUS_DSMGR_API_dsGetDialogEnhancement,
                                                (void *)&param,
                                                sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                printf("%s: DialogEnhancement (GET) GENERAL ERROR\n", __FUNCTION__);
                return dsERR_GENERAL;
        }

        *level = param.enhancerLevel;
        return dsERR_NONE;
}

dsError_t dsGetDolbyVolumeMode(int handle, bool *mode)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsSetDolbyVolumeParam_t param;

        param.handle = handle;
        param.enable = false;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                (char *)IARM_BUS_DSMGR_API_dsGetDolbyVolumeMode,
                                                (void *)&param,
                                                sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                printf("%s: IntelligentEqualizerMode (GET) GENERAL ERROR\n", __FUNCTION__);
                return dsERR_GENERAL;
        }

        *mode = param.enable;
        return dsERR_NONE;
}

dsError_t dsGetIntelligentEqualizerMode(int handle, int *mode)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsIntelligentEqualizerModeParam_t param;

        param.handle = handle;
        param.mode = 0;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                (char *)IARM_BUS_DSMGR_API_dsGetIntelligentEqualizerMode,
                                                (void *)&param,
                                                sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                printf("%s: IntelligentEqualizerMode (GET) GENERAL ERROR\n", __FUNCTION__);
                return dsERR_GENERAL;
        }

        *mode = param.mode;
        return dsERR_NONE;
}

dsError_t dsGetVolumeLeveller(int handle, dsVolumeLeveller_t *volLeveller)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsVolumeLevellerParam_t param;

        param.handle = handle;
	param.volLeveller.mode = 0;
        param.volLeveller.level = 0;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                (char *)IARM_BUS_DSMGR_API_dsGetVolumeLeveller,
                                                (void *)&param,
                                                sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                printf("%s: (GET) GENERAL ERROR\n", __FUNCTION__);
                return dsERR_GENERAL;
        }

	volLeveller->mode = param.volLeveller.mode;
        volLeveller->level = param.volLeveller.level;
        return dsERR_NONE;
}

dsError_t  dsSetVolumeLeveller(int handle, dsVolumeLeveller_t volLeveller)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsVolumeLevellerParam_t param;
        param.handle = handle;
	param.volLeveller.mode = volLeveller.mode;
        param.volLeveller.level = volLeveller.level;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                        (char *)IARM_BUS_DSMGR_API_dsSetVolumeLeveller,
                                                        (void *)&param,
                                                        sizeof(param));
        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                return dsERR_GENERAL;
        }
        return dsERR_NONE;
}

dsError_t dsGetBassEnhancer(int handle, int *boost)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsBassEnhancerParam_t param;

        param.handle = handle;
        param.boost = 0;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                (char *)IARM_BUS_DSMGR_API_dsGetBassEnhancer,
                                                (void *)&param,
                                                sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                printf("%s: (GET) GENERAL ERROR\n", __FUNCTION__);
                return dsERR_GENERAL;
        }

        *boost = param.boost;
        return dsERR_NONE;
}

dsError_t  dsSetBassEnhancer(int handle, int boost)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsBassEnhancerParam_t param;
        param.handle = handle;
        param.boost = boost;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                        (char *)IARM_BUS_DSMGR_API_dsSetBassEnhancer,
                                                        (void *)&param,
                                                        sizeof(param));
        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                return dsERR_GENERAL;
        }
        return dsERR_NONE;
}

dsError_t dsIsSurroundDecoderEnabled(int handle, bool *enabled)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsSurroundDecoderParam_t param;

        param.handle = handle;
        param.enable = false;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                (char *)IARM_BUS_DSMGR_API_dsIsSurroundDecoderEnabled,
                                                (void *)&param,
                                                sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                printf("%s: (GET) GENERAL ERROR\n", __FUNCTION__);
                return dsERR_GENERAL;
        }

        *enabled = param.enable;
        return dsERR_NONE;
}

dsError_t  dsEnableSurroundDecoder(int handle, bool enabled)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsSurroundDecoderParam_t param;
        param.handle = handle;
        param.enable = enabled;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                        (char *)IARM_BUS_DSMGR_API_dsEnableSurroundDecoder,
                                                        (void *)&param,
                                                        sizeof(param));
        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                return dsERR_GENERAL;
        }
        return dsERR_NONE;
}

dsError_t dsGetDRCMode(int handle, int *mode)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsDRCModeParam_t param;

        param.handle = handle;
        param.mode = 0;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                (char *)IARM_BUS_DSMGR_API_dsGetDRCMode,
                                                (void *)&param,
                                                sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                printf("%s: (GET) GENERAL ERROR\n", __FUNCTION__);
                return dsERR_GENERAL;
        }

        *mode = param.mode;
        return dsERR_NONE;
}

dsError_t  dsSetDRCMode(int handle, int mode)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsDRCModeParam_t param;
        param.handle = handle;
        param.mode = mode;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                        (char *)IARM_BUS_DSMGR_API_dsSetDRCMode,
                                                        (void *)&param,
                                                        sizeof(param));
        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                return dsERR_GENERAL;
        }
        return dsERR_NONE;
}

dsError_t dsGetSurroundVirtualizer(int handle, dsSurroundVirtualizer_t* virtualizer)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsSurroundVirtualizerParam_t param;

        param.handle = handle;
	param.virtualizer.mode = 0;
        param.virtualizer.boost = 0;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                (char *)IARM_BUS_DSMGR_API_dsGetSurroundVirtualizer,
                                                (void *)&param,
                                                sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                printf("%s: (GET) GENERAL ERROR\n", __FUNCTION__);
                return dsERR_GENERAL;
        }

	virtualizer->mode = param.virtualizer.mode;
        virtualizer->boost = param.virtualizer.boost;
        return dsERR_NONE;
}

dsError_t  dsSetSurroundVirtualizer(int handle, dsSurroundVirtualizer_t virtualizer)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsSurroundVirtualizerParam_t param;
        param.handle = handle;
	param.virtualizer.mode = virtualizer.mode;
        param.virtualizer.boost = virtualizer.boost;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                        (char *)IARM_BUS_DSMGR_API_dsSetSurroundVirtualizer,
                                                        (void *)&param,
                                                        sizeof(param));
        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                return dsERR_GENERAL;
        }
        return dsERR_NONE;
}

dsError_t dsGetMISteering(int handle, bool *enabled)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsMISteeringParam_t param;

        param.handle = handle;
        param.enable = false;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                (char *)IARM_BUS_DSMGR_API_dsGetMISteering,
                                                (void *)&param,
                                                sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                printf("%s: (GET) GENERAL ERROR\n", __FUNCTION__);
                return dsERR_GENERAL;
        }

        *enabled = param.enable;
        return dsERR_NONE;
}

dsError_t  dsSetMISteering(int handle, bool enabled)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsMISteeringParam_t param;
        param.handle = handle;
        param.enable = enabled;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                        (char *)IARM_BUS_DSMGR_API_dsSetMISteering,
                                                        (void *)&param,
                                                        sizeof(param));
        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                return dsERR_GENERAL;
        }
        return dsERR_NONE;
}

dsError_t dsGetGraphicEqualizerMode(int handle, int *mode)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsGraphicEqualizerModeParam_t param;

        param.handle = handle;
        param.mode = 0;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                (char *)IARM_BUS_DSMGR_API_dsGetGraphicEqualizerMode,
                                                (void *)&param,
                                                sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                printf("%s: GraphicEqualizerMode (GET) GENERAL ERROR\n", __FUNCTION__);
                return dsERR_GENERAL;
        }

        *mode = param.mode;
        return dsERR_NONE;
}

dsError_t dsSetGraphicEqualizerMode(int handle, int mode)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsGraphicEqualizerModeParam_t param;
        param.handle = handle;
        param.mode = mode;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                        (char *)IARM_BUS_DSMGR_API_dsSetGraphicEqualizerMode,
                                                        (void *)&param,
                                                        sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                return dsERR_GENERAL;
        }
        return dsERR_NONE;
}

dsError_t dsGetMS12AudioProfileList(int handle, dsMS12AudioProfileList_t* profiles)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsMS12AudioProfileListParam_t param;

        param.handle = handle;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                (char *)IARM_BUS_DSMGR_API_dsGetMS12AudioProfileList,
                                                (void *)&param,
                                                sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                printf("%s: (GET) GENERAL ERROR\n", __FUNCTION__);
                return dsERR_GENERAL;
        }

        profiles->audioProfileCount = param.profileList.audioProfileCount;
	strncpy(profiles->audioProfileList, param.profileList.audioProfileList, MAX_PROFILE_LIST_BUFFER_LEN);
        return dsERR_NONE;
}

dsError_t dsGetMS12AudioProfile(int handle, char *profile)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsMS12AudioProfileParam_t param;

        param.handle = handle;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                (char *)IARM_BUS_DSMGR_API_dsGetMS12AudioProfile,
                                                (void *)&param,
                                                sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                printf("%s: (GET) GENERAL ERROR\n", __FUNCTION__);
                return dsERR_GENERAL;
        }


        strncpy(profile, param.profile, MAX_PROFILE_STRING_LEN);
        return dsERR_NONE;
}

dsError_t  dsSetMS12AudioProfile(int handle, const char* profile)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsMS12AudioProfileParam_t param;
        param.handle = handle;

	memset( param.profile, 0, sizeof(param.profile) );
        errno_t rc = -1;
        rc = strcpy_s (param.profile,sizeof(param.profile), profile );
        if(rc!=EOK)
        {
                ERR_CHK(rc);
        }
	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                        (char *)IARM_BUS_DSMGR_API_dsSetMS12AudioProfile,
                                                        (void *)&param,
                                                        sizeof(param));
        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                return dsERR_GENERAL;
        }
        return dsERR_NONE;
}

dsError_t  dsSetAssociatedAudioMixing(int handle, bool mixing)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsAssociatedAudioMixingParam_t param;
        param.handle = handle;
        param.mixing = mixing;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                  (char *)IARM_BUS_DSMGR_API_dsSetAssociatedAudioMixing,
                                                  (void *)&param,
                                                  sizeof(param));
        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                return dsERR_GENERAL;
        }
        return dsERR_NONE;
}


dsError_t dsGetAssociatedAudioMixing(int handle, bool *mixing)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsAssociatedAudioMixingParam_t param;

        param.handle = handle;
        param.mixing = false;

	if(mixing == NULL) {
            printf("%s: (GET) Invalid Param error\n", __FUNCTION__);
            return dsERR_INVALID_PARAM;
        }
        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                  (char *)IARM_BUS_DSMGR_API_dsGetAssociatedAudioMixing,
                                                  (void *)&param,
                                                  sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                printf("%s: (GET) GENERAL ERROR\n", __FUNCTION__);
                return dsERR_GENERAL;
        }

        *mixing = param.mixing;
        return dsERR_NONE;
}

dsError_t dsSetFaderControl(int handle, int mixerbalance)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsFaderControlParam_t param;
        param.handle = handle;
        param.mixerbalance = mixerbalance;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                  (char *)IARM_BUS_DSMGR_API_dsSetFaderControl,
                                                  (void *)&param,
                                                  sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                return dsERR_GENERAL;
        }
        return dsERR_NONE;
}


dsError_t dsGetFaderControl(int handle, int *mixerbalance)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsFaderControlParam_t param;

        param.handle = handle;
        param.mixerbalance = 0;

        if(mixerbalance == NULL) {
            printf("%s: (GET) Invalid Param error\n", __FUNCTION__);
            return dsERR_INVALID_PARAM;
        }

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                (char *)IARM_BUS_DSMGR_API_dsGetFaderControl,
                                                (void *)&param,
                                                sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                printf("%s: mixerbalance (GET) GENERAL ERROR\n", __FUNCTION__);
                return dsERR_GENERAL;
        }

        *mixerbalance = param.mixerbalance;
        return dsERR_NONE;
}

dsError_t  dsSetPrimaryLanguage(int handle, const char* pLang)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsPrimaryLanguageParam_t param;
        param.handle = handle;

        if(pLang == NULL) {
            printf("%s: (SET) Invalid Param error\n", __FUNCTION__);
            return dsERR_INVALID_PARAM;
        }

        memset(param.primaryLanguage, '\0', sizeof(param.primaryLanguage));
        errno_t rc = -1;
        rc = strcpy_s (param.primaryLanguage,sizeof(param.primaryLanguage), pLang);
        if(rc!=EOK)
        {
                ERR_CHK(rc);
        }
        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                  (char *)IARM_BUS_DSMGR_API_dsSetPrimaryLanguage,
                                                  (void *)&param,
                                                  sizeof(param));
        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                return dsERR_GENERAL;
        }
        return dsERR_NONE;
}

dsError_t dsGetPrimaryLanguage(int handle, char *pLang)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsPrimaryLanguageParam_t param;

        param.handle = handle;

        if(pLang == NULL) {
            printf("%s: (GET) Invalid Param error\n", __FUNCTION__);
            return dsERR_INVALID_PARAM;
        }

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                (char *)IARM_BUS_DSMGR_API_dsGetPrimaryLanguage,
                                                (void *)&param,
                                                sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                printf("%s: (GET) GENERAL ERROR\n", __FUNCTION__);
                return dsERR_GENERAL;
        }


        strcpy_s(pLang, sizeof(param.primaryLanguage), param.primaryLanguage);
        return dsERR_NONE;
}

dsError_t  dsSetSecondaryLanguage(int handle, const char* sLang)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsSecondaryLanguageParam_t param;
        param.handle = handle;

        if(sLang == NULL) {
            printf("%s: (SET) Invalid Param error\n", __FUNCTION__);
            return dsERR_INVALID_PARAM;
        }

        memset(param.secondaryLanguage, '\0', sizeof(param.secondaryLanguage));
        errno_t rc = -1;
        rc = strcpy_s (param.secondaryLanguage,sizeof(param.secondaryLanguage), sLang);
        if(rc!=EOK)
        {
                ERR_CHK(rc);
        }
        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                  (char *)IARM_BUS_DSMGR_API_dsSetSecondaryLanguage,
                                                  (void *)&param,
                                                  sizeof(param));
        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                return dsERR_GENERAL;
        }
        return dsERR_NONE;
}

dsError_t dsGetSecondaryLanguage(int handle, char *sLang)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsSecondaryLanguageParam_t param;

        param.handle = handle;

        if(sLang == NULL) {
            printf("%s: (GET) Invalid Param error\n", __FUNCTION__);
            return dsERR_INVALID_PARAM;
        }

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                (char *)IARM_BUS_DSMGR_API_dsGetSecondaryLanguage,
                                                (void *)&param,
                                                sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                printf("%s: (GET) GENERAL ERROR\n", __FUNCTION__);
                return dsERR_GENERAL;
        }


        strcpy_s(sLang, sizeof(param.secondaryLanguage), param.secondaryLanguage);
        return dsERR_NONE;
}

dsError_t  dsSetMS12AudioProfileSetttingsOverride(int handle,const char* profileState,const char* profileName,
                                                   const char* profileSettingsName,const char* profileSettingValue)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsMS12SetttingsOverrideParam_t param;
        param.handle = handle;
        
        memset( param.profileState, 0, sizeof(param.profileState) );
        errno_t rc = -1;
        rc = strcpy_s (param.profileState,sizeof(param.profileState), profileState );
        if(rc!=EOK)
        {
                ERR_CHK(rc);
        }

        memset( param.profileName, 0, sizeof(param.profileName) );
        rc = strcpy_s (param.profileName,sizeof(param.profileName), profileName );
        if(rc!=EOK)
        {
                ERR_CHK(rc);
        }

        memset( param.profileSettingsName, 0, sizeof(param.profileSettingsName) );
        rc = strcpy_s (param.profileSettingsName,sizeof(param.profileSettingsName), profileSettingsName );
        if(rc!=EOK)
        {
                ERR_CHK(rc);
        }

        memset( param.profileSettingValue, 0, sizeof(param.profileSettingValue) );
        rc = strcpy_s (param.profileSettingValue,sizeof(param.profileSettingValue),profileSettingValue);
        if(rc!=EOK)
        {
                ERR_CHK(rc);
        }
 
        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                        (char *)IARM_BUS_DSMGR_API_dsSetMS12SetttingsOverride,
                                                        (void *)&param,
                                                        sizeof(param));
        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                return dsERR_GENERAL;
        }
        return dsERR_NONE;

}

dsError_t dsGetSupportedARCTypes(int handle, int *types)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsGetSupportedARCTypesParam_t param;

        param.handle = handle;
        param.types = dsAUDIOARCSUPPORT_NONE;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                (char *)IARM_BUS_DSMGR_API_dsGetSupportedARCTypes,
                                                (void *)&param,
                                                sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                printf("%s: (GET) GENERAL ERROR\n", __FUNCTION__);
                return dsERR_GENERAL;
        }

        *types = param.types;
        return dsERR_NONE;
}

dsError_t dsAudioSetSAD(int handle, dsAudioSADList_t sad_list)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsAudioSetSADParam_t param;
        param.handle = handle;
	param.list.count = sad_list.count;

	for(int i=0;i<sad_list.count;i++) {
	    param.list.sad[i] = sad_list.sad[i];
	}

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                (char *)IARM_BUS_DSMGR_API_dsAudioSetSAD,
                                                (void *)&param,
                                                sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                printf("%s: (SET) GENERAL ERROR\n", __FUNCTION__);
                return dsERR_GENERAL;
        }

        return dsERR_NONE;
}

dsError_t  dsAudioEnableARC(int handle, dsAudioARCStatus_t arcStatus)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsAudioEnableARCParam_t param;
        param.handle = handle;
        param.arcStatus.type = arcStatus.type;
        param.arcStatus.status = arcStatus.status;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                        (char *)IARM_BUS_DSMGR_API_dsAudioEnableARC,
                                                        (void *)&param,
                                                        sizeof(param));
        if (IARM_RESULT_SUCCESS != rpcRet)
        {
		printf("%s: (SET) GENERAL ERROR\n", __FUNCTION__);
                return dsERR_GENERAL;
        }
        return dsERR_NONE;
}

dsError_t dsGetAudioDelay(int handle, uint32_t *audioDelayMs)
{
	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
	dsGetAudioDelayParam_t param;

	param.handle = handle;
	param.audioDelayMs = 0;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
						(char *)IARM_BUS_DSMGR_API_dsGetAudioDelay,
						(void *)&param,
						sizeof(param));

	if (IARM_RESULT_SUCCESS != rpcRet)
	{
		INT_ERROR("%s: AUDIODELAY CLIENT (GET) GENERAL ERROR\n", __FUNCTION__);
		return dsERR_GENERAL;
	}

	*audioDelayMs = param.audioDelayMs;
	return dsERR_NONE;
}

dsError_t dsGetAudioDelayOffset(int handle, uint32_t *audioDelayOffsetMs)
{
	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
	dsAudioDelayOffsetParam_t param;

	param.handle = handle;
	param.audioDelayOffsetMs = 0;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
						(char *)IARM_BUS_DSMGR_API_dsGetAudioDelayOffset,
						(void *)&param,
						sizeof(param));

	if (IARM_RESULT_SUCCESS != rpcRet)
	{
		printf("%s: AUDIODELAY CLIENT (GET) GENERAL ERROR\n", __FUNCTION__);
		return dsERR_GENERAL;
	}

	*audioDelayOffsetMs = param.audioDelayOffsetMs;
	return dsERR_NONE;
}


dsError_t dsSetAudioAtmosOutputMode(int handle, bool enable)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsAudioSetAtmosOutputModeParam_t param;

        param.handle = handle;
        param.enable = enable;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                (char *)IARM_BUS_DSMGR_API_dsSetAudioAtmosOutputMode,
                                                (void *)&param,
                                                sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                printf("%s: AUDIODELAY CLIENT (GET) GENERAL ERROR\n", __FUNCTION__);
                return dsERR_GENERAL;
        }

        return dsERR_NONE;
}
dsError_t dsGetSinkDeviceAtmosCapability(int handle, dsATMOSCapability_t *capability)
{
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
        dsGetAudioAtmosCapabilityParam_t param;

        param.handle = handle;
        param.capability= dsAUDIO_ATMOS_NOTSUPPORTED;

        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                                                (char *)IARM_BUS_DSMGR_API_dsGetSinkDeviceAtmosCapability,
                                                (void *)&param,
                                                sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                printf("%s: AUDIODELAY CLIENT (GET) GENERAL ERROR\n", __FUNCTION__);
                return dsERR_GENERAL;
        }

        *capability = param.capability;
        return dsERR_NONE;
}

dsError_t dsEnableLEConfig(int handle, const bool enable)
{
	_dsLEConfigParam_t param;
	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	param.handle = handle;
	param.enable = enable;


	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsEnableLEConfig,
                            (void *)&param,
                            sizeof(param));

	if (IARM_RESULT_SUCCESS != rpcRet)
	{
		return dsERR_GENERAL;
	}

	return dsERR_NONE;
}

dsError_t dsGetLEConfig(int handle, bool *enable)
{
        dsGetLEConfigParam_t param;
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

        param.handle = handle;
        param.enable = false;


        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsGetLEConfig,
                            (void *)&param,
                            sizeof(param));

        if (IARM_RESULT_SUCCESS != rpcRet)
        {
                return dsERR_GENERAL;
        }

        *enable = param.enable;
        return dsERR_NONE;
}

dsError_t dsGetAudioCapabilities(int handle, int *capabilities)
{
	_DEBUG_ENTER();

	dsGetHDRCapabilitiesParam_t param;
	param.handle = handle;

	IARM_Result_t rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
			(char *) IARM_BUS_DSMGR_API_dsGetAudioCapabilities,
			(void *) &param,
			sizeof(param));
	
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		*capabilities = param.capabilities;
		return param.result;
	}

	return dsERR_GENERAL ;
}

dsError_t dsGetMS12Capabilities(int handle, int *capabilities)
{
	_DEBUG_ENTER();

	dsGetMS12CapabilitiesParam_t param;
	param.handle = handle;

	IARM_Result_t rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
			(char *) IARM_BUS_DSMGR_API_dsGetMS12Capabilities,
			(void *) &param,
			sizeof(param));
	
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		*capabilities = param.capabilities;
		return param.result;
	}

	return dsERR_GENERAL ;
}

dsError_t dsAudioOutIsConnected(int handle, bool* pisCon)
{
        dsAudioOutIsConnectedParam_t param;
        IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

        param.handle = handle;
        param.isCon = true;
        rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *) IARM_BUS_DSMGR_API_dsAudioOutIsConnected,
                            (void *)&param,
                            sizeof(param));
        if (IARM_RESULT_SUCCESS == rpcRet)
        {
                *pisCon = param.isCon;
                return dsERR_NONE;
        }
        return dsERR_GENERAL ;
}

dsError_t dsResetDialogEnhancement(int handle)
{
       IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
       rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *) IARM_BUS_DSMGR_API_dsResetDialogEnhancement,
                            (void *)&handle,
                            sizeof(int*));
        if (IARM_RESULT_SUCCESS == rpcRet)
        {
                return dsERR_NONE;
        }
        return dsERR_GENERAL ;

}


dsError_t dsResetBassEnhancer(int handle)
{
       IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
       rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *) IARM_BUS_DSMGR_API_dsResetBassEnhancer,
                            (void *)&handle,
                            sizeof(int*));
        if (IARM_RESULT_SUCCESS == rpcRet)
        {
                return dsERR_NONE;
        }
        return dsERR_GENERAL ;

}

dsError_t dsResetSurroundVirtualizer(int handle)
{
       IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
       rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *) IARM_BUS_DSMGR_API_dsResetSurroundVirtualizer,
                            (void *)&handle,
                            sizeof(int*));
        if (IARM_RESULT_SUCCESS == rpcRet)
        {
                return dsERR_NONE;
        }
        return dsERR_GENERAL ;

}

dsError_t dsResetVolumeLeveller(int handle)
{
       IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
       rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *) IARM_BUS_DSMGR_API_dsResetVolumeLeveller,
                            (void *)&handle,
                            sizeof(int*));
        if (IARM_RESULT_SUCCESS == rpcRet)
        {
                return dsERR_NONE;
        }
        return dsERR_GENERAL ;

}

/** @} */
/** @} */
