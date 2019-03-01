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

#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "dsError.h"
#include "dsUtl.h"
#include "dsTypes.h"
#include "pthread.h"
#include <pthread.h>
#include "libIARM.h"
#include "libIBus.h"
#include "iarmUtil.h"
#include "dsRpc.h"
#include "dsMgr.h"
#include "dsserverlogger.h"
#include "dsVideoPort.h"
#include "dsVideoPortSettings.h"
#include "dsVideoResolutionSettings.h"

static int m_isInitialized = 0;
static int m_isPlatInitialized = 0;
static pthread_mutex_t dsLock = PTHREAD_MUTEX_INITIALIZER;

#define NULL_HANDLE 0
#define IARM_BUS_Lock(lock) pthread_mutex_lock(&dsLock)
#define IARM_BUS_Unlock(lock) pthread_mutex_unlock(&dsLock)

IARM_Result_t _dsDisplayInit(void *arg);
IARM_Result_t _dsGetDisplay(void *arg);
IARM_Result_t _dsGetDisplayAspectRatio(void *arg);
IARM_Result_t _dsGetEDID(void *arg);
IARM_Result_t _dsGetEDIDBytes(void *arg);
IARM_Result_t _dsDisplayTerm(void *arg);
void _dsDisplayEventCallback(int handle, dsDisplayEvent_t event, void *eventData);
static void  filterEDIDResolution(int handle, dsDisplayEDID_t *edid);
static dsVideoPortType_t _GetDisplayPortType(int handle);

IARM_Result_t dsDisplayMgr_init()
{
	
	if (!m_isPlatInitialized) {
    	/* Nexus init, if any here */
    	dsDisplayInit();
    }
	/*coverity[missing_lock]  CID-19379 using Coverity Annotation to ignore error*/
	m_isPlatInitialized++;

	IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsDisplayInit, _dsDisplayInit);
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t dsDisplayMgr_term()
{
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsDisplayInit(void *arg)
{
    IARM_BUS_Lock(lock);

    if (!m_isInitialized) {

		/* Register appropriate dsRegisterDisplayEventCallback here*/	
        dsRegisterDisplayEventCallback(NULL,_dsDisplayEventCallback);
		
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetDisplay,_dsGetDisplay);
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetDisplayAspectRatio,_dsGetDisplayAspectRatio);
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetEDID,_dsGetEDID);
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetEDIDBytes,_dsGetEDIDBytes);
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsDisplayTerm,_dsDisplayTerm);

		m_isInitialized = 1;
    }

    if (!m_isPlatInitialized) {
    	/* Nexus init, if any here */
    	dsDisplayInit();
    }
	m_isPlatInitialized++;
    IARM_BUS_Unlock(lock);
	return IARM_RESULT_SUCCESS;

}

IARM_Result_t _dsGetDisplay(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);
    
	dsDisplayGetHandleParam_t *param = (dsDisplayGetHandleParam_t *)arg;
    dsGetDisplay(param->type, param->index, &param->handle);
    
	IARM_BUS_Unlock(lock);
	
	return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsGetDisplayAspectRatio(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);
    
	dsDisplayGetAspectRatioParam_t *param = (dsDisplayGetAspectRatioParam_t *)arg;
    dsGetDisplayAspectRatio(param->handle, &param->aspectRatio);
    
	
	IARM_BUS_Unlock(lock);
	
	return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsGetEDID(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);
    
	dsDisplayGetEDIDParam_t *param = (dsDisplayGetEDIDParam_t *)arg;
    dsVideoPortType_t _VPortType;

    dsGetEDID(param->handle, &param->edid);
    
    filterEDIDResolution(param->handle, &param->edid);
	
	IARM_BUS_Unlock(lock);
	
	return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsGetEDIDBytes(void *arg)
{

#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    printf("dsSRV::getEDIDBytes \r\n");

    typedef dsError_t (*dsGetEDIDBytes_t)(int handle, unsigned char **edid, int *length);
    static dsGetEDIDBytes_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetEDIDBytes_t) dlsym(dllib, "dsGetEDIDBytes");
            if (func) {
                printf("dsGetEDIDBytes(void) is defined and loaded\r\n");
            }   
            else {
                printf("dsGetEDIDBytes(void) is not defined\r\n");
            }   
            dlclose(dllib);
        }   
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }   
    }   

    dsDisplayGetEDIDBytesParam_t *param = (dsDisplayGetEDIDBytesParam_t *)arg;

    if (func != 0) {
        unsigned char *edid = NULL;
        int length = 0;
        dsError_t ret = func(param->handle, &edid, &length);
        if (ret == dsERR_NONE && length <= 1024) {
            printf("dsSRV ::getEDIDBytes returns %d bytes\r\n", length);
            memcpy(param->bytes, edid, length);
            param->length = length;
            free(edid);
        }
        param->result = ret;
    }
    else {
        param->result = dsERR_OPERATION_NOT_SUPPORTED;
    }

    IARM_BUS_Unlock(lock);
	
	return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsDisplayTerm(void *arg)
{
    _DEBUG_ENTER();
    
	IARM_BUS_Lock(lock);

    m_isPlatInitialized--;
	
	if (0 == m_isPlatInitialized)
	{
		dsDisplayTerm();
	}

    IARM_BUS_Unlock(lock);
	return IARM_RESULT_SUCCESS;
}


