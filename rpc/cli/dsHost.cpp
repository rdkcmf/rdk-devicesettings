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
#include "dsRpc.h"
#include "dsMgr.h"
#include "dsError.h"
#include "dsTypes.h"
#include "iarmUtil.h"
#include "libIARM.h"
#include "libIBus.h"
#include "dsHost.h"

dsError_t dsSetPreferredSleepMode(dsSleepMode_t mode)
{
    _DEBUG_ENTER();

    dsPreferredSleepMode param;
    param.mode = mode;
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;
    rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                (char *)IARM_BUS_DSMGR_API_dsSetPreferredSleepMode,
                (void *)&param,
                sizeof(param));
    if (IARM_RESULT_SUCCESS == rpcRet)
    {
        return dsERR_NONE;
    }

    return dsERR_GENERAL ;
 
}

dsError_t dsGetPreferredSleepMode(dsSleepMode_t *mode)
{
    _DEBUG_ENTER();

    dsPreferredSleepMode param;
 
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

    rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                (char *)IARM_BUS_DSMGR_API_dsGetPreferredSleepMode,
                (void *)&param,
                sizeof(param));

    if (IARM_RESULT_SUCCESS != rpcRet)
    {
        return dsERR_GENERAL ;
    }

    *mode = param.mode;
   
    return dsERR_NONE;
 
}


dsError_t dsGetCPUTemperature(float *cpuTemperature)
{
    _DEBUG_ENTER();
    
    dsCPUThermalParam param;
   
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

    param.temperature = 0.0;
    
    rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                (char *)IARM_BUS_DSMGR_API_dsGetCPUTemperature,
                (void *)&param,
                sizeof(param));

    if (IARM_RESULT_SUCCESS != rpcRet)
    {
        return dsERR_GENERAL ;
    }

    *cpuTemperature = param.temperature;
   
    return dsERR_NONE;
}


dsError_t dsGetVersion(uint32_t *versionNumber)
{
    _DEBUG_ENTER();
    
    dsVesrionParam param;
   
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

    param.versionNumber = 0x10000;
    
    rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                (char *)IARM_BUS_DSMGR_API_dsGetVersion,
                (void *)&param,
                sizeof(param));

    if (IARM_RESULT_SUCCESS != rpcRet)
    {
        return dsERR_GENERAL ;
    }

    *versionNumber = param.versionNumber;
   
    return dsERR_NONE;
}

dsError_t dsSetVersion(uint32_t versionNumber)
{
    _DEBUG_ENTER();
    
    dsVesrionParam param;
   
    IARM_Result_t rpcRet = IARM_RESULT_SUCCESS;

    param.versionNumber = versionNumber;
    
    rpcRet = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                (char *)IARM_BUS_DSMGR_API_dsSetVersion,
                (void *)&param,
                sizeof(param));

    if (IARM_RESULT_SUCCESS != rpcRet)
    {
        return dsERR_GENERAL ;
    }

    return dsERR_NONE;
}


/** @} */
/** @} */
