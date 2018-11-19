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


#include "dsDisplay.h"
#include "dsclientlogger.h"
#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include "dsError.h"
#include "dsUtl.h"
#include "dsRpc.h"
#include "dsMgr.h"
#include "iarmUtil.h"
#include "libIARM.h"
#include "libIBus.h"
#include "dsTypes.h"
#include "stdlib.h"
	

dsError_t dsDisplayInit()
{
   IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	printf("<<<<< VDISP is initialized in Multi-App Mode >>>>>>>>\r\n");

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsDisplayInit,
                            NULL,
                            0);
  
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		return dsERR_NONE;
	}
	return dsERR_GENERAL;
}

dsError_t dsGetDisplay(dsVideoPortType_t vType, int index, int *handle)
{
  
	 IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
	_DEBUG_ENTER();
    _RETURN_IF_ERROR((handle) != NULL, dsERR_INVALID_PARAM);

	dsDisplayGetHandleParam_t param;

    param.type = vType;
    param.index = index;
     param.handle = NULL;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
							(char *)IARM_BUS_DSMGR_API_dsGetDisplay,
							(void *)&param,
							sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		*handle = param.handle;
		 return dsERR_NONE;
	}
 
	return dsERR_GENERAL ;
  
}

dsError_t dsGetDisplayAspectRatio(int handle, dsVideoAspectRatio_t *aspect)
{
	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
	
	_DEBUG_ENTER();

	dsDisplayGetAspectRatioParam_t param;
    param.handle = handle;



   rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
							(char *)IARM_BUS_DSMGR_API_dsGetDisplayAspectRatio,
							(void *)&param,
							sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		 *aspect = param.aspectRatio;
		 return dsERR_NONE;
	}

	return dsERR_GENERAL ;

}

dsError_t dsGetEDID(int handle, dsDisplayEDID_t *edid)
{
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	_DEBUG_ENTER();

    dsDisplayGetEDIDParam_t param;

    memset(&param,0,sizeof(dsDisplayGetEDIDParam_t));
    param.handle = handle;
    
	
   rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
							(char *)IARM_BUS_DSMGR_API_dsGetEDID,
							(void *)&param,
							sizeof(param));

	memcpy(edid, &param.edid, sizeof(param.edid));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		 return dsERR_NONE;
	}

	return dsERR_GENERAL ;
}

dsError_t dsGetEDIDBytes(int handle, unsigned char **edid, int *length)
{
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	_DEBUG_ENTER();

    dsDisplayGetEDIDBytesParam_t param;

    param.handle = handle;
    
    printf("dsCLI::getEDIDBytes \r\n");
	
   rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
							(char *)IARM_BUS_DSMGR_API_dsGetEDIDBytes,
							(void *)&param,
							sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
        if (param.result == dsERR_NONE) {
            printf("dsCLI ::getEDIDBytes returns %d bytes\r\n", param.length);
            *edid = (unsigned char *)malloc(param.length);
            if (*edid) {
                memcpy(*edid, param.bytes, param.length);
                *length = param.length;
                return dsERR_NONE;
            }
            else {
                return dsERR_GENERAL;
            }
        }
        else {
            return (dsError_t)param.result;
        }
	}
    else {
        return dsERR_GENERAL;
    }
}

dsError_t dsDisplayTerm(void)
{
   _DEBUG_ENTER();

	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsDisplayTerm,
                            NULL,
                            0);
	
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		return dsERR_NONE;
	}

	return dsERR_GENERAL ;
}


/** @} */
/** @} */
