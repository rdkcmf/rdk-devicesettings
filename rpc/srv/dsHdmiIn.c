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

void _dsHdmiInConnectCB(dsHdmiInPort_t port, bool isPortConnected);

#include <iostream>
#include "hostPersistence.hpp"
#include <sstream>

/* Enable for debug tracing */
/* #define HDMI_IN_DEBUG */
#ifdef HDMI_IN_DEBUG
   #define HDMI_IN_TRACE( m ) printf m
#else
   #define HDMI_IN_TRACE( m )
#endif

using namespace std;


IARM_Result_t dsHdmiInMgr_init()
{
    HDMI_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    HDMI_IN_TRACE(("%s - invoking _dsHdmiInInit()\n", __PRETTY_FUNCTION__));
    _dsHdmiInInit(NULL);

    HDMI_IN_TRACE(("%s - invoking IARM_Bus_RegisterCall()\n", __PRETTY_FUNCTION__));
	IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsHdmiInInit, _dsHdmiInInit);

    HDMI_IN_TRACE(("%s <-- \n", __PRETTY_FUNCTION__));

	return IARM_RESULT_SUCCESS;
}

IARM_Result_t dsHdmiInMgr_term()
{
    HDMI_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    HDMI_IN_TRACE(("%s - invoking _dsHdmiInTerm()\n", __PRETTY_FUNCTION__));
    _dsHdmiInTerm(NULL);

    HDMI_IN_TRACE(("%s <-- \n", __PRETTY_FUNCTION__));
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsHdmiInInit(void *arg)
{
    HDMI_IN_TRACE(("%s ---> m_isInitialized=%d, m_isPlatInitialized=%d \n",
                   __PRETTY_FUNCTION__, m_isInitialized, m_isPlatInitialized));

    IARM_BUS_Lock(lock);

    INFO("<<<<< called _dsHdmiInInit >>>>>>>>\r\n");

#ifdef HAS_HDMI_IN_SUPPORT
    if (!m_isPlatInitialized)
    {
        /* Nexus init, if any here */
        HDMI_IN_TRACE(("%s - invoking dsHdmiInInit()\n", __PRETTY_FUNCTION__));
        dsError_t eError = dsHdmiInInit();
    }
    m_isPlatInitialized++;
#endif

    if (!m_isInitialized)
    {
#ifdef HAS_HDMI_IN_SUPPORT
        HDMI_IN_TRACE(("%s - invoking dsHdmiInRegisterConnectCB()\n", __PRETTY_FUNCTION__));
        dsHdmiInRegisterConnectCB(_dsHdmiInConnectCB);
#endif
        HDMI_IN_TRACE(("%s - invoking IARM_Bus_RegisterCall()\n", __PRETTY_FUNCTION__));
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsHdmiInTerm,                  _dsHdmiInTerm);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsHdmiInGetNumberOfInputs,     _dsHdmiInGetNumberOfInputs);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsHdmiInGetStatus,             _dsHdmiInGetStatus);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsHdmiInSelectPort,            _dsHdmiInSelectPort);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsHdmiInScaleVideo,            _dsHdmiInScaleVideo);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsHdmiInSelectZoomMode,        _dsHdmiInSelectZoomMode);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsHdmiInPauseAudio,            _dsHdmiInPauseAudio);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsHdmiInResumeAudio,           _dsHdmiInResumeAudio);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsHdmiInGetCurrentVideoMode,   _dsHdmiInGetCurrentVideoMode);

        m_isInitialized = 1;
    }

    IARM_BUS_Unlock(lock);

    HDMI_IN_TRACE(("%s <-- \n", __PRETTY_FUNCTION__));
    return IARM_RESULT_SUCCESS;
}


IARM_Result_t _dsHdmiInTerm(void *arg)
{
    _DEBUG_ENTER();
    HDMI_IN_TRACE(("%s ---> m_isPlatInitialized=%d\n", __PRETTY_FUNCTION__, m_isPlatInitialized));

    IARM_BUS_Lock(lock);
#ifdef HAS_HDMI_IN_SUPPORT
    if (m_isPlatInitialized)
    {
        m_isPlatInitialized--;
        if (!m_isPlatInitialized)
        {
            HDMI_IN_TRACE(("%s - invoking dsHdmiInTerm()\n", __PRETTY_FUNCTION__));
            dsHdmiInTerm();
        }
    }
#endif
    IARM_BUS_Unlock(lock);

    HDMI_IN_TRACE(("%s <-- \n", __PRETTY_FUNCTION__));
    return IARM_RESULT_SUCCESS;
}


