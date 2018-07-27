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


dsError_t dsGetAudioCompression(int handle, dsAudioCompression_t *compression)
{
    dsError_t ret = dsERR_NONE;
    ret = dsERR_OPERATION_NOT_SUPPORTED;
	return ret;
}

dsError_t dsGetStereoMode(int handle, dsAudioStereoMode_t *stereoMode)
{
	dsError_t ret = dsERR_NONE;
	dsAudioSetStereoModeParam_t param;

	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

    param.handle = handle;
    param.mode = dsAUDIO_STEREO_STEREO; /* Default to stereo */

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

dsError_t dsGetPersistedStereoMode(int handle, dsAudioStereoMode_t *stereoMode)
{
	dsError_t ret = dsERR_NONE;
	dsAudioSetStereoModeParam_t param;

	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

    param.handle = handle;
    param.toPersist = 1;
    param.mode = dsAUDIO_STEREO_STEREO; /* Default to stereo */

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
	dsError_t ret = dsERR_NONE;
    ret = dsERR_OPERATION_NOT_SUPPORTED;
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
	dsError_t ret = dsERR_NONE;
    ret = dsERR_OPERATION_NOT_SUPPORTED;
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
	dsError_t ret = dsERR_NONE;
    ret = dsERR_OPERATION_NOT_SUPPORTED;
	return ret;
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

dsError_t  dsEnableAudioPort(int handle, bool enabled)
{
    _DEBUG_ENTER();

	dsAudioPortEnabledParam_t param;
    param.handle = handle;
    param.enabled = enabled;

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



dsError_t dsSetAudioEncoding(int handle, dsAudioEncoding_t encoding)
{
	dsError_t ret = dsERR_NONE;
	/* This is a empty operation in RNG150 */
	return ret;
}

dsError_t dsSetAudioCompression(int handle, dsAudioCompression_t compression)
{
	dsError_t ret = dsERR_NONE;
	/* This is a empty operation in RNG150 */
	return ret;
}


dsError_t dsSetStereoMode(int handle, dsAudioStereoMode_t mode,bool IsPersist);
dsError_t dsSetStereoMode(int handle, dsAudioStereoMode_t mode,bool IsPersist)
{
    _DEBUG_ENTER();
    _RETURN_IF_ERROR(dsAudioStereoMode_isValid(mode), dsERR_INVALID_PARAM);

	dsAudioSetStereoModeParam_t param;

    param.handle = handle;
    param.mode = mode;
    param.rpcResult = dsERR_NONE;
	param.toPersist = IsPersist;
	  
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

dsError_t dsSetStereoAuto(int handle, int autoMode)
{
    _DEBUG_ENTER();

	dsAudioSetStereoAutoParam_t param;
	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

    param.handle = handle;
    param.autoMode = autoMode;


  
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
	dsError_t ret = dsERR_NONE;
	/* This is a empty operation in RNG150 */
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
	dsError_t ret = dsERR_NONE;
	/* This is a empty operation in RNG150 */
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

/** @} */
/** @} */
