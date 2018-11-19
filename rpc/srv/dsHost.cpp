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


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <iostream>
#include "hostPersistence.hpp"
#include <sstream>
#include "iarmUtil.h"
#include "libIARM.h"
#include "libIBus.h"
#include "dsRpc.h"
#include "dsError.h"
#include "dsTypes.h"
#include "dsHost.h"


static int m_isInitialized = 0;
static pthread_mutex_t hostLock = PTHREAD_MUTEX_INITIALIZER;

#define IARM_BUS_Lock(lock) pthread_mutex_lock(&hostLock)
#define IARM_BUS_Unlock(lock) pthread_mutex_unlock(&hostLock)


IARM_Result_t _dsGetPreferredSleepMode(void *arg);
IARM_Result_t _dsSetPreferredSleepMode(void *arg);
IARM_Result_t _dsGetCPUTemperature(void *arg);
IARM_Result_t _dsGetVersion(void *arg);
IARM_Result_t _dsSetVersion(void *arg);

static dsSleepMode_t _SleepMode = dsHOST_SLEEP_MODE_LIGHT;

using namespace std;

static string enumToString( dsSleepMode_t mode );
static dsSleepMode_t stringToEnum ( string mode );

#define DSHAL_API_VERSION_MAJOR_DEFAULT     1 
#define DSHAL_API_VERSION_MINOR_DEFAULT     0


IARM_Result_t dsHostMgr_init()
{
  
   try{
        string mode="LIGHT_SLEEP";
        mode = device::HostPersistence::getInstance().getProperty("Power.Mode",mode);
        printf("Sleep mode Persistent value is -> %s\n",mode.c_str());
        _SleepMode = stringToEnum(mode);        
    }
    catch(...)
    {
        printf("Error in Reading the Power Mode Persisent \r\n");
    }

    if (!m_isInitialized) {
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetPreferredSleepMode,_dsGetPreferredSleepMode);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetPreferredSleepMode,_dsSetPreferredSleepMode);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetCPUTemperature,_dsGetCPUTemperature);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetVersion,_dsGetVersion);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetVersion,_dsSetVersion);
        
        uint32_t  halVersion = 0x10000;
        if (dsERR_NONE != dsGetVersion(&halVersion))
        {
            halVersion =  dsHAL_APIVER(DSHAL_API_VERSION_MAJOR_DEFAULT, DSHAL_API_VERSION_MINOR_DEFAULT);
        }
        __TIMESTAMP();printf("DS HAL Version is - %d.%d \r\n",dsHAL_APIVER_MAJOR(halVersion),dsHAL_APIVER_MINOR(halVersion));
      
        m_isInitialized = 1;
    }
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t dsHostMgr_term()
{
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsSetPreferredSleepMode(void *arg)
{
    IARM_Result_t ret =  IARM_RESULT_SUCCESS;
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    dsPreferredSleepMode *param = (dsPreferredSleepMode *)arg;
    printf("_dsSetPreferredSleepMode called with the mode - %s \r\n", enumToString(param->mode).c_str());
    try{
        
        device::HostPersistence::getInstance().persistHostProperty("Power.Mode",enumToString(param->mode));
        _SleepMode  = param->mode;
        ret = IARM_RESULT_SUCCESS;
    }
    catch(...)
    {
        printf("Error in Persisting the Power Mode\r\n");
    }
    IARM_BUS_Unlock(lock);
    return ret;
}

IARM_Result_t _dsGetPreferredSleepMode(void *arg)
{
    IARM_Result_t ret = IARM_RESULT_SUCCESS;

   _DEBUG_ENTER();
    
    IARM_BUS_Lock(lock);

    dsPreferredSleepMode *param = (dsPreferredSleepMode *)arg;
    param->mode =_SleepMode;
    
    printf("_dsGetPreferredSleepMode: Mode  - %s \r\n", enumToString(param->mode).c_str());
    
    IARM_BUS_Unlock(lock);
    return ret;
}

string enumToString( dsSleepMode_t mode )
{
	string ret;
	switch(mode)
	{
		case dsHOST_SLEEP_MODE_LIGHT:
			ret = "LIGHT_SLEEP";
		break;
		case dsHOST_SLEEP_MODE_DEEP:
			ret = "DEEP_SLEEP";
		break;
		default:
		ret = "LIGHT_SLEEP";
	}
	return ret;
}

dsSleepMode_t stringToEnum (string mode )
{
    if(mode == "LIGHT_SLEEP")
    {
        return dsHOST_SLEEP_MODE_LIGHT;
    }
    else if(mode == "DEEP_SLEEP")
    {
        return dsHOST_SLEEP_MODE_DEEP;
    }
    return dsHOST_SLEEP_MODE_LIGHT; 
}

IARM_Result_t _dsGetCPUTemperature(void *arg)
{
   
    _DEBUG_ENTER();
    IARM_Result_t ret = IARM_RESULT_SUCCESS;


    #ifdef HAS_THERMAL_API 
    IARM_BUS_Lock(lock);
   
    dsCPUThermalParam *param = (dsCPUThermalParam *)arg;
    float temperature = 45.0;
    dsError_t retValue = dsERR_NONE;

    retValue = dsGetCPUTemperature(&temperature);
    if (retValue == dsERR_NONE)
    {
        param->temperature = temperature;
    }

    printf("Current temperature in SRV is: %+7.2fC\n", param->temperature);

    IARM_BUS_Unlock(lock);
    #endif

    return ret;
}


IARM_Result_t _dsGetVersion(void *arg)
{
   
    _DEBUG_ENTER();
   
    IARM_BUS_Lock(lock);
   
    IARM_Result_t ret = IARM_RESULT_SUCCESS;
    dsError_t retValue = dsERR_NONE;
    dsVesrionParam *param = (dsVesrionParam *)arg;
    uint32_t  ver = 0x10000;
 
    retValue = dsGetVersion(&ver);
    if (retValue == dsERR_NONE)
    {
        param->versionNumber = ver;
    }
    else
    {
        param->versionNumber =  dsHAL_APIVER(DSHAL_API_VERSION_MAJOR_DEFAULT, DSHAL_API_VERSION_MINOR_DEFAULT);
    }
          
    __TIMESTAMP();printf("DS HAL Version - %d.%d \r\n",dsHAL_APIVER_MAJOR(param->versionNumber),dsHAL_APIVER_MINOR(param->versionNumber));
   
    IARM_BUS_Unlock(lock);

    return ret;
}


IARM_Result_t _dsSetVersion(void *arg)
{
   
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    IARM_Result_t ret = IARM_RESULT_SUCCESS;
    
    dsError_t retValue = dsERR_NONE;
    dsVesrionParam *param = (dsVesrionParam *)arg;

    retValue = dsSetVersion(param->versionNumber);
    if (retValue != dsERR_NONE)
    {
       __TIMESTAMP();printf("Error in Setting versionNumber: 0x%08X \r\n",param->versionNumber);
    }

    IARM_BUS_Unlock(lock);
        
    return ret;
}


/** @} */
/** @} */
