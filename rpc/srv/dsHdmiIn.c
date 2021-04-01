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




/**
* @defgroup devicesettings
* @{
* @defgroup rpc
* @{
**/


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <dlfcn.h>
#include "dsHdmiIn.h"
#include "dsRpc.h"
#include "dsTypes.h"
#include "dsserverlogger.h"
#include "dsMgr.h"

#include "iarmUtil.h"
#include "libIARM.h"
#include "libIBus.h"


#define direct_list_top(list) ((list))
#define IARM_BUS_Lock(lock) pthread_mutex_lock(&fpLock)
#define IARM_BUS_Unlock(lock) pthread_mutex_unlock(&fpLock)

static int m_isInitialized = 0;
static int m_isPlatInitialized=0;
static pthread_mutex_t fpLock = PTHREAD_MUTEX_INITIALIZER;

IARM_Result_t dsHdmiInMgr_init();
IARM_Result_t dsHdmiInMgr_term();
IARM_Result_t _dsHdmiInInit(void *arg);
IARM_Result_t _dsHdmiInTerm(void *arg);
IARM_Result_t _dsHdmiInLoadKsvs(void *arg);
IARM_Result_t _dsHdmiInGetNumberOfInputs(void *arg);
IARM_Result_t _dsHdmiInGetStatus(void *arg);
IARM_Result_t _dsHdmiInSelectPort(void *arg);
IARM_Result_t _dsHdmiInToggleHotPlug(void *arg);
IARM_Result_t _dsHdmiInLoadEdidData(void *arg);
IARM_Result_t _dsHdmiInSetRepeater(void *arg);
IARM_Result_t _dsHdmiInScaleVideo(void *arg);
IARM_Result_t _dsHdmiInSelectZoomMode(void *arg);
IARM_Result_t _dsHdmiInPauseAudio(void *arg);
IARM_Result_t _dsHdmiInResumeAudio(void *arg);
IARM_Result_t _dsHdmiInGetCurrentVideoMode(void *arg);
IARM_Result_t _dsGetEDIDBytesInfo (void *arg);
IARM_Result_t _dsGetHDMISPDInfo (void *arg);

void _dsHdmiInConnectCB(dsHdmiInPort_t port, bool isPortConnected);
void _dsHdmiInSignalChangeCB(dsHdmiInPort_t port, dsHdmiInSignalStatus_t sigStatus);
void _dsHdmiInStatusChangeCB(dsHdmiInStatus_t inputStatus);

static dsHdmiInCap_t hdmiInCap_gs;

#include <iostream>
#include "hostPersistence.hpp"
#include <sstream>


using namespace std;


static dsError_t isHdmiARCPort (int iPort, bool* isArcEnabled) {
    dsError_t eRet = dsERR_GENERAL; 

    typedef bool (*dsIsHdmiARCPort_t)(int iPortArg);
    static dsIsHdmiARCPort_t dsIsHdmiARCPortFunc = 0;
    if (dsIsHdmiARCPortFunc == 0) {
       void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
       if (dllib) {
            dsIsHdmiARCPortFunc = (dsIsHdmiARCPort_t) dlsym(dllib, "dsIsHdmiARCPort");
            if(dsIsHdmiARCPortFunc == 0) {
                printf("%s:%d dsIsHdmiARCPort (int) is not defined %s\r\n", __FUNCTION__,__LINE__, dlerror());
                eRet = dsERR_GENERAL;
            }
            else {
                printf("%s:%d dsIsHdmiARCPort dsIsHdmiARCPortFunc loaded\r\n", __FUNCTION__,__LINE__);
            }
            dlclose(dllib);
        }
        else {
            printf("%s:%d dsIsHdmiARCPort  Opening RDK_DSHAL_NAME[%s] failed %s\r\n", 
                   __FUNCTION__,__LINE__, RDK_DSHAL_NAME, dlerror());  //CID 168096 - Print Args
            eRet = dsERR_GENERAL;
        }
    }
    if (0 != dsIsHdmiARCPortFunc) { 
        *isArcEnabled = dsIsHdmiARCPortFunc (iPort);
        printf("%s: dsIsHdmiARCPort port %d isArcEnabled:%d\r\n", __FUNCTION__, iPort, *isArcEnabled);
    }
    else {
        printf("%s: dsIsHdmiARCPort  dsIsHdmiARCPortFunc = %p\n", dsIsHdmiARCPortFunc);
    }
    return eRet;
}

