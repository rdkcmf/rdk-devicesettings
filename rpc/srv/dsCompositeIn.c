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
#include "dsCompositeIn.h"
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

IARM_Result_t dsCompositeInMgr_init();
IARM_Result_t dsCompositeInMgr_term();
IARM_Result_t _dsCompositeInInit(void *arg);
IARM_Result_t _dsCompositeInTerm(void *arg);
IARM_Result_t _dsCompositeInGetNumberOfInputs(void *arg);
IARM_Result_t _dsCompositeInGetStatus(void *arg);
IARM_Result_t _dsCompositeInSelectPort(void *arg);
IARM_Result_t _dsCompositeInScaleVideo(void *arg);

void _dsCompositeInConnectCB(dsCompositeInPort_t port, bool isPortConnected);

#include <iostream>
#include "hostPersistence.hpp"
#include <sstream>


using namespace std;


IARM_Result_t dsCompositeInMgr_init()
{

    _dsCompositeInInit(NULL);
    IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsCompositeInInit, _dsCompositeInInit);

    return IARM_RESULT_SUCCESS;
}

IARM_Result_t dsCompositeInMgr_term()
{

    _dsCompositeInTerm(NULL);
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsCompositeInInit(void *arg)
{
    printf("%s:%d ---> m_isInitialized=%d, m_isPlatInitialized=%d \n",
                   __PRETTY_FUNCTION__,__LINE__, m_isInitialized, m_isPlatInitialized);

    IARM_BUS_Lock(lock);


#ifdef HAS_COMPOSITE_IN_SUPPORT
    if (!m_isPlatInitialized)
    {
        typedef dsError_t (*dsCompositeInInit_t)(void);
        static dsCompositeInInit_t func = 0;
        if (func == 0) {
            void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
            if (dllib) {
                func = (dsCompositeInInit_t) dlsym(dllib, "dsCompositeInInit");
                if(func == 0) {
                    printf("dsCompositeInInit(void) is not defined\r\n");
                }
                dlclose(dllib);
            }
            else {
                printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
            }
        }

        if(func) {
              printf("%s:%d - invoking dsCompositeInInit()\n", __PRETTY_FUNCTION__,__LINE__);
              func();
        }
    }
    m_isPlatInitialized++;
#endif

    if (!m_isInitialized)
    {
#ifdef HAS_COMPOSITE_IN_SUPPORT
        typedef dsError_t (*dsCompositeInRegisterConnectCB_t)(dsCompositeInConnectCB_t CBFunc);
        static dsCompositeInRegisterConnectCB_t cbfunc = 0;
        if (cbfunc == 0) {
            void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
            if (dllib) {
                cbfunc = (dsCompositeInRegisterConnectCB_t) dlsym(dllib, "dsCompositeInRegisterConnectCB");
                if(cbfunc == 0) {
                    printf("dsCompositeInRegisterConnectCB(dsCompositeInConnectCB_t) is not defined\r\n");
                }
                dlclose(dllib);
            }
            else {
                printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
            }
        }

        if(cbfunc) {
             printf("%s:%d - invoking dsCompositeInRegisterConnectCB()\n", __PRETTY_FUNCTION__,__LINE__);
             cbfunc(_dsCompositeInConnectCB);
        }
#endif
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsCompositeInTerm,                  _dsCompositeInTerm);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsCompositeInGetNumberOfInputs,     _dsCompositeInGetNumberOfInputs);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsCompositeInGetStatus,             _dsCompositeInGetStatus);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsCompositeInSelectPort,            _dsCompositeInSelectPort);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsCompositeInScaleVideo,            _dsCompositeInScaleVideo);

        m_isInitialized = 1;
    }

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}


IARM_Result_t _dsCompositeInTerm(void *arg)
{
    _DEBUG_ENTER();
    printf("%s:%d ---> m_isPlatInitialized=%d\n", __PRETTY_FUNCTION__,__LINE__, m_isPlatInitialized);

    IARM_BUS_Lock(lock);
#ifdef HAS_COMPOSITE_IN_SUPPORT
    if (m_isPlatInitialized)
    {
        m_isPlatInitialized--;
        if (!m_isPlatInitialized)
        {
            typedef dsError_t (*dsCompositeInTerm_t)(void);
            static dsCompositeInTerm_t func = 0;
            if (func == 0) {
                void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
                if (dllib) {
                    func = (dsCompositeInTerm_t) dlsym(dllib, "dsCompositeInTerm");
                    if(func == 0) {
                        printf("dsCompositeInTerm(void) is not defined\r\n");
                    }
                    dlclose(dllib);
                }
                else {
                    printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
                }
            }
            if(func) {
                 printf("%s:%d - invoking dsCompositeInTerm()\n", __PRETTY_FUNCTION__,__LINE__);
                 func();
            }
        }
    }
#endif
    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}


