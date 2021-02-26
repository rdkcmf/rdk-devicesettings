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
/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright ARRIS Enterprises, Inc. 2015.
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
 * limitations under
*/ 


/**
* @defgroup devicesettings
* @{
* @defgroup rpc
* @{
**/


#include <stdio.h>
#include <string.h>
#include "dsHdmiIn.h"
#include "dsRpc.h"
#include "dsMgr.h"
#include "dsclientlogger.h"
#include "iarmUtil.h"
#include "libIARM.h"
#include "libIBus.h"


dsError_t dsHdmiInInit (void)
{
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsHdmiInInit,
                            NULL,
                            0);
  
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
            printf("%s:%d - dsERR_NONE\n", __PRETTY_FUNCTION__,__LINE__);
		return dsERR_NONE;
	}

    printf("%s:%d - dsERR_GENERAL\n", __PRETTY_FUNCTION__,__LINE__);
	return dsERR_GENERAL;
}


dsError_t dsHdmiInTerm (void)
{
    _DEBUG_ENTER();

   IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
   rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsHdmiInTerm,
                            NULL,
                            0);
	
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
        printf("%s:%d - dsERR_NONE\n", __PRETTY_FUNCTION__,__LINE__);
		return dsERR_NONE;
	}

    printf("%s:%d - dsERR_GENERAL\n", __PRETTY_FUNCTION__,__LINE__);
	return dsERR_GENERAL ;
}


dsError_t dsHdmiInGetNumberOfInputs (uint8_t *pNumHdmiInputs)
{
    _DEBUG_ENTER();
    _RETURN_IF_ERROR(pNumHdmiInputs != NULL, dsERR_INVALID_PARAM);

    dsHdmiInGetNumberOfInputsParam_t param;
    param.numHdmiInputs = 0;

    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsHdmiInGetNumberOfInputs,
                            (void *)&param,
                            sizeof(param));
  
    if (IARM_RESULT_SUCCESS == rpcRet)
    {
        *pNumHdmiInputs = param.numHdmiInputs;
        printf("%s:%d - %d\n", __PRETTY_FUNCTION__,__LINE__, param.result);
        return param.result;
	}

    printf("%s:%d - dsERR_GENERAL\n", __PRETTY_FUNCTION__,__LINE__);
	return dsERR_GENERAL;
}

dsError_t dsHdmiInGetStatus (dsHdmiInStatus_t *pStatus)
{
    _DEBUG_ENTER();
    _RETURN_IF_ERROR(pStatus != NULL, dsERR_INVALID_PARAM);


    dsHdmiInGetStatusParam_t param;
    memset (&param, 0, sizeof(param));

    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsHdmiInGetStatus,
                            (void *)&param,
                            sizeof(param));
  
    if (IARM_RESULT_SUCCESS == rpcRet)
    {
        *pStatus = param.status;
        printf("%s:%d - %d\n", __PRETTY_FUNCTION__,__LINE__, param.result);
        return param.result;
	}

    printf("%s:%d - dsERR_GENERAL\n", __PRETTY_FUNCTION__,__LINE__);
	return dsERR_GENERAL;
}

dsError_t dsHdmiInSelectPort (dsHdmiInPort_t ePort)
{
    _DEBUG_ENTER();

    dsHdmiInSelectPortParam_t param;
    param.port = ePort;

    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsHdmiInSelectPort,
                            (void *)&param,
                            sizeof(param));
  
    if (IARM_RESULT_SUCCESS == rpcRet)
    {
        printf("%s:%d - %d\n", __PRETTY_FUNCTION__,__LINE__, param.result);
        return param.result;
	}

    printf("%s:%d - dsERR_GENERAL\n", __PRETTY_FUNCTION__,__LINE__);
	return dsERR_GENERAL;
}

