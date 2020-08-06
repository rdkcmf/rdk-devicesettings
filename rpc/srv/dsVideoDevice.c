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

#include <sys/types.h>
#include <stdint.h>
#include <iostream>
#include <string.h>
#include <pthread.h>
#include "dsError.h"
#include "dsUtl.h"
#include "dsTypes.h"
#include "pthread.h"
#include "libIARM.h"
#include "iarmUtil.h"
#include "libIBus.h"
#include "dsRpc.h"
#include "dsMgr.h"
#include "dsserverlogger.h"
#include "hostPersistence.hpp"
#include <dlfcn.h>
#ifdef HAS_HDMI_IN_SUPPORT
#include "dsHdmiIn.h"
#endif

static int m_isInitialized = 0;
static int m_isPlatInitialized = 0;
static pthread_mutex_t dsLock = PTHREAD_MUTEX_INITIALIZER;
static dsVideoZoom_t srv_dfc = dsVIDEO_ZOOM_FULL;
static bool force_disable_hdr = true;
bool enableHDRDVStatus = false;

#define IARM_BUS_Lock(lock) pthread_mutex_lock(&dsLock)
#define IARM_BUS_Unlock(lock) pthread_mutex_unlock(&dsLock)

IARM_Result_t _dsVideoDeviceInit(void *arg);
IARM_Result_t _dsGetVideoDevice(void *arg);
IARM_Result_t _dsSetDFC(void *arg);
IARM_Result_t _dsGetDFC(void *arg);
IARM_Result_t _dsVideoDeviceTerm(void *arg);
IARM_Result_t _dsGetHDRCapabilities(void *arg);
IARM_Result_t _dsGetSupportedVideoCodingFormats(void *arg);
IARM_Result_t _dsGetVideoCodecInfo(void *arg);
IARM_Result_t _dsForceDisableHDR(void *arg);
static bool get_HDR_DV_RFC_config();

IARM_Result_t dsVideoDeviceMgr_init()
{
   IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsVideoDeviceInit, _dsVideoDeviceInit);

	try
	{
		 std::string _ZoomSettings("Full");
		/* Get the Zoom from Persistence */
		_ZoomSettings = device::HostPersistence::getInstance().getProperty("VideoDevice.DFC", _ZoomSettings);
		if (_ZoomSettings.compare("None") == 0)
		{
			srv_dfc = dsVIDEO_ZOOM_NONE;
		}
#ifdef HAS_HDMI_IN_SUPPORT
            dsHdmiInSelectZoomMode(srv_dfc);
#endif
	}
	catch(...) 
	{
		printf("Exception in Getting the Zoom settings on Startup..... \r\n");
	}

	try
	{
		std::string _hdr_setting("false");
		_hdr_setting = device::HostPersistence::getInstance().getProperty("VideoDevice.forceHDRDisabled", _hdr_setting);
		if (_hdr_setting.compare("false") == 0)
		{
			force_disable_hdr = false;
		}
		else
		{
			force_disable_hdr = true;
			printf("HDR support in disabled configuration.\n");
		}
	}
	catch(...) 
	{
		printf("Exception in getting force-disable-HDR setting at start up.\r\n");
	}

	enableHDRDVStatus = get_HDR_DV_RFC_config();
	printf("HDR DV support status is: %s.\n", (true == enableHDRDVStatus? "enabled" : "disabled"));


    if (!m_isPlatInitialized) {
    	dsVideoDeviceInit();
    }
    /*coverity[missing_lock]  CID-19380 using Coverity Annotation to ignore error*/
    m_isPlatInitialized++;
    
   return IARM_RESULT_SUCCESS;
}

IARM_Result_t dsVideoDeviceMgr_term()
{
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsVideoDeviceInit(void *arg)
{
    IARM_BUS_Lock(lock);
   
	if (!m_isInitialized) {

		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetVideoDevice,_dsGetVideoDevice);
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetDFC,_dsSetDFC);
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetDFC,_dsGetDFC);
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsVideoDeviceTerm,_dsVideoDeviceTerm);
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetHDRCapabilities,_dsGetHDRCapabilities); 
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetSupportedVideoCodingFormats, _dsGetSupportedVideoCodingFormats); 
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetVideoCodecInfo, _dsGetVideoCodecInfo); 
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetForceDisableHDR, _dsForceDisableHDR); 
        m_isInitialized = 1;
    }

    if (!m_isPlatInitialized) {
    	dsVideoDeviceInit();
    }
    
	m_isPlatInitialized++;

	IARM_BUS_Unlock(lock);
	
	return IARM_RESULT_SUCCESS;

}