IARM_Result_t _dsCompositeInGetNumberOfInputs(void *arg)
{
    _DEBUG_ENTER();

    dsCompositeInGetNumberOfInputsParam_t *param = (dsCompositeInGetNumberOfInputsParam_t *)arg;

    IARM_BUS_Lock(lock);

#ifdef HAS_COMPOSITE_IN_SUPPORT
    typedef dsError_t (*dsCompositeInGetNumberOfInputs_t)(uint8_t *);
    static dsCompositeInGetNumberOfInputs_t func = NULL;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsCompositeInGetNumberOfInputs_t) dlsym(dllib, "dsCompositeInGetNumberOfInputs");
            if(func == 0) {
                printf("dsCompositeInGetNumberOfInputs(uint8_t *) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }
    if(func) {
        printf("%s:%d - invoking dsCompositeInGetNumberOfInputs()\n", __PRETTY_FUNCTION__,__LINE__);
        param->result = func(&param->numCompositeInputs);
    }
#else
    param->result = dsERR_GENERAL;
    #endif
    IARM_BUS_Unlock(lock);
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsCompositeInGetStatus(void *arg)
{
    _DEBUG_ENTER();

    dsCompositeInGetStatusParam_t *param= (dsCompositeInGetStatusParam_t *)arg;

    IARM_BUS_Lock(lock);

#ifdef HAS_COMPOSITE_IN_SUPPORT
    typedef dsError_t (*dsCompositeInGetStatus_t)(dsCompositeInStatus_t *);
    static dsCompositeInGetStatus_t func = NULL;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsCompositeInGetStatus_t) dlsym(dllib, "dsCompositeInGetStatus");
            if(func ==0) {
                printf("dsCompositeInGetStatus(dsCompositeInStatus_t *) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }
    if(func) {
        printf("%s:%d - invoking dsCompositeInGetStatus()\n", __PRETTY_FUNCTION__,__LINE__);
        param->result = func(&param->status);
    }
#else
    param->result = dsERR_GENERAL;
#endif

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsCompositeInSelectPort(void *arg)
{
    _DEBUG_ENTER();

    dsCompositeInSelectPortParam_t *param = (dsCompositeInSelectPortParam_t *)arg;

    IARM_BUS_Lock(lock);

#ifdef HAS_COMPOSITE_IN_SUPPORT
    typedef dsError_t (*dsCompositeInSelectPort_t)(dsCompositeInPort_t);
    static dsCompositeInSelectPort_t func = NULL;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsCompositeInSelectPort_t) dlsym(dllib, "dsCompositeInSelectPort");
            if(func == 0) {
                printf("dsCompositeInSelectPort(dsCompositeInPort_t) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }
    if(func) {
        printf("%s:%d - invoking dsCompositeInSelectPort()\n", __PRETTY_FUNCTION__,__LINE__);
        param->result = func(param->port);
    }
#else
    param->result = dsERR_GENERAL;
#endif

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsCompositeInScaleVideo(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);
    dsCompositeInScaleVideoParam_t *param = (dsCompositeInScaleVideoParam_t *)arg;

#ifdef HAS_COMPOSITE_IN_SUPPORT
    typedef dsError_t (*dsCompositeInScaleVideo_t)(int32_t , int32_t , int32_t , int32_t);
    static dsCompositeInScaleVideo_t func = NULL;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsCompositeInScaleVideo_t) dlsym(dllib, "dsCompositeInScaleVideo");
            if(func == 0) {
                printf("dsCompositeInScaleVideo(int32_t x, int32_t y, int32_t width, int32_t height) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    if(func) {
        printf("%s:%d - invoking dsCompositeInScaleVideo()\n", __PRETTY_FUNCTION__,__LINE__);
        param->result = func(param->videoRect.x, param->videoRect.y, param->videoRect.width, param->videoRect.height);
    }
#else
    param->result = dsERR_GENERAL;
#endif

    IARM_BUS_Unlock(lock);

	return IARM_RESULT_SUCCESS;
}


void _dsCompositeInConnectCB(dsCompositeInPort_t port, bool isPortConnected)
{
    IARM_Bus_DSMgr_EventData_t composite_in_hpd_eventData;
    __TIMESTAMP();
    printf("%s:%d - COMPOSITE In hotplug update!!!!!!..%d, %d\r\n",__PRETTY_FUNCTION__,__LINE__, port, isPortConnected);
    composite_in_hpd_eventData.data.composite_in_connect.port = port;
    composite_in_hpd_eventData.data.composite_in_connect.isPortConnected = isPortConnected;

    IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,
	                        (IARM_EventId_t)IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_HOTPLUG,
	                        (void *)&composite_in_hpd_eventData,
	                        sizeof(composite_in_hpd_eventData));

}

/** @} */
/** @} */
