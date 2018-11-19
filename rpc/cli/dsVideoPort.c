/* If not stated otherwise in this file or this component's Licenses.txt file the
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


#include "dsVideoPort.h"
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
#include "dsclientlogger.h"



dsError_t dsVideoPortInit()
{
    printf("<<<<< VOP is initialized in Multi-App Mode >>>>>>>>\r\n");

	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsVideoPortInit,
                            NULL,
                            0);
  
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		return dsERR_NONE;
	}

	return dsERR_GENERAL;
}

dsError_t dsGetVideoPort(dsVideoPortType_t type, int index, int *handle)
{
    _DEBUG_ENTER();
    _RETURN_IF_ERROR(dsVideoPortType_isValid(type), dsERR_INVALID_PARAM);
    _RETURN_IF_ERROR((handle) != NULL, dsERR_INVALID_PARAM);

	dsVideoPortGetHandleParam_t param;
    param.type = type;
    param.index = index;
    param.handle = NULL;



    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

    rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
							(char *)IARM_BUS_DSMGR_API_dsGetVideoPort,
							 &param,
							sizeof(param));

	printf("%s..%d-%d\n",__func__,param.type,param.handle);

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		*handle = param.handle;
		 return dsERR_NONE;
	}
 
	return dsERR_GENERAL ;
}


dsError_t dsIsHDCPEnabled(int handle, bool *enabled)
{
    _DEBUG_ENTER();
    
    _RETURN_IF_ERROR(enabled != NULL, dsERR_INVALID_PARAM);

    dsVideoPortIsHDCPEnabledParam_t param;
   
    param.handle = handle;
    param.enabled = false;
	


	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
    
	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
							(char *)IARM_BUS_DSMGR_API_dsIsHDCPEnabled,
							(void *)&param,
							sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		 *enabled = param.enabled;
		 return dsERR_NONE;
	}

	return dsERR_GENERAL ;
}

dsError_t dsIsVideoPortEnabled(int handle, bool *enabled)
{
    _DEBUG_ENTER();
    
    _RETURN_IF_ERROR(enabled != NULL, dsERR_INVALID_PARAM);

    dsVideoPortIsEnabledParam_t param;
   
    param.handle = handle;
    param.enabled = false;
	


	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
    
	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
							(char *)IARM_BUS_DSMGR_API_dsIsVideoPortEnabled,
							(void *)&param,
							sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		 *enabled = param.enabled;
		 return dsERR_NONE;
	}

	return dsERR_GENERAL ;
}

dsError_t dsGetHDCPStatus (int handle, dsHdcpStatus_t *status)
{
    _DEBUG_ENTER();
    _RETURN_IF_ERROR(status != NULL, dsERR_INVALID_PARAM);

    dsVideoPortGetHDCPStatus_t param;
    param.handle = handle;
    param.hdcpStatus = dsHDCP_STATUS_UNAUTHENTICATED; 

    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
    rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
							(char *)IARM_BUS_DSMGR_API_dsGetHDCPStatus,
							(void *)&param,
							sizeof(param));
    if (IARM_RESULT_SUCCESS == rpcRet)
    {
     	*status = param.hdcpStatus;
	 	return dsERR_NONE;
    }

    return dsERR_GENERAL ;
}

dsError_t dsGetHDCPProtocol (int handle, dsHdcpProtocolVersion_t *version)
{
    _DEBUG_ENTER();
    _RETURN_IF_ERROR(version != NULL, dsERR_INVALID_PARAM);

    dsVideoPortGetHDCPProtocolVersion_t param;
    param.handle = handle;
    param.protocolVersion = dsHDCP_VERSION_MAX;

    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
    rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
					(char *)IARM_BUS_DSMGR_API_dsGetHDCPProtocol,
					(void *)&param,
					sizeof(param));
    if (IARM_RESULT_SUCCESS == rpcRet)
    {
     	*version = param.protocolVersion;
     	return dsERR_NONE;
    }

    return dsERR_GENERAL ;
}

dsError_t dsGetHDCPReceiverProtocol (int handle, dsHdcpProtocolVersion_t *version)
{
    _DEBUG_ENTER();
    _RETURN_IF_ERROR(version != NULL, dsERR_INVALID_PARAM);

    dsVideoPortGetHDCPProtocolVersion_t param;
    param.handle = handle;
    param.protocolVersion = dsHDCP_VERSION_MAX;

    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
    rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
					(char *)IARM_BUS_DSMGR_API_dsGetHDCPReceiverProtocol,
					(void *)&param,
					sizeof(param));
    if (IARM_RESULT_SUCCESS == rpcRet)
    {
     	*version = param.protocolVersion;
     	return dsERR_NONE;
    }

    return dsERR_GENERAL ;
}

dsError_t dsGetHDCPCurrentProtocol (int handle, dsHdcpProtocolVersion_t *version)
{
    _DEBUG_ENTER();
    _RETURN_IF_ERROR(version != NULL, dsERR_INVALID_PARAM);

    dsVideoPortGetHDCPProtocolVersion_t param;
    param.handle = handle;
    param.protocolVersion = dsHDCP_VERSION_MAX;

    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
    rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
					(char *)IARM_BUS_DSMGR_API_dsGetHDCPCurrentProtocol,
					(void *)&param,
					sizeof(param));
    if (IARM_RESULT_SUCCESS == rpcRet)
    {
     	*version = param.protocolVersion;
     	return dsERR_NONE;
    }

    return dsERR_GENERAL ;
}
dsError_t  dsIsDisplayConnected(int handle, bool *connected)
{
    _DEBUG_ENTER();
    _RETURN_IF_ERROR(connected != NULL, dsERR_INVALID_PARAM);
   
	dsVideoPortIsDisplayConnectedParam_t param;
    
	param.handle = handle;
    param.connected = false;

	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
	

	
	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
							(char *)IARM_BUS_DSMGR_API_dsIsDisplayConnected,
							(void *)&param,
							sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		 *connected = param.connected;
		 return dsERR_NONE;
	}

	return dsERR_GENERAL ;
}

dsError_t  dsIsDisplaySurround(int handle, bool *surround)
{
    _DEBUG_ENTER();
    _RETURN_IF_ERROR(surround != NULL, dsERR_INVALID_PARAM);
   
	dsVideoPortIsDisplaySurroundParam_t param;
    
	param.handle = handle;
	param.surround = false;

	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
	

	
	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
							(char *)IARM_BUS_DSMGR_API_dsIsDisplaySurround,
							(void *)&param,
							sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		 *surround = param.surround;
		 return dsERR_NONE;
	}

	return dsERR_GENERAL ;
}

dsError_t  dsGetSurroundMode(int handle, int *surround)
{
    _DEBUG_ENTER();
    _RETURN_IF_ERROR(surround != NULL, dsERR_INVALID_PARAM);
   
	dsVideoPortGetSurroundModeParam_t param;
    
	param.handle = handle;
	param.surround = dsSURROUNDMODE_NONE;

	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
	

	
	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
							(char *)IARM_BUS_DSMGR_API_dsGetSurroundMode,
							(void *)&param,
							sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		 *surround = param.surround;
		 return dsERR_NONE;
	}

	return dsERR_GENERAL ;
}
dsError_t  dsEnableVideoPort(int handle, bool enabled)
{
    _DEBUG_ENTER();

	dsVideoPortSetEnabledParam_t param;
    param.handle = handle;
    param.enabled = enabled;


    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
							(char *)IARM_BUS_DSMGR_API_dsEnableVideoPort,
							(void *)&param,
							sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		return dsERR_NONE;
	}

	return dsERR_GENERAL ;
}


dsError_t  dsGetResolution(int handle, dsVideoPortResolution_t *resolution)
{
    _DEBUG_ENTER();
    _RETURN_IF_ERROR(resolution != NULL, dsERR_INVALID_PARAM);
 
	dsVideoPortGetResolutionParam_t param;
    
    param.handle = handle;
    param.toPersist = false;
    param.resolution = *resolution;

	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
							(char *)IARM_BUS_DSMGR_API_dsGetResolution,
							(void *)&param,
							sizeof(param));

	*resolution = param.resolution;

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		return dsERR_NONE;
	}
	return dsERR_GENERAL ;
}

dsError_t dsSetScartParameter(int handle, const char* parameter_str, const char* value_str)
{
	_DEBUG_ENTER();
	if ((value_str == NULL) || (parameter_str == NULL))
	{
		return dsERR_INVALID_PARAM;
	}

	dsScartParamParam_t param;

	memset(&param, 0, sizeof(param));
	param.handle = handle;
	param.result = dsERR_NONE;
	strncpy(param.param_bytes, parameter_str, DSSCART_PARAM_LEN_MAX -1);
	strncpy(param.value_bytes, value_str, DSSCART_VALUE_LEN_MAX - 1);
	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
							(char *)IARM_BUS_DSMGR_API_dsSetScartParameter,
							(void *)&param,
							sizeof(param));

	if ((IARM_RESULT_SUCCESS == rpcRet) && (dsERR_NONE == param.result))
	{
		return dsERR_NONE;
	}

	return dsERR_GENERAL;
}

dsError_t  dsSetResolution(int handle, dsVideoPortResolution_t *resolution)
{
    _DEBUG_ENTER();
    _RETURN_IF_ERROR(resolution != NULL, dsERR_INVALID_PARAM);
 
	dsVideoPortSetResolutionParam_t param;
    
    param.handle = handle;
    param.toPersist = true;
    param.forceCompatible = true;
    param.resolution = *resolution;

	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

	rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
							(char *)IARM_BUS_DSMGR_API_dsSetResolution,
							(void *)&param,
							sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		return dsERR_NONE;
	}
	
	return dsERR_GENERAL ;
}

dsError_t dsVideoPortTerm(void)
{
    _DEBUG_ENTER();

   IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

   rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsVideoPortTerm,
                            NULL,
                            0);
	
	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		return dsERR_NONE;
	}

	return dsERR_GENERAL ;
}

dsError_t  dsEnableHDCP(int handle, bool contentProtect, char *hdcpKey, size_t keySize)
{
    _DEBUG_ENTER();

    if ((keySize <= 0) || (keySize > HDCP_KEY_MAX_SIZE) )
    {
        return dsERR_INVALID_PARAM;
    }

    if (contentProtect && !hdcpKey) {
        return dsERR_INVALID_PARAM;
    }

    dsEnableHDCPParam_t param;
    
    memset(&param, 0, sizeof(param));
    param.handle = handle;
    param.contentProtect = contentProtect;
    param.keySize = keySize;
    param.rpcResult = dsERR_NONE;

    if (contentProtect && hdcpKey && keySize && keySize <= HDCP_KEY_MAX_SIZE) {
        memcpy(param.hdcpKey, hdcpKey, keySize);
    } 
    
    printf("IARM:CLI:dsEnableHDCP %d, %p, %d\r\n", contentProtect, hdcpKey, keySize);

    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
    rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
		    (char *)IARM_BUS_DSMGR_API_dsEnableHDCP,
		    (void *)&param,
		    sizeof(param));

	if( (IARM_RESULT_SUCCESS == rpcRet) && (dsERR_NONE == param.rpcResult))
	{
		return dsERR_NONE;
	}

	return dsERR_GENERAL ;
}

dsError_t dsIsVideoPortActive(int handle, bool *active)
{
    _DEBUG_ENTER();
    
    _RETURN_IF_ERROR(active != NULL, dsERR_INVALID_PARAM);

    dsVideoPortIsActiveParam_t param;
   
    param.handle = handle;
    param.active = false;
    param.result = dsERR_NONE;
	

	IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
    rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
							(char *)IARM_BUS_DSMGR_API_dsIsVideoPortActive,
							(void *)&param,
							sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		*active = param.active;
		return param.result;
	}

	return dsERR_GENERAL ;
}

dsError_t dsGetTVHDRCapabilities(int handle, int *capabilities)
{
	_DEBUG_ENTER();

	dsGetHDRCapabilitiesParam_t param;
	memset(&param, 0, sizeof(param));
	param.handle = handle;

	IARM_Result_t rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
			(char *) IARM_BUS_DSMGR_API_dsGetTVHDRCapabilities,
			(void *) &param,
			sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		*capabilities = param.capabilities;
		return param.result;
	}

	return dsERR_GENERAL ;
}

dsError_t dsSupportedTvResolutions(int handle, int *resolutions)
{
	_DEBUG_ENTER();

	dsSupportedResolutionParam_t param;
	memset(&param, 0, sizeof(param));
	param.handle = handle;

	IARM_Result_t rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
			(char *) IARM_BUS_DSMGR_API_dsGetSupportedTVResolution,
			(void *) &param,
			sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		*resolutions = param.resolutions;
		return param.result;
	}

	return dsERR_GENERAL ;
}

dsError_t dsSetForceDisable4KSupport(int handle, bool disable)
{
	_DEBUG_ENTER();

	dsForceDisable4KParam_t param;
	memset(&param, 0, sizeof(param));
	param.handle = handle;
	param.disable = disable;

	IARM_Result_t rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
			(char *) IARM_BUS_DSMGR_API_dsSetForceDisable4K,
			(void *) &param,
			sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		return param.result;
	}

	return dsERR_GENERAL ;
}

dsError_t dsGetForceDisable4KSupport(int handle, bool *disable)
{
	_DEBUG_ENTER();

	dsForceDisable4KParam_t param;
	memset(&param, 0, sizeof(param));
	param.handle = handle;

	IARM_Result_t rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
			(char *) IARM_BUS_DSMGR_API_dsGetForceDisable4K,
			(void *) &param,
			sizeof(param));

	if (IARM_RESULT_SUCCESS == rpcRet)
	{
		*disable = param.disable;
		return param.result;
	}

	return dsERR_GENERAL ;
}
/** @} */
/** @} */