static dsError_t getEDIDBytesInfo (int iHdmiPort, unsigned char **edid, int *length) {
    dsError_t eRet = dsERR_GENERAL;
    typedef dsError_t (*dsGetEDIDBytesInfo_t)(int iHdmiPort, unsigned char **edid, int *length);
    static dsGetEDIDBytesInfo_t dsGetEDIDBytesInfoFunc = 0;
    if (dsGetEDIDBytesInfoFunc == 0) {
       void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
       if (dllib) {
            dsGetEDIDBytesInfoFunc = (dsGetEDIDBytesInfo_t) dlsym(dllib, "dsGetEDIDBytesInfo");
            if(dsGetEDIDBytesInfoFunc == 0) {
                printf("%s:%d dsGetEDIDBytesInfo (int) is not defined %s\r\n", __FUNCTION__,__LINE__, dlerror());
                eRet = dsERR_GENERAL;
            }
            else {
                printf("%s:%d dsGetEDIDBytesInfoFunc loaded\r\n", __FUNCTION__,__LINE__);
            }
            dlclose(dllib);
        }
        else {
            printf("%s:%d dsGetEDIDBytesInfo  Opening RDK_DSHAL_NAME [%] failed %s\r\n",
                   __FUNCTION__,__LINE__, RDK_DSHAL_NAME, dlerror());
            eRet = dsERR_GENERAL;
        }
    }
    if (0 != dsGetEDIDBytesInfoFunc) {
        eRet = dsGetEDIDBytesInfoFunc (iHdmiPort, edid, length);
        printf("[srv] %s: dsGetEDIDBytesInfoFunc eRet: %d data len: %d \r\n", __FUNCTION__,eRet, *length);
    }
    else {
        printf("%s:  dsIsHdmiARCPortFunc = %p\n", __FUNCTION__, dsGetEDIDBytesInfoFunc);
    }
    return eRet;
}

static dsError_t getHDMISPDInfo (int iHdmiPort, unsigned char **spd) {
    dsError_t eRet = dsERR_GENERAL;
    typedef dsError_t (*dsGetHDMISPDInfo_t)(int iHdmiPort, unsigned char **data);
    static dsGetHDMISPDInfo_t dsGetHDMISPDInfoFunc = 0;
    if (dsGetHDMISPDInfoFunc == 0) {
       void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
       if (dllib) {
            dsGetHDMISPDInfoFunc = (dsGetHDMISPDInfo_t) dlsym(dllib, "dsGetHDMISPDInfo");
            if(dsGetHDMISPDInfoFunc == 0) {
                printf("%s:%d dsGetHDMISPDInfo (int) is not defined %s\r\n", __FUNCTION__,__LINE__, dlerror());
                eRet = dsERR_GENERAL;
            }
            else {
                printf("%s:%d dsGetHDMISPDInfoFunc loaded\r\n", __FUNCTION__,__LINE__);
            }
            dlclose(dllib);
        }
        else {
            printf("%s:%d dsGetHDMISPDInfo  Opening RDK_DSHAL_NAME [%s] failed %s\r\n",
                   __FUNCTION__,__LINE__, RDK_DSHAL_NAME, dlerror());
            eRet = dsERR_GENERAL;
        }
    }
    if (0 != dsGetHDMISPDInfoFunc) {
        eRet = dsGetHDMISPDInfoFunc (iHdmiPort, spd);
        printf("[srv] %s: dsGetHDMISPDInfoFunc eRet: %d \r\n", __FUNCTION__,eRet);
    }
    else {
        printf("%s:  dsGetHDMISPDInfoFunc = %p\n", __FUNCTION__, dsGetHDMISPDInfoFunc);
    }
    return eRet;
}