IARM_Result_t _dsGetVideoDevice(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);
    
	dsVideoDeviceGetHandleParam_t *param = (dsVideoDeviceGetHandleParam_t *)arg;
    dsGetVideoDevice(param->index, &param->handle);
    
	
    IARM_BUS_Unlock(lock);

	return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsSetDFC(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);
    
	IARM_Bus_DSMgr_EventData_t eventData;
	dsVideoDeviceSetDFCParam_t *param = (dsVideoDeviceSetDFCParam_t *)arg;

	if (param != NULL)
	{
		try
		{
			if(param->dfc == dsVIDEO_ZOOM_NONE)
			{
				printf("\n Call Zoom setting NONE\n");
				dsSetDFC(param->handle,param->dfc);
				eventData.data.dfc.zoomsettings = dsVIDEO_ZOOM_NONE;
				srv_dfc = param->dfc;
				IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,(IARM_EventId_t)IARM_BUS_DSMGR_EVENT_ZOOM_SETTINGS,(void *)&eventData, sizeof(eventData));
				device::HostPersistence::getInstance().persistHostProperty("VideoDevice.DFC","None");
			}
			else if(param->dfc == dsVIDEO_ZOOM_FULL)
			{
				printf("\n Call Zoom setting FULL\n");
				dsSetDFC(param->handle,param->dfc);
				eventData.data.dfc.zoomsettings =  dsVIDEO_ZOOM_FULL;
				srv_dfc = param->dfc;
				IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,(IARM_EventId_t)IARM_BUS_DSMGR_EVENT_ZOOM_SETTINGS,(void *)&eventData, sizeof(eventData));
				device::HostPersistence::getInstance().persistHostProperty("VideoDevice.DFC","Full");

			}
			else if(param->dfc == dsVIDEO_ZOOM_16_9_ZOOM)
			{
				printf("\n Call Zoom setting dsVIDEO_ZOOM_16_9_ZOOM\n");
				dsSetDFC(param->handle,param->dfc);
				eventData.data.dfc.zoomsettings =  dsVIDEO_ZOOM_FULL;
				srv_dfc = param->dfc;
				IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,(IARM_EventId_t)IARM_BUS_DSMGR_EVENT_ZOOM_SETTINGS,(void *)&eventData, sizeof(eventData));
				device::HostPersistence::getInstance().persistHostProperty("VideoDevice.DFC","Full");
			}
			else
			{
				printf("\n ERROR: unsupported Zoom setting %d\n", param->dfc);
			}

#ifdef HAS_HDMI_IN_SUPPORT
            dsHdmiInSelectZoomMode(srv_dfc);
#endif
		}
		catch(...) 
		{
			printf("Error in Setting the Video Resolution..... \r\n");
		}
		
	}
  
    IARM_BUS_Unlock(lock);
	return IARM_RESULT_SUCCESS;
}




IARM_Result_t _dsGetDFC(void *arg)
{
    _DEBUG_ENTER();
    
	IARM_BUS_Lock(lock);

	dsVideoDeviceSetDFCParam_t *param = (dsVideoDeviceSetDFCParam_t *)arg;
	
	if (param != NULL)
	{
		param->dfc = srv_dfc;
		INFO("The Zoom Settings value is %d \r\n",param->dfc);
	}

	IARM_BUS_Unlock(lock);
	
	return IARM_RESULT_SUCCESS;
}


