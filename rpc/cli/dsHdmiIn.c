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

/* Enable for debug tracing */
/* #define HDMI_IN_DEBUG */
#ifdef HDMI_IN_DEBUG
   #define HDMI_IN_TRACE( m ) printf m
#else
   #define HDMI_IN_TRACE( m )
#endif

dsError_t dsHdmiInInit (void)
{
    HDMI_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));
    HDMI_IN_TRACE(("<<<<< HDMI In is initialized in Multi-App Mode >>>>>>>>\r\n"));

    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsHdmiInInit,
                            NULL,
                            0);
  
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
        HDMI_IN_TRACE(("%s <--- dsERR_NONE\n", __PRETTY_FUNCTION__));
		return dsERR_NONE;
	}

    HDMI_IN_TRACE(("%s <--- dsERR_GENERAL\n", __PRETTY_FUNCTION__));
	return dsERR_GENERAL;
}


dsError_t dsHdmiInTerm (void)
{
    _DEBUG_ENTER();
    HDMI_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

   IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
   rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsHdmiInTerm,
                            NULL,
                            0);
	
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
        HDMI_IN_TRACE(("%s <--- dsERR_NONE\n", __PRETTY_FUNCTION__));
		return dsERR_NONE;
	}

    HDMI_IN_TRACE(("%s <--- dsERR_GENERAL\n", __PRETTY_FUNCTION__));
	return dsERR_GENERAL ;
}


dsError_t dsHdmiInGetNumberOfInputs (uint8_t *pNumHdmiInputs)
{
    _DEBUG_ENTER();
    _RETURN_IF_ERROR(pNumHdmiInputs != NULL, dsERR_INVALID_PARAM);

    HDMI_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

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
        HDMI_IN_TRACE(("%s <--- %d\n", __PRETTY_FUNCTION__, param.result));
        return param.result;
	}

    HDMI_IN_TRACE(("%s <--- dsERR_GENERAL\n", __PRETTY_FUNCTION__));
	return dsERR_GENERAL;
}

dsError_t dsHdmiInGetStatus (dsHdmiInStatus_t *pStatus)
{
    _DEBUG_ENTER();
    _RETURN_IF_ERROR(pStatus != NULL, dsERR_INVALID_PARAM);

    HDMI_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

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
        HDMI_IN_TRACE(("%s <--- %d\n", __PRETTY_FUNCTION__, param.result));
        return param.result;
	}

    HDMI_IN_TRACE(("%s <--- dsERR_GENERAL\n", __PRETTY_FUNCTION__));
	return dsERR_GENERAL;
}

dsError_t dsHdmiInSelectPort (dsHdmiInPort_t ePort)
{
    _DEBUG_ENTER();
    HDMI_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    dsHdmiInSelectPortParam_t param;
    param.port = ePort;

    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsHdmiInSelectPort,
                            (void *)&param,
                            sizeof(param));
  
    if (IARM_RESULT_SUCCESS == rpcRet)
    {
        HDMI_IN_TRACE(("%s <--- %d\n", __PRETTY_FUNCTION__, param.result));
        return param.result;
	}

    HDMI_IN_TRACE(("%s <--- dsERR_GENERAL\n", __PRETTY_FUNCTION__));
	return dsERR_GENERAL;
}

dsError_t dsHdmiInScaleVideo (int32_t x, int32_t y, int32_t width, int32_t height)
{
    _DEBUG_ENTER();
    HDMI_IN_TRACE(("%s ---> x=%d, y=%d, width=%d, height=%d\n", __PRETTY_FUNCTION__, x, y, width, height));

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
        HDMI_IN_TRACE(("%s <--- %d\n", __PRETTY_FUNCTION__, param.result));
        return param.result;
	}

    HDMI_IN_TRACE(("%s <--- dsERR_GENERAL\n", __PRETTY_FUNCTION__));
	return dsERR_GENERAL;
}

dsError_t dsHdmiInSelectZoomMode (dsVideoZoom_t requestedZoomMode)
{
    _DEBUG_ENTER();
    HDMI_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    dsHdmiInSelectZoomModeParam_t param;
    param.zoomMode = requestedZoomMode;
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsHdmiInSelectZoomMode,
                            (void *)&param,
                            sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
        HDMI_IN_TRACE(("%s <--- %d\n", __PRETTY_FUNCTION__, param.result));
        return param.result;
	}

    HDMI_IN_TRACE(("%s <--- dsERR_GENERAL\n", __PRETTY_FUNCTION__));
	return dsERR_GENERAL;
}

dsError_t dsHdmiInPauseAudio ()
{
    _DEBUG_ENTER();
    HDMI_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    dsError_t param;
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsHdmiInPauseAudio,
                            (void *)&param,
                            sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
        HDMI_IN_TRACE(("%s <--- %d\n", __PRETTY_FUNCTION__, param));
        return param;
	}

    HDMI_IN_TRACE(("%s <--- dsERR_GENERAL\n", __PRETTY_FUNCTION__));
	return dsERR_GENERAL;
}

dsError_t dsHdmiInResumeAudio ()
{
    _DEBUG_ENTER();
    HDMI_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    dsError_t param;
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsHdmiInResumeAudio,
                            (void *)&param,
                            sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
        HDMI_IN_TRACE(("%s <--- %d\n", __PRETTY_FUNCTION__, param));
        return param;
	}

    HDMI_IN_TRACE(("%s <--- dsERR_GENERAL\n", __PRETTY_FUNCTION__));
	return dsERR_GENERAL;
}

dsError_t dsHdmiInGetCurrentVideoMode (dsVideoPortResolution_t *resolution)
{
    _DEBUG_ENTER();
    _RETURN_IF_ERROR(resolution != NULL, dsERR_INVALID_PARAM);

    HDMI_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

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
        HDMI_IN_TRACE(("%s <--- %d\n", __PRETTY_FUNCTION__, param.result));
        return param.result;
	}

    HDMI_IN_TRACE(("%s <--- dsERR_GENERAL\n", __PRETTY_FUNCTION__));
	return dsERR_GENERAL;
}

/** @} */
/** @} */