IARM_Result_t dsHdmiInMgr_init()
{
    _dsHdmiInInit(NULL);

    IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsHdmiInInit, _dsHdmiInInit);

    return IARM_RESULT_SUCCESS;
}

IARM_Result_t dsHdmiInMgr_term()
{
    _dsHdmiInTerm(NULL);
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsHdmiInInit(void *arg)
{
    printf("%s:%d ---> m_isInitialized=%d, m_isPlatInitialized=%d \n",
                   __PRETTY_FUNCTION__,__LINE__, m_isInitialized, m_isPlatInitialized);

    IARM_BUS_Lock(lock);

#ifdef HAS_HDMI_IN_SUPPORT
    if (!m_isPlatInitialized)
    {
        /* Nexus init, if any here */
        dsError_t eError = dsHdmiInInit();
    }
    m_isPlatInitialized++;
#endif

    if (!m_isInitialized)
    {
#ifdef HAS_HDMI_IN_SUPPORT
        dsHdmiInRegisterConnectCB(_dsHdmiInConnectCB);

        typedef dsError_t (*dsHdmiInRegisterSignalChangeCB_t)(dsHdmiInSignalChangeCB_t CBFunc);
        static dsHdmiInRegisterSignalChangeCB_t signalChangeCBFunc = 0;
        if (signalChangeCBFunc == 0) {
            void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
            if (dllib) {
                signalChangeCBFunc = (dsHdmiInRegisterSignalChangeCB_t) dlsym(dllib, "dsHdmiInRegisterSignalChangeCB");
                if(signalChangeCBFunc == 0) {
                    printf("dsHdmiInRegisterSignalChangeCB(dsHdmiInSignalChangeCB_t) is not defined\r\n");
                }
                dlclose(dllib);
            }
            else {
                printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
            }
        }

        if(signalChangeCBFunc) {
             signalChangeCBFunc(_dsHdmiInSignalChangeCB);
        }

        typedef dsError_t (*dsHdmiInRegisterStatusChangeCB_t)(dsHdmiInStatusChangeCB_t CBFunc);
        static dsHdmiInRegisterStatusChangeCB_t statusChangeCBFunc = 0;
        if (statusChangeCBFunc == 0) {
            void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
            if (dllib) {
                statusChangeCBFunc = (dsHdmiInRegisterStatusChangeCB_t) dlsym(dllib, "dsHdmiInRegisterStatusChangeCB");
                if(statusChangeCBFunc == 0) {
                    printf("dsHdmiInRegisterStatusChangeCB(dsHdmiInStatusChangeCB_t) is not defined\r\n");
                }
                dlclose(dllib);
            }
            else {
                printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
            }
        }

        if(statusChangeCBFunc) {
             statusChangeCBFunc(_dsHdmiInStatusChangeCB);
        }

#endif
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsHdmiInTerm,                  _dsHdmiInTerm);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsHdmiInGetNumberOfInputs,     _dsHdmiInGetNumberOfInputs);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsHdmiInGetStatus,             _dsHdmiInGetStatus);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsHdmiInSelectPort,            _dsHdmiInSelectPort);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsHdmiInScaleVideo,            _dsHdmiInScaleVideo);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsHdmiInSelectZoomMode,        _dsHdmiInSelectZoomMode);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsHdmiInPauseAudio,            _dsHdmiInPauseAudio);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsHdmiInResumeAudio,           _dsHdmiInResumeAudio);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsHdmiInGetCurrentVideoMode,   _dsHdmiInGetCurrentVideoMode);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetEDIDBytesInfo,              _dsGetEDIDBytesInfo);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetHDMISPDInfo,              _dsGetHDMISPDInfo);

        int itr = 0;
        bool isARCCapable = false;
        for (itr = 0; itr < dsHDMI_IN_PORT_MAX; itr++) {
            isARCCapable = false;
            isHdmiARCPort (itr, &isARCCapable);
            hdmiInCap_gs.isPortArcCapable[itr] = isARCCapable; 
        }
        m_isInitialized = 1;
    }

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}