IARM_Result_t _dsHdmiInGetNumberOfInputs(void *arg)
{
    _DEBUG_ENTER();
    HDMI_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    dsHdmiInGetNumberOfInputsParam_t *param = (dsHdmiInGetNumberOfInputsParam_t *)arg;

    IARM_BUS_Lock(lock);

#ifdef HAS_HDMI_IN_SUPPORT
    param->result = dsHdmiInGetNumberOfInputs(&param->numHdmiInputs);
#else
    param->result = dsERR_GENERAL;
    #endif
    IARM_BUS_Unlock(lock);

    HDMI_IN_TRACE(("%s <-- \n", __PRETTY_FUNCTION__));
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsHdmiInGetStatus(void *arg)
{
    _DEBUG_ENTER();
    HDMI_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    dsHdmiInGetStatusParam_t *param= (dsHdmiInGetStatusParam_t *)arg;

    IARM_BUS_Lock(lock);

#ifdef HAS_HDMI_IN_SUPPORT
    param->result = dsHdmiInGetStatus(&param->status);
#else
    param->result = dsERR_GENERAL;
#endif

    IARM_BUS_Unlock(lock);

    HDMI_IN_TRACE(("%s <-- \n", __PRETTY_FUNCTION__));
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsHdmiInSelectPort(void *arg)
{
    _DEBUG_ENTER();
    HDMI_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    dsHdmiInSelectPortParam_t *param = (dsHdmiInSelectPortParam_t *)arg;

    IARM_BUS_Lock(lock);

#ifdef HAS_HDMI_IN_SUPPORT
    param->result = dsHdmiInSelectPort(param->port);
#else
    param->result = dsERR_GENERAL;
#endif

    IARM_BUS_Unlock(lock);

    HDMI_IN_TRACE(("%s <-- \n", __PRETTY_FUNCTION__));
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsHdmiInScaleVideo(void *arg)
{
    _DEBUG_ENTER();
    HDMI_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    IARM_BUS_Lock(lock);
    dsHdmiInScaleVideoParam_t *param = (dsHdmiInScaleVideoParam_t *)arg;
    HDMI_IN_TRACE(("%s - x=%d, y=%d, width=%d, height=%d \n",
                   __PRETTY_FUNCTION__, param->videoRect.x, param->videoRect.y, param->videoRect.width, param->videoRect.height));

#ifdef HAS_HDMI_IN_SUPPORT
    param->result = dsHdmiInScaleVideo(param->videoRect.x, param->videoRect.y, param->videoRect.width, param->videoRect.height);
#else
    param->result = dsERR_GENERAL;
#endif

    IARM_BUS_Unlock(lock);

    HDMI_IN_TRACE(("%s <-- \n", __PRETTY_FUNCTION__));
	return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsHdmiInSelectZoomMode(void *arg)
{
    _DEBUG_ENTER();
    HDMI_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    IARM_BUS_Lock(lock);
    dsHdmiInSelectZoomModeParam_t *param = (dsHdmiInSelectZoomModeParam_t *)arg;

#ifdef HAS_HDMI_IN_SUPPORT
    param->result = dsHdmiInSelectZoomMode(param->zoomMode);
#else
    param->result = dsERR_GENERAL;
#endif

    IARM_BUS_Unlock(lock);

    HDMI_IN_TRACE(("%s <-- \n", __PRETTY_FUNCTION__));
	return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsHdmiInPauseAudio(void *arg)
{
    _DEBUG_ENTER();
    HDMI_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    IARM_BUS_Lock(lock);
    dsError_t *param = (dsError_t *)arg;

#ifdef HAS_HDMI_IN_SUPPORT
    *param = dsHdmiInPauseAudio();
#else
    *param = dsERR_GENERAL;
#endif

    IARM_BUS_Unlock(lock);

    HDMI_IN_TRACE(("%s <-- \n", __PRETTY_FUNCTION__));
	return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsHdmiInResumeAudio(void *arg)
{
    _DEBUG_ENTER();
    HDMI_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    IARM_BUS_Lock(lock);
    dsError_t *param = (dsError_t *)arg;

#ifdef HAS_HDMI_IN_SUPPORT
    *param = dsHdmiInResumeAudio();
#else
    *param = dsERR_GENERAL;
#endif

    IARM_BUS_Unlock(lock);

    HDMI_IN_TRACE(("%s <-- \n", __PRETTY_FUNCTION__));
	return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsHdmiInGetCurrentVideoMode(void *arg)
{
    _DEBUG_ENTER();
    HDMI_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    dsHdmiInGetResolutionParam_t *param = (dsHdmiInGetResolutionParam_t *)arg;

    IARM_BUS_Lock(lock);

#ifdef HAS_HDMI_IN_SUPPORT
    param->result = dsHdmiInGetCurrentVideoMode(&param->resolution);
#else
    param->result = dsERR_GENERAL;
#endif

    IARM_BUS_Unlock(lock);

    HDMI_IN_TRACE(("%s <-- \n", __PRETTY_FUNCTION__));
    return IARM_RESULT_SUCCESS;
}

void _dsHdmiInConnectCB(dsHdmiInPort_t port, bool isPortConnected)
{
    IARM_Bus_DSMgr_EventData_t hdmi_in_hpd_eventData;
    HDMI_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));
    __TIMESTAMP();
    HDMI_IN_TRACE(("HDMI In hotplug update!!!!!!..%d, %d\r\n", port, isPortConnected));
    hdmi_in_hpd_eventData.data.hdmi_in_connect.port = port;
    hdmi_in_hpd_eventData.data.hdmi_in_connect.isPortConnected = isPortConnected;
			
    IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,
	                        (IARM_EventId_t)IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG,
	                        (void *)&hdmi_in_hpd_eventData, 
	                        sizeof(hdmi_in_hpd_eventData));
	                        
    HDMI_IN_TRACE(("%s <-- \n", __PRETTY_FUNCTION__));
}

/** @} */
/** @} */
