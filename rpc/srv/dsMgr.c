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


#include "dsMgr.h"
#include "libIARM.h"
#include "dsError.h"
#include "libIARM.h"
#include "dsHost.h"
#include <stdlib.h> 
#include <stdio.h> 
#include "dsserverlogger.h"

#include <iostream>
#include "hostPersistence.hpp"


extern IARM_Result_t dsAudioMgr_init();
extern IARM_Result_t dsVideoPortMgr_init();
extern IARM_Result_t dsVideoDeviceMgr_init();
extern IARM_Result_t dsFPDMgr_init();
extern IARM_Result_t dsDisplayMgr_init();
extern IARM_Result_t dsHostMgr_init();
extern IARM_Result_t dsAudioMgr_term();
extern IARM_Result_t dsVideoPortMgr_term();
extern IARM_Result_t dsVideoDeviceMgr_term();
extern IARM_Result_t dsFPDMgr_term();
extern IARM_Result_t dsDisplayMgr_term();
extern IARM_Result_t dsHostMgr_term();
extern IARM_Result_t dsHdmiInMgr_init();
extern IARM_Result_t dsHdmiInMgr_term();

IARM_Result_t dsMgr_init()
{
    IARM_Result_t ret = IARM_RESULT_SUCCESS;

	device::HostPersistence::getInstance().load();
	dsHostInit();
	dsDisplayMgr_init();
	dsAudioMgr_init();
	dsVideoPortMgr_init();
	dsVideoDeviceMgr_init();
	dsFPDMgr_init();
	dsHostMgr_init();
	dsHdmiInMgr_init();
	return ret;
}

IARM_Result_t dsMgr_term()
{
    IARM_Result_t ret = IARM_RESULT_SUCCESS;
   
	dsAudioMgr_term();
	dsVideoPortMgr_term();
	dsVideoDeviceMgr_term();
	dsFPDMgr_term();
	dsDisplayMgr_term();
	dsHostMgr_term();
    dsHdmiInMgr_term();
	return ret;
}


/** @} */
/** @} */