IARM_Result_t _dsHdmiInTerm(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);
#ifdef HAS_HDMI_IN_SUPPORT
    if (m_isPlatInitialized)
    {
        m_isPlatInitialized--;
        if (!m_isPlatInitialized)
        {
            dsHdmiInTerm();
        }
    }
#endif
    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}


IARM_Result_t _dsHdmiInGetNumberOfInputs(void *arg)
{
    _DEBUG_ENTER();

    dsHdmiInGetNumberOfInputsParam_t *param = (dsHdmiInGetNumberOfInputsParam_t *)arg;

    IARM_BUS_Lock(lock);

#ifdef HAS_HDMI_IN_SUPPORT
    param->result = dsHdmiInGetNumberOfInputs(&param->numHdmiInputs);
#else
    param->result = dsERR_GENERAL;
    #endif
    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsHdmiInGetStatus(void *arg)
{
    _DEBUG_ENTER();

    dsHdmiInGetStatusParam_t *param= (dsHdmiInGetStatusParam_t *)arg;

    IARM_BUS_Lock(lock);

#ifdef HAS_HDMI_IN_SUPPORT
    param->result = dsHdmiInGetStatus(&param->status);
#else
    param->result = dsERR_GENERAL;
#endif

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsHdmiInSelectPort(void *arg)
{
    _DEBUG_ENTER();

    dsHdmiInSelectPortParam_t *param = (dsHdmiInSelectPortParam_t *)arg;

    IARM_BUS_Lock(lock);

#ifdef HAS_HDMI_IN_SUPPORT
    param->result = dsHdmiInSelectPort(param->port);
#else
    param->result = dsERR_GENERAL;
#endif

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsHdmiInScaleVideo(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);
    dsHdmiInScaleVideoParam_t *param = (dsHdmiInScaleVideoParam_t *)arg;

#ifdef HAS_HDMI_IN_SUPPORT
    param->result = dsHdmiInScaleVideo(param->videoRect.x, param->videoRect.y, param->videoRect.width, param->videoRect.height);
#else
    param->result = dsERR_GENERAL;
#endif

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsHdmiInSelectZoomMode(void *arg)
{
    _DEBUG_ENTER();
    dsHdmiInSelectZoomModeParam_t *param = (dsHdmiInSelectZoomModeParam_t *)arg;

#ifdef HAS_HDMI_IN_SUPPORT
    param->result = dsHdmiInSelectZoomMode(param->zoomMode);
#else
    param->result = dsERR_GENERAL;
#endif

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsHdmiInPauseAudio(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);
    dsError_t *param = (dsError_t *)arg;

#ifdef HAS_HDMI_IN_SUPPORT
    *param = dsHdmiInPauseAudio();
#else
    *param = dsERR_GENERAL;
#endif

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsHdmiInResumeAudio(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);
    dsError_t *param = (dsError_t *)arg;

#ifdef HAS_HDMI_IN_SUPPORT
    *param = dsHdmiInResumeAudio();
#else
    *param = dsERR_GENERAL;
#endif

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsHdmiInGetCurrentVideoMode(void *arg)
{
    _DEBUG_ENTER();

    dsHdmiInGetResolutionParam_t *param = (dsHdmiInGetResolutionParam_t *)arg;

    IARM_BUS_Lock(lock);

#ifdef HAS_HDMI_IN_SUPPORT
    param->result = dsHdmiInGetCurrentVideoMode(&param->resolution);
#else
    param->result = dsERR_GENERAL;
#endif

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}

void _dsHdmiInConnectCB(dsHdmiInPort_t port, bool isPortConnected)
{
    IARM_Bus_DSMgr_EventData_t hdmi_in_hpd_eventData;
    __TIMESTAMP(); 
    printf("%s:%d - HDMI In hotplug update!!!!!!..Port: %d, isPort: %d\r\n",__PRETTY_FUNCTION__,__LINE__, port, isPortConnected);
    hdmi_in_hpd_eventData.data.hdmi_in_connect.port = port;
    hdmi_in_hpd_eventData.data.hdmi_in_connect.isPortConnected = isPortConnected;
			
    IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,
	                        (IARM_EventId_t)IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG,
	                        (void *)&hdmi_in_hpd_eventData, 
	                        sizeof(hdmi_in_hpd_eventData));
           
}

void _dsHdmiInSignalChangeCB(dsHdmiInPort_t port, dsHdmiInSignalStatus_t sigStatus)
{
    IARM_Bus_DSMgr_EventData_t hdmi_in_sigStatus_eventData;
    __TIMESTAMP();
    printf("%s:%d - HDMI In signal status change update!!!!!! Port: %d, Signal Status: %d\r\n", __PRETTY_FUNCTION__,__LINE__,port, sigStatus);
    hdmi_in_sigStatus_eventData.data.hdmi_in_sig_status.port = port;
    hdmi_in_sigStatus_eventData.data.hdmi_in_sig_status.status = sigStatus;

    IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,
			        (IARM_EventId_t)IARM_BUS_DSMGR_EVENT_HDMI_IN_SIGNAL_STATUS,
			        (void *)&hdmi_in_sigStatus_eventData,
			        sizeof(hdmi_in_sigStatus_eventData));

}

void _dsHdmiInStatusChangeCB(dsHdmiInStatus_t inputStatus)
{
    IARM_Bus_DSMgr_EventData_t hdmi_in_status_eventData;
    __TIMESTAMP();
    printf("%s:%d - HDMI In status change update!!!!!! Port: %d, isPresented: %d\r\n", __PRETTY_FUNCTION__,__LINE__, inputStatus.activePort, inputStatus.isPresented);
    hdmi_in_status_eventData.data.hdmi_in_status.port = inputStatus.activePort;
    hdmi_in_status_eventData.data.hdmi_in_status.isPresented = inputStatus.isPresented;

    IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,
                                (IARM_EventId_t)IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS,
                                (void *)&hdmi_in_status_eventData,
                                sizeof(hdmi_in_status_eventData));

}

IARM_Result_t _dsGetEDIDBytesInfo (void *arg) {
    dsError_t eRet = dsERR_GENERAL;
    dsGetEDIDBytesInfoParam_t *param = (dsGetEDIDBytesInfoParam_t *) arg;
    memset (param->edid, '\0', MAX_EDID_BYTES_LEN);
    unsigned char *edidArg = NULL;
    IARM_BUS_Lock(lock);
    eRet = getEDIDBytesInfo (param->iHdmiPort, (unsigned char **)(&edidArg), &(param->length));
    memcpy(param->edid, edidArg, param->length);
    param->result = eRet;
    IARM_BUS_Unlock(lock);
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsGetHDMISPDInfo(void *arg)
{
    _DEBUG_ENTER();
    printf("%s:%d [srv] _dsGetHDMISPDInfo \n", __PRETTY_FUNCTION__,__LINE__);

    dsGetHDMISPDInfoParam_t *param = (dsGetHDMISPDInfoParam_t *)arg;

    IARM_BUS_Lock(lock);

    memset (param->spdInfo, '\0', sizeof(struct dsSpd_infoframe_st));
    unsigned char *spdArg = NULL;
    param->result = getHDMISPDInfo(param->iHdmiPort, (unsigned char **)(&spdArg));
    printf("[srv] %s: dsGetHDMISPDInfo eRet: %d\r\n", __FUNCTION__, param->result);
    if (spdArg != NULL) {
        memcpy(param->spdInfo, spdArg, sizeof(struct dsSpd_infoframe_st));
    }

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}

/** @} */
/** @} */