dsError_t dsHdmiInScaleVideo (int32_t x, int32_t y, int32_t width, int32_t height)
{
    _DEBUG_ENTER();

    dsHdmiInScaleVideoParam_t param;
    param.videoRect.x      = x;
    param.videoRect.y      = y;
    param.videoRect.width  = width;
    param.videoRect.height = height;
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsHdmiInScaleVideo,
                            (void *)&param,
                            sizeof(param));
  
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
        printf("%s:%d - %d\n", __PRETTY_FUNCTION__,__LINE__, param.result);
        return param.result;
	}

    printf("%s:%d - dsERR_GENERAL\n", __PRETTY_FUNCTION__,__LINE__);
	return dsERR_GENERAL;
}

dsError_t dsHdmiInSelectZoomMode (dsVideoZoom_t requestedZoomMode)
{
    _DEBUG_ENTER();

    dsHdmiInSelectZoomModeParam_t param;
    param.zoomMode = requestedZoomMode;
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsHdmiInSelectZoomMode,
                            (void *)&param,
                            sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
        printf("%s:%d - %d\n", __PRETTY_FUNCTION__,__LINE__, param.result);
        return param.result;
	}

    printf("%s:%d - dsERR_GENERAL\n", __PRETTY_FUNCTION__,__LINE__);
	return dsERR_GENERAL;
}

dsError_t dsHdmiInPauseAudio ()
{
    _DEBUG_ENTER();

    dsError_t param;
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsHdmiInPauseAudio,
                            (void *)&param,
                            sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
        printf("%s:%d - %d\n", __PRETTY_FUNCTION__,__LINE__, param);
        return param;
	}

    printf("%s:%d - dsERR_GENERAL\n", __PRETTY_FUNCTION__,__LINE__);
	return dsERR_GENERAL;
}

dsError_t dsHdmiInResumeAudio ()
{
    _DEBUG_ENTER();

    dsError_t param;
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsHdmiInResumeAudio,
                            (void *)&param,
                            sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
        printf("%s:%d - %d\n", __PRETTY_FUNCTION__,__LINE__, param);
        return param;
	}

    printf("%s:%d - dsERR_GENERAL\n", __PRETTY_FUNCTION__,__LINE__);
	return dsERR_GENERAL;
}

dsError_t dsHdmiInGetCurrentVideoMode (dsVideoPortResolution_t *resolution)
{
    _DEBUG_ENTER();
    _RETURN_IF_ERROR(resolution != NULL, dsERR_INVALID_PARAM);


    _dsHdmiInGetResolutionParam_t param;
    param.resolution = *resolution;

    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsHdmiInGetCurrentVideoMode,
                            (void *)&param,
                            sizeof(param));

    if (IARM_RESULT_SUCCESS == rpcRet)
    {
        *resolution = param.resolution;
        printf("%s:%d - %d\n", __PRETTY_FUNCTION__,__LINE__, param.result);
        return param.result;
	}

    printf("%s:%d - dsERR_GENERAL\n", __PRETTY_FUNCTION__,__LINE__);
	return dsERR_GENERAL;
}

dsError_t dsGetEDIDBytesInfo (int iHdmiPort, unsigned char **edid, int *length)
{
    _DEBUG_ENTER();
    _RETURN_IF_ERROR(edid != NULL, dsERR_INVALID_PARAM);
    _RETURN_IF_ERROR(length != NULL, dsERR_INVALID_PARAM);


    dsGetEDIDBytesInfoParam_t param;
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
    param.iHdmiPort = iHdmiPort;
    rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsGetEDIDBytesInfo,
                            (void *)&param,
                            sizeof(param));

    if (IARM_RESULT_SUCCESS == rpcRet)
    {
        *length = param.length;
        *edid = param.edid;
        printf("[cli] %s: dsGetEDIDBytesInfo eRet: %d data len: %d \r\n", __FUNCTION__, param.result, *length);
        return param.result;
    }
    printf("%s:%d - dsERR_GENERAL\n", __PRETTY_FUNCTION__,__LINE__);
    return dsERR_GENERAL;
}


/** @} */
/** @} */