/*HDMI Call back */
void _dsDisplayEventCallback(int handle, dsDisplayEvent_t event, void *eventData)
{
	IARM_Bus_DSMgr_EventData_t _eventData;
    IARM_Bus_DSMgr_EventId_t _eventId;


	if (NULL_HANDLE == handle)
	{
		printf("Err:HDMI Hot plug back has NULL Handle... !!!!!!..\r\n");
	}
	switch(event)
	{
		case dsDISPLAY_EVENT_CONNECTED:
			__TIMESTAMP();printf("connecting HDMI to display !!!!!!..\r\n");
			_eventData.data.hdmi_hpd.event =  dsDISPLAY_EVENT_CONNECTED;
            _eventId = IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG;
	        break;

		case dsDISPLAY_EVENT_DISCONNECTED:
			__TIMESTAMP();printf("Disconnecting HDMI from display !!!!!!!! ..\r\n");
			_eventData.data.hdmi_hpd.event =  dsDISPLAY_EVENT_DISCONNECTED ;
            _eventId = IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG;
            break;

        case dsDISPLAY_RXSENSE_ON:
            __TIMESTAMP();printf("Rx Sense Status ON !!!!!!!! ..\r\n");
            _eventData.data.hdmi_rxsense.status =  dsDISPLAY_RXSENSE_ON ;
            _eventId = IARM_BUS_DSMGR_EVENT_RX_SENSE;
            break;

        case dsDISPLAY_RXSENSE_OFF:
            __TIMESTAMP();printf("Rx Sense Status OFF !!!!!!!! ..\r\n");
            _eventData.data.hdmi_rxsense.status =  dsDISPLAY_RXSENSE_OFF ;
            _eventId = IARM_BUS_DSMGR_EVENT_RX_SENSE;
            break;    
                
        case dsDISPLAY_HDCPPROTOCOL_CHANGE:
             __TIMESTAMP();printf("HDCP Protocol Version Change !!!!!!!! ..\r\n");
             _eventId = IARM_BUS_DSMGR_EVENT_HDCP_STATUS;
             break;

        default:
			printf("Error: Unsupported event in _dsHdmiCallback...\r\n");
			return;
			
	}
    IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,(IARM_EventId_t)_eventId,(void *)&_eventData, sizeof(_eventData));
}

static void filterEDIDResolution(int handle, dsDisplayEDID_t *edid)
{
    dsVideoPortResolution_t *edidResn = NULL;
    dsVideoPortResolution_t *presolution = NULL;
    dsDisplayEDID_t edidData;
    dsVideoPortType_t _VPortType = _GetDisplayPortType(handle);
    int numOfSupportedResolution = 0;

    if(_VPortType == dsVIDEOPORT_TYPE_HDMI)
    { 
        printf("EDID for HDMI Port\r\n");    
        size_t iCount = dsUTL_DIM(kResolutions);
        
        /*Initialize the struct*/
        memset(&edidData,0,sizeof(edidData));
        /*Copy the content */
        memcpy(&edidData,edid,sizeof(edidData));
        
        printf("[DsMgr] Total Resolution Count from HAL: %d\r\n",edid->numOfSupportedResolution);
        edid->numOfSupportedResolution = 0;
        for (size_t i = 0; i < iCount; i++)
        {
            presolution = &kResolutions[i];
            for (size_t j = 0; j < edidData.numOfSupportedResolution; j++)
            {    
                edidResn = &(edidData.suppResolutionList[j]);
                
                if (0 == (strcmp(presolution->name,edidResn->name)))
                {
                    edid->suppResolutionList[edid->numOfSupportedResolution] = kResolutions[i];
                    edid->numOfSupportedResolution++;
                    numOfSupportedResolution++;
                    printf("[DsMgr] presolution->name : %s, resolution count : %d\r\n",presolution->name,numOfSupportedResolution);
                }
            }    
        }    
    }
    else
    {
        printf("EDID for Non HDMI Port\r\n");            
    }
}

static dsVideoPortType_t _GetDisplayPortType(int handle)
{
    int numPorts,i;
    int halhandle = 0;
    
    numPorts = dsUTL_DIM(kSupportedPortTypes);
    for(i=0; i< numPorts; i++)
    {
        dsGetDisplay(kPorts[i].id.type, kPorts[i].id.index, &halhandle);
        if (handle == halhandle)
        {
            return kPorts[i].id.type;
        }
    }
    printf("Error: The Requested Display is not part of Platform Port Configuration \r\n");
    return dsVIDEOPORT_TYPE_MAX;
}

/** @} */
/** @} */