IARM_Result_t _dsVideoDeviceTerm(void *arg)
{
    _DEBUG_ENTER();
    
	IARM_BUS_Lock(lock);
    
	m_isPlatInitialized --;

	if (0 == m_isPlatInitialized)
	{
		dsVideoDeviceTerm();
	}
	
    IARM_BUS_Unlock(lock);
	
	return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsGetHDRCapabilities(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);
    
    typedef dsError_t (*dsGetHDRCapabilitiesFunc_t)(int handle, int *capabilities);
    static dsGetHDRCapabilitiesFunc_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetHDRCapabilitiesFunc_t)dlsym(dllib, "dsGetHDRCapabilities");
            if (func) {
                printf("dsGetHDRCapabilities() is defined and loaded\r\n");
            }
            else {
                printf("dsGetHDRCapabilities() is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }
    dsGetHDRCapabilitiesParam_t *param = (dsGetHDRCapabilitiesParam_t *)arg;
    if((0 != func) && (false == force_disable_hdr)) {
        param->result = func(param->handle, &param->capabilities);
        if((param->capabilities & dsHDRSTANDARD_DolbyVision) && (enableHDRDVStatus == false))
        {
            param->capabilities &= ~dsHDRSTANDARD_DolbyVision;
            printf("dsGetHDRCapabilities() DolbyVision Disabled param->capabilities:%x\r\n",param->capabilities);
        }
    }
    else {
        param->capabilities = dsHDRSTANDARD_NONE;
    }

    IARM_BUS_Unlock(lock);
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsGetSupportedVideoCodingFormats(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);
    
    typedef dsError_t (*dsGetSupportedVideoCodingFormatsFunc_t)(int handle, unsigned int *supported_formats);
    static dsGetSupportedVideoCodingFormatsFunc_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetSupportedVideoCodingFormatsFunc_t)dlsym(dllib, "dsGetSupportedVideoCodingFormats");
            if (func) {
                printf("dsGetSupportedVideoCodingFormats() is defined and loaded\r\n");
            }
            else {
                printf("dsGetSupportedVideoCodingFormats() is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }
    dsGetSupportedVideoCodingFormatsParam_t *param = (dsGetSupportedVideoCodingFormatsParam_t *)arg;
    if(0 != func) {
        param->result = func(param->handle, &param->supported_formats);
    }
    else {
        param->supported_formats = 0x0; //Safe default: no formats supported.
    }

    IARM_BUS_Unlock(lock);
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsGetVideoCodecInfo(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetVideoCodecInfoFunc_t)(int handle, dsVideoCodingFormat_t codec, dsVideoCodecInfo_t * info);
    static dsGetVideoCodecInfoFunc_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetVideoCodecInfoFunc_t)dlsym(dllib, "dsGetVideoCodecInfo");
            if (func) {
                printf("dsGetVideoCodecInfo() is defined and loaded\r\n");
            }
            else {
                printf("dsGetVideoCodecInfo() is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsGetVideoCodecInfoParam_t *param = (dsGetVideoCodecInfoParam_t *)arg;
    if(0 != func) {
        param->result = func(param->handle, param->format, &param->info);
    }
    else {
        param->result = dsERR_OPERATION_NOT_SUPPORTED;
    }

    IARM_BUS_Unlock(lock);
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsForceDisableHDR(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);
    
    dsForceDisableHDRParam_t *param = (dsForceDisableHDRParam_t *)arg;
	param->result = dsERR_NONE;

	force_disable_hdr = param->disable;
	if(force_disable_hdr)
	{
		device::HostPersistence::getInstance().persistHostProperty("VideoDevice.forceHDRDisabled","true");
	}
	else
	{
		device::HostPersistence::getInstance().persistHostProperty("VideoDevice.forceHDRDisabled","false");
	}

    IARM_BUS_Unlock(lock);
    return IARM_RESULT_SUCCESS;
}

#define RFC_BUFFER_SIZE 100
static bool get_HDR_DV_RFC_config()
{
    bool is_enabled = false;
    const char * cmd = "tr181Set -g Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.DolbyVision.Enable 2>&1 1>/dev/null";
    /*Note: the above redirection is necessary because tr181 prints the value of parameter to stderr instead of stdout. */
    printf("%s: getting RFC config using command \"%s\"\n", __FUNCTION__, cmd);
    FILE * pipe = popen(cmd, "r");
    if(pipe)
    {
        char buffer[RFC_BUFFER_SIZE];
        if(NULL == fgets(buffer, RFC_BUFFER_SIZE, pipe))
        {
            printf("%s: could not parse output of command <%s>.\n", __FUNCTION__, cmd);
        }
        else
        {
            printf("%s: read output \"%s\"\n", __FUNCTION__, buffer);
            if(0 == strncasecmp(buffer, "true", 4))
            {
                is_enabled = true;
            }
        }
    }
    printf("%s: the feature is %s.\n", __FUNCTION__, (true == is_enabled? "enabled" : "disabled"));
    pclose(pipe);
    return is_enabled;
}

/** @} */
/** @} */
