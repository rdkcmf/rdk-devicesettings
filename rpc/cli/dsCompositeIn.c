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
#include "dsCompositeIn.h"
#include "dsRpc.h"
#include "dsMgr.h"
#include "dsclientlogger.h"
#include "iarmUtil.h"
#include "libIARM.h"
#include "libIBus.h"

/* Enable for debug tracing */
/* #define COMPOSITE_IN_DEBUG */
#ifdef COMPOSITE_IN_DEBUG
   #define COMPOSITE_IN_TRACE( m ) printf m
#else
   #define COMPOSITE_IN_TRACE( m )
#endif

dsError_t dsCompositeInInit (void)
{
    COMPOSITE_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));
    COMPOSITE_IN_TRACE(("<<<<< COMPOSITE In is initialized in Multi-App Mode >>>>>>>>\r\n"));

    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsCompositeInInit,
                            NULL,
                            0);

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
        COMPOSITE_IN_TRACE(("%s <--- dsERR_NONE\n", __PRETTY_FUNCTION__));
		return dsERR_NONE;
	}

    COMPOSITE_IN_TRACE(("%s <--- dsERR_GENERAL\n", __PRETTY_FUNCTION__));
	return dsERR_GENERAL;
}


dsError_t dsCompositeInTerm (void)
{
    _DEBUG_ENTER();
    COMPOSITE_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

   IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
   rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsCompositeInTerm,
                            NULL,
                            0);

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
        COMPOSITE_IN_TRACE(("%s <--- dsERR_NONE\n", __PRETTY_FUNCTION__));
		return dsERR_NONE;
	}

    COMPOSITE_IN_TRACE(("%s <--- dsERR_GENERAL\n", __PRETTY_FUNCTION__));
	return dsERR_GENERAL ;
}


dsError_t dsCompositeInGetNumberOfInputs (uint8_t *pNumCompositeInputs)
{
    _DEBUG_ENTER();
    _RETURN_IF_ERROR(pNumCompositeInputs != NULL, dsERR_INVALID_PARAM);

    COMPOSITE_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    dsCompositeInGetNumberOfInputsParam_t param;
    param.numCompositeInputs = 0;

    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsCompositeInGetNumberOfInputs,
                            (void *)&param,
                            sizeof(param));

    if (IARM_RESULT_SUCCESS == rpcRet)
    {
        *pNumCompositeInputs = param.numCompositeInputs;
        COMPOSITE_IN_TRACE(("%s <--- %d\n", __PRETTY_FUNCTION__, param.result));
        return param.result;
	}

    COMPOSITE_IN_TRACE(("%s <--- dsERR_GENERAL\n", __PRETTY_FUNCTION__));
	return dsERR_GENERAL;
}

dsError_t dsCompositeInGetStatus (dsCompositeInStatus_t *pStatus)
{
    _DEBUG_ENTER();
    _RETURN_IF_ERROR(pStatus != NULL, dsERR_INVALID_PARAM);

    COMPOSITE_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    dsCompositeInGetStatusParam_t param;
    memset (&param, 0, sizeof(param));

    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsCompositeInGetStatus,
                            (void *)&param,
                            sizeof(param));

    if (IARM_RESULT_SUCCESS == rpcRet)
    {
        *pStatus = param.status;
        COMPOSITE_IN_TRACE(("%s <--- %d\n", __PRETTY_FUNCTION__, param.result));
        return param.result;
	}

    COMPOSITE_IN_TRACE(("%s <--- dsERR_GENERAL\n", __PRETTY_FUNCTION__));
	return dsERR_GENERAL;
}

dsError_t dsCompositeInSelectPort (dsCompositeInPort_t ePort)
{
    _DEBUG_ENTER();
    COMPOSITE_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    dsCompositeInSelectPortParam_t param;
    param.port = ePort;

    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsCompositeInSelectPort,
                            (void *)&param,
                            sizeof(param));

    if (IARM_RESULT_SUCCESS == rpcRet)
    {
        COMPOSITE_IN_TRACE(("%s <--- %d\n", __PRETTY_FUNCTION__, param.result));
        return param.result;
	}

    COMPOSITE_IN_TRACE(("%s <--- dsERR_GENERAL\n", __PRETTY_FUNCTION__));
	return dsERR_GENERAL;
}

dsError_t dsCompositeInScaleVideo (int32_t x, int32_t y, int32_t width, int32_t height)
{
    _DEBUG_ENTER();
    COMPOSITE_IN_TRACE(("%s ---> x=%d, y=%d, width=%d, height=%d\n", __PRETTY_FUNCTION__, x, y, width, height));

    dsCompositeInScaleVideoParam_t param;
    param.videoRect.x      = x;
    param.videoRect.y      = y;
    param.videoRect.width  = width;
    param.videoRect.height = height;
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsCompositeInScaleVideo,
                            (void *)&param,
                            sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
        COMPOSITE_IN_TRACE(("%s <--- %d\n", __PRETTY_FUNCTION__, param.result));
        return param.result;
	}

    COMPOSITE_IN_TRACE(("%s <--- dsERR_GENERAL\n", __PRETTY_FUNCTION__));
	return dsERR_GENERAL;
}


/** @} */
/** @} */