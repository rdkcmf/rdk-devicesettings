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


#include "dsVideoDevice.h"
#include "dsclientlogger.h"
#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include "dsError.h"
#include "dsUtl.h"
#include "dsRpc.h"
#include "dsMgr.h"
#include "iarmUtil.h"
#include "libIBus.h"
#include "libIARM.h"
#include "dsTypes.h"


dsError_t dsVideoDeviceInit()
{
    printf("<<<<< VID is initialized in Multi-App Mode >>>>>>>>\r\n");

   IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsVideoDeviceInit,
                            NULL,
                            0);
  
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		return dsERR_NONE;
	}
	return dsERR_GENERAL;
}

dsError_t dsGetVideoDevice(int index, int *handle)
{
    _DEBUG_ENTER();
    _RETURN_IF_ERROR((handle) != NULL, dsERR_INVALID_PARAM);
	
   dsVideoDeviceGetHandleParam_t param;
   param.index = index;
   param.handle = NULL;
   
   IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

   rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
							(char *)IARM_BUS_DSMGR_API_dsGetVideoDevice,
							(void *)&param,
							sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		  *handle =param.handle;
		 return dsERR_NONE;
	}
 
	return dsERR_GENERAL ;
}

dsError_t  dsSetDFC(int handle, dsVideoZoom_t dfc)
{
    _DEBUG_ENTER();
   
   dsVideoDeviceSetDFCParam_t param;
   param.handle = handle;
   param.dfc = dfc;

   IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;



   rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
							(char *)IARM_BUS_DSMGR_API_dsSetDFC,
							(void *)&param,
							sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		 return dsERR_NONE;
	}
 
	return dsERR_GENERAL ;

}



dsError_t  dsGetDFC(int handle, dsVideoZoom_t *dfc)
{
    _DEBUG_ENTER();
   
   dsVideoDeviceSetDFCParam_t param;
   param.handle = handle;
   param.dfc = dsVIDEO_ZOOM_NONE;

   IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

   rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
							(char *)IARM_BUS_DSMGR_API_dsGetDFC,
							(void *)&param,
							sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		 *dfc = param.dfc;
		 return dsERR_NONE;
	}
 
	return dsERR_GENERAL ;

}



dsError_t dsVideoDeviceTerm(void)
{
    _DEBUG_ENTER();
  
   IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
   rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsVideoDeviceTerm,
                            NULL,
                            0);
	
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		return dsERR_NONE;
	}

	return dsERR_GENERAL ;
}

dsError_t dsGetHDRCapabilities(int handle, int *capabilities)
{
	_DEBUG_ENTER();

	dsGetHDRCapabilitiesParam_t param;
	param.handle = handle;

	IARM_Result_t rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
			(char *) IARM_BUS_DSMGR_API_dsGetHDRCapabilities,
			(void *) &param,
			sizeof(param));
	
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		*capabilities = param.capabilities;
		return param.result;
	}

	return dsERR_GENERAL ;
}

dsError_t dsGetSupportedVideoCodingFormats(int handle, unsigned int * supported_formats)
{
    _DEBUG_ENTER();

    dsGetSupportedVideoCodingFormatsParam_t param;
    param.handle = handle;
	
    IARM_Result_t rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
			(char *) IARM_BUS_DSMGR_API_dsGetSupportedVideoCodingFormats,
			(void *) &param,
			sizeof(param));
	
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		*supported_formats = param.supported_formats;
		return param.result;
	}
    return dsERR_GENERAL;
}

dsError_t dsGetVideoCodecInfo(int handle, dsVideoCodingFormat_t format, dsVideoCodecInfo_t* info)
{
    _DEBUG_ENTER();

    dsGetVideoCodecInfoParam_t param;
    param.handle = handle;
    param.format = format;
    info->num_entries = 0;
	
    IARM_Result_t rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
			(char *) IARM_BUS_DSMGR_API_dsGetVideoCodecInfo,
			(void *) &param,
			sizeof(param));
	
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		*info = param.info;
        return param.result;
	}
    return dsERR_GENERAL;
}

dsError_t dsForceDisableHDRSupport(int handle, bool disable)
{
	_DEBUG_ENTER();

	dsForceDisableHDRParam_t param;
	memset(&param, 0, sizeof(param));
	param.handle = handle;
	param.disable = disable;

	IARM_Result_t rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
			(char *) IARM_BUS_DSMGR_API_dsSetForceDisableHDR,
			(void *) &param,
			sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		return param.result;
	}

	return dsERR_GENERAL ;
}
dsError_t dsEnableHDRDVSupport(int handle, bool  enable)
{
        _DEBUG_ENTER();

        dsEnableHDRDVSupportParam_t param;
        memset(&param, 0, sizeof(param));
        param.handle = handle;
        param.enable= enable;

        IARM_Result_t rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                        (char *) IARM_BUS_DSMGR_API_dsEnableHDRDVSupport,
                        (void *) &param,
                        sizeof(param));

        if (IARM_RESULT_SUCCESS == rpcRet)
        {
                return param.result;
        }

        return dsERR_GENERAL ;

}
/** @} */
/** @} */
