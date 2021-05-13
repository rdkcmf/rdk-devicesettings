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


#include "dsVideoPort.h"
#include "dsDisplay.h"

#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include <dlfcn.h>
#include "dsError.h"
#include "dsUtl.h"
#include "dsTypes.h"
#include "pthread.h"
#include <pthread.h>
#include "libIARM.h"
#include "iarmUtil.h"
#include "libIBusDaemon.h"
#include "libIBus.h"
#include "dsRpc.h"
#include "dsMgr.h"
#include <iostream>
#include <string.h>
#include "hostPersistence.hpp"
#include "dsserverlogger.h"
#include "dsTypes.h"
#include "dsVideoPortSettings.h"

#ifdef DEVICESETTINGS_DEFAULT_RESOLUTION
  #define DEFAULT_RESOLUTION DEVICESETTINGS_DEFAULT_RESOLUTION
#else
  #define DEFAULT_RESOLUTION "720p"
#endif
#define DEFAULT_SD_RESOLUTION "480i"

static int m_isInitialized = 0;
static int m_isPlatInitialized = 0;
static pthread_mutex_t dsLock = PTHREAD_MUTEX_INITIALIZER;
static std::string _dsHDMIResolution(DEFAULT_RESOLUTION);
static std::string _dsCompResolution(DEFAULT_RESOLUTION);
static std::string _dsBBResolution(DEFAULT_SD_RESOLUTION);
static std::string _dsRFResolution(DEFAULT_SD_RESOLUTION);
static dsHdcpStatus_t _hdcpStatus = dsHDCP_STATUS_UNAUTHENTICATED;
static bool force_disable_4K = false;
extern bool enableHDRDVStatus;
#define NULL_HANDLE 0
#define IARM_BUS_Lock(lock) pthread_mutex_lock(&dsLock)
#define IARM_BUS_Unlock(lock) pthread_mutex_unlock(&dsLock)

IARM_Result_t _dsVideoPortInit(void *arg);
IARM_Result_t _dsGetVideoPort(void *arg);
IARM_Result_t _dsIsVideoPortEnabled(void *arg);
IARM_Result_t _dsIsDisplayConnected(void *arg);
IARM_Result_t _dsIsDisplaySurround(void *arg);
IARM_Result_t _dsGetSurroundMode(void *arg);
IARM_Result_t _dsEnableVideoPort(void *arg);
IARM_Result_t _dsSetResolution(void *arg);
IARM_Result_t _dsGetResolution(void *arg);
IARM_Result_t _dsVideoPortTerm(void *arg);
IARM_Result_t _dsEnableHDCP(void *arg);
IARM_Result_t _dsIsHDCPEnabled(void *arg);
IARM_Result_t _dsGetHDCPStatus(void *arg);
IARM_Result_t _dsGetHDCPProtocol(void *arg);
IARM_Result_t _dsGetHDCPReceiverProtocol(void *arg);
IARM_Result_t _dsGetHDCPCurrentProtocol(void *arg);
IARM_Result_t _dsIsVideoPortActive(void *arg);
IARM_Result_t _dsGetTVHDRCapabilities(void *arg);
IARM_Result_t _dsSupportedTvResolutions(void *arg);
IARM_Result_t _dsSetForceDisable4K(void *arg);
IARM_Result_t _dsGetForceDisable4K(void *arg);
IARM_Result_t _dsSetScartParameter(void *arg);
IARM_Result_t _dsIsOutputHDR(void *arg);
IARM_Result_t _dsResetOutputToSDR(void *arg);
IARM_Result_t _dsSetHdmiPreference(void *arg);
IARM_Result_t _dsGetHdmiPreference(void *arg);
IARM_Result_t _dsGetVideoEOTF(void *arg);
IARM_Result_t _dsGetMatrixCoefficients(void* arg);
IARM_Result_t _dsGetColorDepth(void* arg);
IARM_Result_t _dsGetColorSpace(void* arg);
IARM_Result_t _dsGetCurrentOutputSettings(void* arg);
IARM_Result_t _dsSetBackgroundColor(void *arg);


static dsVideoPortType_t _GetVideoPortType(int handle);
static int  _dsVideoPortPreResolutionCall(dsVideoPortResolution_t *resolution);
static int  _dsSendVideoPortPostResolutionCall(dsVideoPortResolution_t *resolution);
void _dsHdcpCallback(int handle, dsHdcpStatus_t event);
static void persistResolution(dsVideoPortSetResolutionParam_t *param);


#define IsHDCompatible(p)  (((p) >= dsVIDEO_PIXELRES_1280x720 ) && ((p) < dsVIDEO_PIXELRES_MAX))
static  std::string getCompatibleResolution(dsVideoPortResolution_t *SrcResn);
static bool    IsCompatibleResolution(dsVideoResolution_t pixelResolution1,dsVideoResolution_t pixelResolution2);
static dsVideoResolution_t getPixelResolution(std::string &resolution);

IARM_Result_t dsVideoPortMgr_init()
{
   IARM_BUS_Lock(lock);
   std::string _Resolution(DEFAULT_RESOLUTION);
	
	try
	{
		/*TBD - Get the Device type Dynamically*/
		/*
			* Read the HDMI,Component or Composite
			* Next is to browse through all supported ports  i.e kPorts to differentiate between 
			* Component and Composite. TBD - Remove HAS_ONLY_COMPOSITE
		*/
		_dsHDMIResolution = device::HostPersistence::getInstance().getProperty("HDMI0.resolution",_Resolution);
		__TIMESTAMP();printf("The Persistent HDMI resolution read is %s \r\n",_dsHDMIResolution.c_str());
		#ifdef HAS_ONLY_COMPOSITE
           _Resolution = DEFAULT_RESOLUTION;
			_dsCompResolution = device::HostPersistence::getInstance().getProperty("Baseband0.resolution",_Resolution);
		#else
           _Resolution = DEFAULT_RESOLUTION;
			_dsCompResolution = device::HostPersistence::getInstance().getProperty("COMPONENT0.resolution",_Resolution);
		#endif
		__TIMESTAMP();printf("The Persistent Component/Composite resolution read is %s \r\n",_dsCompResolution.c_str());
                 _dsRFResolution = device::HostPersistence::getInstance().getProperty("RF0.resolution",_Resolution);
                __TIMESTAMP();printf("The Persistent RF resolution read is %s \r\n",_dsRFResolution.c_str());
                _dsBBResolution = device::HostPersistence::getInstance().getProperty("Baseband0.resolution",_Resolution);
                __TIMESTAMP();printf("The Persistent BB resolution read is %s \r\n",_dsBBResolution.c_str());
					
		if (!m_isPlatInitialized) 
		{
			/*Initialize the Video Ports */
			dsVideoPortInit();
		}
		/*coverity[missing_lock]  CID-18497 using Coverity Annotation to ignore error*/
		m_isPlatInitialized ++;
	}
	catch(...) 
	{
		printf("Error in Getting the Video Resolution on Startup..... \r\n");
	}
	try
	{
		std::string _4K_setting("false");
		_4K_setting = device::HostPersistence::getInstance().getProperty("VideoDevice.force4KDisabled", _4K_setting);
		if (_4K_setting.compare("true") == 0)
		{
			force_disable_4K = true;
			printf("4K support in disabled configuration.\n");
                }
		else
		{
			force_disable_4K = false;
		}
	}
	catch(...) 
	{
		printf("Exception in getting force-disable-4K setting at start up.\r\n");
	}
	IARM_BUS_Unlock(lock);  //CID:136282 - Data race condition
	IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsVideoPortInit, _dsVideoPortInit);
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t dsVideoPortMgr_term()
{
   return IARM_RESULT_SUCCESS;
}


IARM_Result_t _dsVideoPortInit(void *arg)
{
    IARM_BUS_Lock(lock);

    if (!m_isInitialized) {
		
		#ifdef HAS_HDCP_CALLBACK
			dsRegisterHdcpStatusCallback(NULL,_dsHdcpCallback);
		#endif
		
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetVideoPort,_dsGetVideoPort);
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsIsVideoPortEnabled,_dsIsVideoPortEnabled);
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsIsDisplayConnected,_dsIsDisplayConnected);
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsIsDisplaySurround,_dsIsDisplaySurround);
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetSurroundMode,_dsGetSurroundMode);
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsEnableVideoPort,_dsEnableVideoPort);
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetResolution,_dsSetResolution);
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetResolution,_dsGetResolution);
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsVideoPortTerm,_dsVideoPortTerm);
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsEnableHDCP ,_dsEnableHDCP);
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsIsHDCPEnabled,_dsIsHDCPEnabled);
	    IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetHDCPStatus ,_dsGetHDCPStatus); 
	    IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetHDCPProtocol ,_dsGetHDCPProtocol);
	    IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetHDCPReceiverProtocol ,_dsGetHDCPReceiverProtocol);
	    IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetHDCPCurrentProtocol ,_dsGetHDCPCurrentProtocol);
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsIsVideoPortActive ,_dsIsVideoPortActive); 
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetTVHDRCapabilities,_dsGetTVHDRCapabilities);
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetSupportedTVResolution,_dsSupportedTvResolutions);
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetForceDisable4K, _dsSetForceDisable4K); 
		IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetForceDisable4K, _dsGetForceDisable4K); 
        	IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetScartParameter,_dsSetScartParameter);
                IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsIsOutputHDR,_dsIsOutputHDR);
                IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsResetOutputToSDR,_dsResetOutputToSDR);
                IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetHdmiPreference,_dsSetHdmiPreference);
                IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetHdmiPreference,_dsGetHdmiPreference);
                IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetVideoEOTF,_dsGetVideoEOTF);
                IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetMatrixCoefficients,_dsGetMatrixCoefficients);
                IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetColorDepth,_dsGetColorDepth);
                IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetColorSpace,_dsGetColorSpace);
                IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetCurrentOutputSettings,_dsGetCurrentOutputSettings);
                IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetBackgroundColor,_dsSetBackgroundColor);
	
        m_isInitialized = 1;
    }

    if (!m_isPlatInitialized) {
    	/* Nexus init, if any here */
    	dsVideoPortInit();
    }
    m_isPlatInitialized++;

    IARM_BUS_Unlock(lock);
	
	return IARM_RESULT_SUCCESS;

}

IARM_Result_t _dsGetVideoPort(void *arg)
{
    _DEBUG_ENTER();
    
	IARM_BUS_Lock(lock);
    
	dsVideoPortGetHandleParam_t *param = (dsVideoPortGetHandleParam_t *)arg;
    dsGetVideoPort(param->type, param->index, &param->handle);

	IARM_BUS_Unlock(lock);
	return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsIsVideoPortEnabled(void *arg)
{
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);
    
	dsVideoPortIsEnabledParam_t *param = (dsVideoPortIsEnabledParam_t *)arg;
    dsIsVideoPortEnabled(param->handle, &param->enabled);
   
    IARM_BUS_Unlock(lock);
	
	return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsIsVideoPortActive(void *arg)
{
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);
    
	dsVideoPortIsActiveParam_t *param = (dsVideoPortIsActiveParam_t *)arg;
    
    dsVideoPortType_t _VPortType = _GetVideoPortType(param->handle);

	if (_VPortType == dsVIDEOPORT_TYPE_HDMI ||
         _VPortType == dsVIDEOPORT_TYPE_INTERNAL)
	{
		param->result = dsIsVideoPortActive(param->handle, &param->active);
	}
	else if (_VPortType == dsVIDEOPORT_TYPE_COMPONENT || _VPortType == dsVIDEOPORT_TYPE_RF)
	{
		param->active = true;
        param->result =  dsERR_NONE;
	}
	else if (_VPortType == dsVIDEOPORT_TYPE_SCART)
	{
		param->active = true;
		param->result = dsERR_NONE;
	}

    IARM_BUS_Unlock(lock);
	
	return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsGetVideoEOTF(void* arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetVideoEOTF_t)(int handle, dsHDRStandard_t* video_eotf);
    static dsGetVideoEOTF_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetVideoEOTF_t) dlsym(dllib, "dsGetVideoEOTF");
            if (func) {
                printf("dsGetVideoEOTF_t(int, dsHDRStandard_t*) is defined and loaded\r\n");
            }
            else {
                printf("dsGetVideoEOTF_t(int, dsHDRStandard_t*) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsEot_t* param = (dsEot_t*)arg;

    if (func != 0) {
        param->result = func(param->handle, &param->video_eotf);
    }
    else {
        param->video_eotf = dsHDRSTANDARD_NONE;
    }

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsGetMatrixCoefficients(void* arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetMatrixCoefficients_t)(int handle, dsDisplayMatrixCoefficients_t* matrix_coefficients);
    static dsGetMatrixCoefficients_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetMatrixCoefficients_t) dlsym(dllib, "dsGetMatrixCoefficients");
            if (func) {
                printf("dsGetMatrixCoefficients_t(int, dsDisplayMatrixCoefficients_t*) is defined and loaded\r\n");
            }
            else {
                printf("dsGetMatrixCoefficients_t(int, dsDisplayMatrixCoefficients_t*) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsMatrixCoefficients_t* param = (dsMatrixCoefficients_t*)arg;

    if (func != 0) {
        param->result = func(param->handle, &param->matrix_coefficients);
    }
    else {
        param->matrix_coefficients = dsDISPLAY_MATRIXCOEFFICIENT_UNKNOWN;
    }

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsGetColorDepth(void* arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetColorDepth_t)(int handle, unsigned int* color_depth);
    static dsGetColorDepth_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetColorDepth_t) dlsym(dllib, "dsGetColorDepth");
            if (func) {
                printf("dsGetColorDepth_t(int, unsigned int*) is defined and loaded\r\n");
            }
            else {
                printf("dsGetColorDepth_t(int, unsigned int*) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsColorDepth_t* param = (dsColorDepth_t*)arg;

    if (func != 0) {
        param->result = func(param->handle, &param->color_depth);
    }
    else {
        param->color_depth = 0;
    }

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsGetColorSpace(void* arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetColorSpace_t)(int handle, dsDisplayColorSpace_t* color_space);
    static dsGetColorSpace_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetColorSpace_t) dlsym(dllib, "dsGetColorSpace");
            if (func) {
                printf("dsGetColorSpace_t(int, dsDisplayColorSpace_t*) is defined and loaded\r\n");
            }
            else {
                printf("dsGetColorSpace_t(int, dsDisplayColorSpace_t*) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsColorSpace_t* param = (dsColorSpace_t*)arg;

    if (func != 0) {
        param->result = func(param->handle, &param->color_space);
    }
    else {
        param->color_space = dsDISPLAY_COLORSPACE_UNKNOWN;
    }

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsGetCurrentOutputSettings(void* arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetCurrentOutputSettings_t)(int handle, dsHDRStandard_t* video_eotf, dsDisplayMatrixCoefficients_t* matrix_coefficients, dsDisplayColorSpace_t* color_space, unsigned int* color_depth);
    static dsGetCurrentOutputSettings_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetCurrentOutputSettings_t) dlsym(dllib, "dsGetCurrentOutputSettings");
            if (func) {
                printf("dsGetCurrentOutputSettings_t(int, dsHDRStandard_t*, dsDisplayMatrixCoefficients_t*, dsDisplayColorSpace_t*, unsigned int*) is defined and loaded\r\n");
            }
            else {
                printf("dsGetCurrentOutputSettings_t(int, dsHDRStandard_t*, dsDisplayMatrixCoefficients_t*, dsDisplayColorSpace_t*, unsigned int*) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsCurrentOutputSettings_t* param = (dsCurrentOutputSettings_t*)arg;

    if (func != 0) {
        param->result = func(param->handle, &param->video_eotf, &param->matrix_coefficients, &param->color_space, &param->color_depth);
    }
    else {
        param->color_space = dsDISPLAY_COLORSPACE_UNKNOWN;
        param->color_depth = 0;
        param->matrix_coefficients = dsDISPLAY_MATRIXCOEFFICIENT_UNKNOWN;
        param->video_eotf = dsHDRSTANDARD_NONE;
    }

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsIsHDCPEnabled(void *arg)
{
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);
    
    dsVideoPortIsHDCPEnabledParam_t *param = (dsVideoPortIsHDCPEnabledParam_t *)arg;
    dsIsHDCPEnabled(param->handle, &param->enabled);

#if 0
    if(param->enabled){
    	__TIMESTAMP();printf("isHDCP =true !!!!!!..\r\n");
	}
    else{
    	__TIMESTAMP();printf("isHDCP =false !!!!!!..\r\n");
    }
#endif

    IARM_BUS_Unlock(lock);
	
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsIsDisplayConnected(void *arg)
{
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);
   
	dsVideoPortIsDisplayConnectedParam_t *param = (dsVideoPortIsDisplayConnectedParam_t *)arg;
    dsIsDisplayConnected(param->handle,&param->connected);

	IARM_BUS_Unlock(lock);
	
	return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsIsDisplaySurround(void *arg)
{

#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    printf("dsSRV::_dsIsDisplaySurround \r\n");

    typedef dsError_t (*dsIsDisplaySurround_t)(int handle, bool *surround);
    static dsIsDisplaySurround_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsIsDisplaySurround_t) dlsym(dllib, "dsIsDisplaySurround");
            if (func) {
                printf("dsIsDisplaySurround_t(int, bool*) is defined and loaded\r\n");
            }   
            else {
                printf("dsIsDisplaySurround_t(int, bool*) is not defined\r\n");
            }   
            dlclose(dllib);
        }   
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }   
    }   

	dsVideoPortIsDisplaySurroundParam_t *param = (dsVideoPortIsDisplaySurroundParam_t *)arg;

    if (func != 0) {
        dsError_t ret = func(param->handle, &param->surround);
        printf("dsSRV ::isDisplaySurround() returns %d %d\r\n", ret, param->surround);
    }
    else {
        param->surround = false;
    }

	IARM_BUS_Unlock(lock);
	
	return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsGetSurroundMode(void *arg)
{

#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    printf("dsSRV::_dsGetSurroundMode \r\n");

    typedef dsError_t (*dsGetSurroundMode_t)(int handle, int *surround);
    static dsGetSurroundMode_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetSurroundMode_t) dlsym(dllib, "dsGetSurroundMode");
            if (func) {
                printf("dsGetSurroundMode_t(int, int*) is defined and loaded\r\n");
            }
            else {
                printf("dsGetSurroundMode_t(int, int*) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

	dsVideoPortGetSurroundModeParam_t *param = (dsVideoPortGetSurroundModeParam_t *)arg;

    if (func != 0) {
        dsError_t ret = func(param->handle, &param->surround);
        printf("dsSRV ::_dsGetSurroundMode() returns %d %d\r\n", ret, param->surround);
    }
    else {
        param->surround = dsSURROUNDMODE_NONE;
    }

	IARM_BUS_Unlock(lock);
	return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsEnableVideoPort(void *arg)
{
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);
   
	dsVideoPortSetEnabledParam_t *param = (dsVideoPortSetEnabledParam_t *)arg;
    dsEnableVideoPort(param->handle, param->enabled);
   
    IARM_BUS_Unlock(lock);
	
	return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsGetResolution(void *arg)
{
    _DEBUG_ENTER();
   
	IARM_BUS_Lock(lock);

	
   std::string _Resolution(DEFAULT_RESOLUTION);
	dsVideoPortGetResolutionParam_t *param = (dsVideoPortGetResolutionParam_t *)arg;
	dsVideoPortResolution_t *resolution = &param->resolution;	

	dsVideoPortType_t _VPortType = _GetVideoPortType(param->handle);

	if (_VPortType == dsVIDEOPORT_TYPE_HDMI ||
         _VPortType == dsVIDEOPORT_TYPE_INTERNAL)
	{
		if(param->toPersist)
		{
			_Resolution = device::HostPersistence::getInstance().getProperty("HDMI0.resolution",_Resolution);
			__TIMESTAMP();printf("Reading HDMI  persistent resolution %s\r\n",_Resolution.c_str());
		}
		else{
			_Resolution = _dsHDMIResolution;
		}
	}
	else if (_VPortType == dsVIDEOPORT_TYPE_COMPONENT)
	{
		if(param->toPersist){
			#ifdef HAS_ONLY_COMPOSITE
				_Resolution = device::HostPersistence::getInstance().getProperty("Baseband0.resolution",_Resolution);
			#else
				_Resolution = device::HostPersistence::getInstance().getProperty("COMPONENT0.resolution",_Resolution);
			#endif
			__TIMESTAMP();printf("Reading Component persistent resolution %s\r\n",_Resolution.c_str());
		}
		else{
			_Resolution = _dsCompResolution; 
		}
	}
        else if (_VPortType == dsVIDEOPORT_TYPE_BB )
        {
                if(param->toPersist){
                        _Resolution = device::HostPersistence::getInstance().getProperty("Baseband0.resolution",_Resolution);
                        __TIMESTAMP();printf("Reading BB persistent resolution %s\r\n",_Resolution.c_str());
                }
                else{
                        _Resolution = _dsBBResolution;
                }
        }
        else if (_VPortType == dsVIDEOPORT_TYPE_RF )
        {
                if(param->toPersist){
                        _Resolution = device::HostPersistence::getInstance().getProperty("RF0.resolution",_Resolution);
                        __TIMESTAMP();printf("Reading RF persistent resolution %s\r\n",_Resolution.c_str());
                }
                else{
                        _Resolution = _dsRFResolution;
                }
        }
	
	strcpy(resolution->name,_Resolution.c_str());
        printf("%s _VPortType:%d  resolution::%s \n",__FUNCTION__,_VPortType,resolution->name);
	IARM_BUS_Unlock(lock);
	
	return IARM_RESULT_SUCCESS;
}


IARM_Result_t _dsSetResolution(void *arg)
{
    _DEBUG_ENTER();
	dsError_t ret = dsERR_NONE;
    
	IARM_BUS_Lock(lock);

	dsVideoPortSetResolutionParam_t *param = (dsVideoPortSetResolutionParam_t *)arg;
	if (param != NULL)   //CID:82753 - Reverse_inull
        {
		dsVideoPortType_t _VPortType = _GetVideoPortType(param->handle);
		bool isConnected = 0;
		dsIsDisplayConnected(param->handle,&isConnected);
		if(!isConnected)
		{
		    printf("Port _VPortType:%d  not connected..Ignoring Resolution Request------\r\n",_VPortType);
		    ret = dsERR_OPERATION_NOT_SUPPORTED;
		    param->result = ret;
		    IARM_BUS_Unlock(lock);
		    return IARM_RESULT_SUCCESS;
		}
	
		dsVideoPortResolution_t resolution = param->resolution;
		std::string resolutionName(resolution.name);
                __TIMESTAMP();printf("Resolution Requested ..%s \r\n",resolution.name);

		if(force_disable_4K)
		{
			std::size_t location = resolutionName.find("2160");
			if(std::string::npos != location)
			{
				//Trying to set 4K resolution when it's disabled. This cannot be allowed.
				printf("Error! Cannot set 4K resolution. Support for 4K is disabled.\n");
                                ret = dsERR_OPERATION_NOT_SUPPORTED;
                                param->result = ret;
				IARM_BUS_Unlock(lock);
				return IARM_RESULT_SUCCESS;
			}
		}
		/* * Check the Platform Resolution 
		   * If the platform Resolution is same as requested , Do not set new resolution
		   * Needed to avoid setting resolution during multiple hot plug  
		 */

		dsVideoPortResolution_t platresolution;
		memset(platresolution.name,'\0',sizeof(platresolution.name));
		dsGetResolution(param->handle,&platresolution);
		__TIMESTAMP();printf("Resolution Requested ..%s Platform Resolution - %s\r\n",resolution.name,platresolution.name);
		if ((strcmp(resolution.name,platresolution.name) == 0 ))
		{
			
			printf("Same Resolution ..Ignoring Resolution Request------\r\n");
                        _dsHDMIResolution = platresolution.name;
                        /*!< Persist Resolution Settings */
			persistResolution(param);
                        param->result = ret;
			IARM_BUS_Unlock(lock);
			return IARM_RESULT_SUCCESS;
		}
	
		/*!< Resolution Pre Change Event  - IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE */
		_dsVideoPortPreResolutionCall(&param->resolution);

		/*!< Set Platform Resolution  */
		ret = dsSetResolution(param->handle, &param->resolution, param->toPersist);
		
		/*!< Resolution Post Change Event  - IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE */
		_dsSendVideoPortPostResolutionCall(&param->resolution);
		
		if (ret == dsERR_NONE)
		{
			/*!< Persist Resolution Settings */
			persistResolution(param);
		}
		param->result = ret;
	}
		
    IARM_BUS_Unlock(lock);
	
	return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsVideoPortTerm(void *arg)
{
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    m_isPlatInitialized--;
	
	if (0 == m_isPlatInitialized)
	{
		dsVideoPortTerm();
	}
    
	IARM_BUS_Unlock(lock);
 
	return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsEnableHDCP(void *arg)
{
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);
   
    dsError_t ret = dsERR_NONE;

    __TIMESTAMP();printf("Enable HDCP in Platform !! \r\n");

    dsEnableHDCPParam_t *param = (dsEnableHDCPParam_t *)arg;
    ret = dsEnableHDCP(param->handle, param->contentProtect, param->hdcpKey, param->keySize);
    param->rpcResult = ret;
     
    IARM_BUS_Unlock(lock);
	
	return IARM_RESULT_SUCCESS;
}


static int  _dsSendVideoPortPostResolutionCall(dsVideoPortResolution_t *resolution)
{
	dsError_t ret = dsERR_NONE;

	if (resolution == NULL)
	{
		ret = dsERR_INVALID_PARAM;
	}

	if (ret == dsERR_NONE){
		  IARM_Bus_DSMgr_EventData_t eventData;
		  IARM_Bus_CommonAPI_ResChange_Param_t param;

			switch(resolution->pixelResolution) {
			case dsVIDEO_PIXELRES_720x480:
				param.width =  720;
				param.height = 480;
			break;
			case dsVIDEO_PIXELRES_720x576:
				param.width =  720;
				param.height = 576;
			break;

			case dsVIDEO_PIXELRES_1280x720:
				param.width =  1280;
				param.height = 720;
			break;
			
			case dsVIDEO_PIXELRES_1920x1080:
				param.width =  1920;
				param.height = 1080;
			break;
            case dsVIDEO_PIXELRES_3840x2160:
                param.width =  3840;
                param.height = 2160;
                break;
            case dsVIDEO_PIXELRES_4096x2160:
                param.width =  4096;
                param.height = 2160;
                break;
			case dsVIDEO_PIXELRES_MAX: //to mute compiler warning
			default:
				param.width =  1280;
				param.height = 720;
			break;
			}
			eventData.data.resn.width = param.width;
			eventData.data.resn.height = param.height;
			IARM_BusDaemon_ResolutionPostchange(param);
			IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,(IARM_EventId_t)IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE,(void *)&eventData, sizeof(eventData));
		}
	return ret;
}
static int  _dsVideoPortPreResolutionCall(dsVideoPortResolution_t *resolution)
{
	dsError_t ret = dsERR_NONE;
	if (resolution == NULL)
	{
		ret = dsERR_INVALID_PARAM;
	}

	if (ret == dsERR_NONE){
			IARM_Bus_CommonAPI_ResChange_Param_t param;

			switch(resolution->pixelResolution) {
			case dsVIDEO_PIXELRES_720x480:
				param.width =  720;
				param.height = 480;
			break;
			case dsVIDEO_PIXELRES_720x576:
				param.width =  720;
				param.height = 576;
			break;

			case dsVIDEO_PIXELRES_1280x720:
				param.width =  1280;
				param.height = 720;
			break;
			
			case dsVIDEO_PIXELRES_1920x1080:
				param.width =  1920;
				param.height = 1080;
			break;
            case dsVIDEO_PIXELRES_3840x2160:
                param.width =  3840;
                param.height = 2160;
                break;
            case dsVIDEO_PIXELRES_4096x2160:
                param.width =  4096;
                param.height = 2160;
                break;
			case dsVIDEO_PIXELRES_MAX: //to mute compiler warning
			default:
				param.width =  1280;
				param.height = 720;
			break;
			}
		IARM_BusDaemon_ResolutionPrechange(param);
		}
	return ret;
}

/*HDCP Status  Call back */
void _dsHdcpCallback (int handle, dsHdcpStatus_t status)
{
	IARM_Bus_DSMgr_EventData_t hdcp_eventData;
	

	if (handle == NULL_HANDLE)
	{
		printf("Err:HDMI Hot plug back has NULL Handle... !!!!!!..\r\n");
	}
	switch(status)
	{
		case dsHDCP_STATUS_AUTHENTICATED:
			__TIMESTAMP();printf("DS HDCP Authenticated Event!!!!!!..\r\n");
			hdcp_eventData.data.hdmi_hdcp.hdcpStatus =  dsHDCP_STATUS_AUTHENTICATED;
			_hdcpStatus = status;
			break;

		case dsHDCP_STATUS_AUTHENTICATIONFAILURE:
			__TIMESTAMP();printf("DS HDCP Failure Event!!!!!!..\r\n");
			 hdcp_eventData.data.hdmi_hdcp.hdcpStatus =  dsHDCP_STATUS_AUTHENTICATIONFAILURE;
			_hdcpStatus = status;
			break;
		/* Based on discussion with Steve, we may handle the unpowered and unauthenticated 
		cases in a different manner. Logging this events for now.*/
		case dsHDCP_STATUS_UNPOWERED:
		case dsHDCP_STATUS_UNAUTHENTICATED:
		default:
			__TIMESTAMP();printf("HDCP Event Status from HAL is ...%d\n",status);
			hdcp_eventData.data.hdmi_hdcp.hdcpStatus = _hdcpStatus = status;
			break;
	}
	
	IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,(IARM_EventId_t)IARM_BUS_DSMGR_EVENT_HDCP_STATUS,(void *)&hdcp_eventData, sizeof(hdcp_eventData));
}

IARM_Result_t _dsGetHDCPStatus (void *arg)
{
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);
    
    dsVideoPortGetHDCPStatus_t *param = (dsVideoPortGetHDCPStatus_t *)arg;

    if (param != NULL) {
      param->hdcpStatus = _hdcpStatus;
    }
   
    IARM_BUS_Unlock(lock);
	
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsGetHDCPProtocol (void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetHDCPProtocol_t)(int handle, dsHdcpProtocolVersion_t *protocolVersion);
    static dsGetHDCPProtocol_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetHDCPProtocol_t) dlsym(dllib, "dsGetHDCPProtocol");
            if (func) {
                printf("dsGetHDCPProtocol_t(int, dsHdcpProtocolVersion_t *) is defined and loaded\r\n");
            }
            else {
                printf("dsGetHDCPProtocol_t(int, dsHdcpProtocolVersion_t *) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

	dsVideoPortGetHDCPProtocolVersion_t *param = (dsVideoPortGetHDCPProtocolVersion_t *)arg;

    if (func != 0) {
        dsError_t ret = func(param->handle, &param->protocolVersion);
    }
    else {
        param->protocolVersion = dsHDCP_VERSION_1X;
    }

	IARM_BUS_Unlock(lock);

	return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsGetHDCPReceiverProtocol (void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetHDCPReceiverProtocol_t)(int handle, dsHdcpProtocolVersion_t *protocolVersion);
    static dsGetHDCPReceiverProtocol_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetHDCPReceiverProtocol_t) dlsym(dllib, "dsGetHDCPReceiverProtocol");
            if (func) {
                printf("dsGetHDCPReceiverProtocol_t(int, dsHdcpProtocolVersion_t *) is defined and loaded\r\n");
            }
            else {
                printf("dsGetHDCPReceiverProtocol_t(int, dsHdcpProtocolVersion_t *) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

	dsVideoPortGetHDCPProtocolVersion_t *param = (dsVideoPortGetHDCPProtocolVersion_t *)arg;

    if (func != 0) {
        dsError_t ret = func(param->handle, &param->protocolVersion);
    }
    else {
        param->protocolVersion = dsHDCP_VERSION_1X;
    }

	IARM_BUS_Unlock(lock);

	return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsGetHDCPCurrentProtocol (void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetHDCPCurrentProtocol_t)(int handle, dsHdcpProtocolVersion_t *protocolVersion);
    static dsGetHDCPCurrentProtocol_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetHDCPCurrentProtocol_t) dlsym(dllib, "dsGetHDCPCurrentProtocol");
            if (func) {
                printf("dsGetHDCPCurrentProtocol_t(int, dsHdcpProtocolVersion_t *) is defined and loaded\r\n");
            }
            else {
                printf("dsGetHDCPCurrentProtocol_t(int, dsHdcpProtocolVersion_t *) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

	dsVideoPortGetHDCPProtocolVersion_t *param = (dsVideoPortGetHDCPProtocolVersion_t *)arg;

    if (func != 0) {
        dsError_t ret = func(param->handle, &param->protocolVersion);
    }
    else {
        param->protocolVersion = dsHDCP_VERSION_1X;
    }

	IARM_BUS_Unlock(lock);

	return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsGetTVHDRCapabilities(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetTVHDRCapabilitiesFunc_t)(int handle, int *capabilities);
    static dsGetTVHDRCapabilitiesFunc_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetTVHDRCapabilitiesFunc_t)dlsym(dllib, "dsGetTVHDRCapabilities");
            if (func) {
                printf("dsGetTVHDRCapabilities() is defined and loaded\r\n");
            }
            else {
                printf("dsGetTVHDRCapabilities() is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }
    dsGetHDRCapabilitiesParam_t *param = (dsGetHDRCapabilitiesParam_t *)arg;
    if(0 != func) {
        param->result = func(param->handle, &param->capabilities);
        if((param->capabilities & dsHDRSTANDARD_DolbyVision) && (enableHDRDVStatus == false))
        {
            param->capabilities &= ~dsHDRSTANDARD_DolbyVision;
            printf("_dsGetTVHDRCapabilities() DolbyVision Disabled param->capabilities:%x\r\n",param->capabilities);
        }
    }
    else {
        param->capabilities = dsHDRSTANDARD_NONE;
    }

    IARM_BUS_Unlock(lock);
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsSupportedTvResolutions(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsSupportedTvResolutionsFunc_t)(int handle,int *resolutions);
    static dsSupportedTvResolutionsFunc_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSupportedTvResolutionsFunc_t)dlsym(dllib, "dsSupportedTvResolutions");
            if (func) {
                printf("dsSupportedTvResolutions() is defined and loaded\r\n");
            }
            else {
                printf("dsSupportedTvResolutions() is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }
    dsSupportedResolutionParam_t *param = (dsSupportedResolutionParam_t *)arg;
    if(0 != func) {
        param->result = func(param->handle, &param->resolutions);
    }
    else {
        param->resolutions = 0;
    }

    IARM_BUS_Unlock(lock);
    return IARM_RESULT_SUCCESS;
}


static dsVideoPortType_t _GetVideoPortType(int handle)
{
    int numPorts,i;
    int halhandle = 0;
    
    numPorts = dsUTL_DIM(kSupportedPortTypes);
    for(i=0; i< numPorts; i++)
    {
		dsGetVideoPort(kPorts[i].id.type, kPorts[i].id.index, &halhandle);
		if (handle == halhandle)
		{
			return kPorts[i].id.type;
		}
	}
	printf("Error: The Requested Video Port is not part of Platform Port Configuration \r\n");
	return dsVIDEOPORT_TYPE_MAX;
}


static void persistResolution(dsVideoPortSetResolutionParam_t *param)
{
	dsVideoPortResolution_t resolution = param->resolution;
	std::string resolutionName(resolution.name);

	try
	{
		dsVideoPortType_t _VPortType = _GetVideoPortType(param->handle);
		if (_VPortType == dsVIDEOPORT_TYPE_HDMI ||
             _VPortType == dsVIDEOPORT_TYPE_INTERNAL)
		{
			if(param->toPersist){
				device::HostPersistence::getInstance().persistHostProperty("HDMI0.resolution",resolutionName);
			}
			
			__TIMESTAMP();printf("Set Resolution on HDMI Port!!!!!!..\r\n");
			_dsHDMIResolution = resolutionName;

			if (false == IsCompatibleResolution(resolution.pixelResolution,getPixelResolution(_dsCompResolution)))
			{
				__TIMESTAMP();printf("HDMI Resolution is not Compatible with Analog ports..\r\n");
				_dsCompResolution = getCompatibleResolution(&resolution);
				__TIMESTAMP();printf("New Compatible resolution is %s  \r\n",_dsCompResolution.c_str());
				
				if(param->forceCompatible)
				{
					#ifdef HAS_ONLY_COMPOSITE
						device::HostPersistence::getInstance().persistHostProperty("Baseband0.resolution",_dsCompResolution);
					#else
						device::HostPersistence::getInstance().persistHostProperty("COMPONENT0.resolution",_dsCompResolution);
					#endif	
				}
			}
			else
			{
				__TIMESTAMP();printf("HDMI and Analog Ports Resolutions are  Compatible \r\n");
			}

		}
		else if (_VPortType == dsVIDEOPORT_TYPE_COMPONENT)
		{

			if(param->toPersist){
				#ifdef HAS_ONLY_COMPOSITE
					device::HostPersistence::getInstance().persistHostProperty("Baseband0.resolution",resolutionName);
				#else
					device::HostPersistence::getInstance().persistHostProperty("COMPONENT0.resolution",resolutionName);
				#endif	
			}

			__TIMESTAMP();printf("Set Resolution on Component/Composite Ports!!!!!!..\r\n");
			_dsCompResolution = resolutionName;
			if (false == IsCompatibleResolution(resolution.pixelResolution,getPixelResolution(_dsHDMIResolution)))
			{
				__TIMESTAMP();printf("HDMI Resolution is not Compatible with Analog ports..\r\n");
				
				_dsHDMIResolution = getCompatibleResolution(&resolution);
				if (_dsHDMIResolution.compare("480i") == 0)
					_dsHDMIResolution = "480p";

				__TIMESTAMP();printf("New Compatible resolution is %s  \r\n",_dsHDMIResolution.c_str());
				if(param->forceCompatible)
				{
					device::HostPersistence::getInstance().persistHostProperty("HDMI0.resolution",_dsHDMIResolution);
				}
			}
			else
			{
				__TIMESTAMP();printf("HDMI and Analog Ports Resolutions are  Compatible \r\n");
			}
		}
                else if (_VPortType == dsVIDEOPORT_TYPE_BB)
                {

                        if(param->toPersist){
                               device::HostPersistence::getInstance().persistHostProperty("Baseband0.resolution",resolutionName);
                        }

                        __TIMESTAMP();printf("Set Resolution on Composite Ports!!!!!!..\r\n");
                        _dsBBResolution = resolutionName;
                        if (false == IsCompatibleResolution(resolution.pixelResolution,getPixelResolution(_dsHDMIResolution)))
                        {
                                __TIMESTAMP();printf("HDMI Resolution is not Compatible with Analog ports..\r\n");

                                _dsHDMIResolution = getCompatibleResolution(&resolution);
                                if (_dsHDMIResolution.compare("480i") == 0)
                                        _dsHDMIResolution = "480p";

                                __TIMESTAMP();printf("New Compatible resolution is %s  \r\n",_dsHDMIResolution.c_str());
                        }
                        else
                        {
                                __TIMESTAMP();printf("HDMI and Analog Ports Resolutions are  Compatible \r\n");
                        }
                }
                else if (_VPortType == dsVIDEOPORT_TYPE_RF)
                {

                        if(param->toPersist){
                               device::HostPersistence::getInstance().persistHostProperty("RF0.resolution",resolutionName);
                        }

                        __TIMESTAMP();printf("Set Resolution on RF Ports!!!!!!..\r\n");
                        _dsRFResolution = resolutionName;
                        if (false == IsCompatibleResolution(resolution.pixelResolution,getPixelResolution(_dsHDMIResolution)))
                        {
                                __TIMESTAMP();printf("HDMI Resolution is not Compatible with Analog ports..\r\n");

                                _dsHDMIResolution = getCompatibleResolution(&resolution);
                                if (_dsHDMIResolution.compare("480i") == 0)
                                        _dsHDMIResolution = "480p";

                                __TIMESTAMP();printf("New Compatible resolution is %s  \r\n",_dsHDMIResolution.c_str());
                        }
                        else
                        {
                                __TIMESTAMP();printf("HDMI and Analog Ports Resolutions are  Compatible \r\n");
                        }
                }
	}
	catch(...) 
	{
		printf("Error in Persisting the Video Resolution..... \r\n");
	}
}


#ifdef HAS_INIT_RESN_SETTINGS
IARM_Result_t _dsInitResolution(void *arg)
{
	_DEBUG_ENTER();
	dsError_t ret = dsERR_NONE;
	IARM_BUS_Lock(lock);

	dsVideoPortSetResolutionParam_t *param = (dsVideoPortSetResolutionParam_t *)arg;	
	if (param == NULL)
	{
		return IARM_RESULT_INVALID_STATE;
	}

	ret = dsInitResolution(&param->resolution);
	if (ret == dsERR_NONE)
	{
		persistResolution(param);
	}

	IARM_BUS_Unlock(lock);	
	return IARM_RESULT_SUCCESS;
}
#endif


static  std::string getCompatibleResolution(dsVideoPortResolution_t *SrcResn)
{
   dsError_t ret = dsERR_NONE;
   std::string resolution("720p");

   if (SrcResn == NULL)
   {
      ret = dsERR_INVALID_PARAM;
   }

   if (ret == dsERR_NONE){
      switch(SrcResn->pixelResolution) {
      
         case dsVIDEO_PIXELRES_720x480:
          	return resolution.assign(SrcResn->name);
         break;

         case dsVIDEO_PIXELRES_1280x720:
         case dsVIDEO_PIXELRES_1920x1080:           
            case dsVIDEO_PIXELRES_3840x2160:
            case dsVIDEO_PIXELRES_4096x2160:
         case dsVIDEO_PIXELRES_MAX: 
         default:
       		  return resolution.assign(kResolutions[kDefaultResIndex].name);
         break;
      }
   }
   return resolution;
}

static bool  IsCompatibleResolution(dsVideoResolution_t pixelResolution1,dsVideoResolution_t pixelResolution2)
{
   bool bret = false;

 	if( pixelResolution1 == pixelResolution2) {
            bret = true;
        }
	else if((IsHDCompatible(pixelResolution1)) && (IsHDCompatible(pixelResolution2))) {
            bret = true;
         }
    return  bret;
}

static dsVideoResolution_t getPixelResolution(std::string &resolution )
{
  	dsVideoPortResolution_t *Resn = &kResolutions[kDefaultResIndex]; 
	
	for (unsigned int i = 0; i < dsUTL_DIM(kResolutions); i++)
	{
		Resn = &kResolutions[i];
		if (resolution.compare(Resn->name) == 0 )
		{
			break;
		}
	}
	return Resn->pixelResolution;
}


IARM_Result_t _dsSetForceDisable4K(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);
    
    dsForceDisable4KParam_t *param = (dsForceDisable4KParam_t *)arg;
	param->result = dsERR_NONE;

	force_disable_4K = param->disable;
	if(force_disable_4K)
	{
		device::HostPersistence::getInstance().persistHostProperty("VideoDevice.force4KDisabled","true");
	}
	else
	{
		device::HostPersistence::getInstance().persistHostProperty("VideoDevice.force4KDisabled","false");
	}

    IARM_BUS_Unlock(lock);
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsGetForceDisable4K(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);
    
    dsForceDisable4KParam_t *param = (dsForceDisable4KParam_t *)arg;
	param->result = dsERR_NONE;

	param->disable = force_disable_4K;
    IARM_BUS_Unlock(lock);
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsSetScartParameter(void *arg)
{
    dsScartParamParam_t *param = (dsScartParamParam_t *)arg;
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    printf("dsSRV::_dsSetScartParameter\r\n");

    typedef dsError_t (*dsScartParamFunc_t)(int handle, const char* parameter_str, const char* value_str);
    static dsScartParamFunc_t func = NULL;
    if (func == NULL) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib != NULL) {
            func = (dsScartParamFunc_t) dlsym(dllib, "dsSetScartParameter");
            if (func != NULL) {
                printf("dsSRV: dsSetScartParameter(int,const char*,const char*) is defined and loaded\r\n");
            }
            else {
                printf("dsSRV: dsSetScartParameter(int,const char*,const char*) is not defined\r\n");
            }
	    dlclose(dllib);  //CID:87033 - Resource leak
        }
        else {
            printf("dsSRV: Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    if (param != NULL) {
        param->result = dsERR_GENERAL;

        if (func != NULL) {
            param->result = func(param->handle, param->param_bytes, param->value_bytes);
        }
    }

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;

}

IARM_Result_t _dsIsOutputHDR(void *arg)
{
    dsIsOutputHDRParam_t *param = (dsIsOutputHDRParam_t*) arg;
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    printf("dsSRV::_dsIsOutputHDR\r\n");

    typedef dsError_t (*dsIsOutputHDR_t)(int handle, bool *hdr);
    static dsIsOutputHDR_t func = NULL;
    if (func == NULL) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib != NULL) {
            func = (dsIsOutputHDR_t) dlsym(dllib, "dsIsOutputHDR");
            if (func != NULL) {
                printf("dsSRV: dsIsOutputHDR(int handle, bool *hdr) is defined and loaded\r\n");
            }
            else {
                printf("dsSRV: dsIsOutputHDR(int handle, bool *hdr) is not defined\r\n");
            }
	    dlclose(dllib);   //CID:83623 - Resource leak
        }
        else {
            printf("dsSRV: Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    if (param != NULL) {
        param->result = dsERR_GENERAL;

        if (func != NULL) {
            param->result = func(param->handle, &param->hdr);
        }
    }


    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;

}

IARM_Result_t _dsResetOutputToSDR(void *arg)
{
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    printf("dsSRV::_dsResetOutputToSDR\r\n");

    typedef dsError_t (*dsResetOutputToSDR_t)();
    static dsResetOutputToSDR_t func = NULL;
    if (func == NULL) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib != NULL) {
            func = (dsResetOutputToSDR_t) dlsym(dllib, "dsResetOutputToSDR");
            if (func != NULL) {
                printf("dsSRV: dsResetOutputToSDR() is defined and loaded\r\n");
            }
            else {
                printf("dsSRV: dsResetOutputToSDR() is not defined\r\n");
            }
	    dlclose(dllib);  //CID:88069 - Resource leak
        }
        else {
            printf("dsSRV: Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

        if (func != NULL) {
            dsError_t result = func();
        }


    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;

}

IARM_Result_t _dsSetHdmiPreference(void *arg)
{
    dsSetHdmiPreferenceParam_t *param = (dsSetHdmiPreferenceParam_t*) arg;
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    printf("dsSRV::_dsSetHdmiPreference\r\n");

    typedef dsError_t (*dsSetHdmiPreference_t)(int handle, dsHdcpProtocolVersion_t *hdcpCurrentProtocol);
    static dsSetHdmiPreference_t func = NULL;
    if (func == NULL) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib != NULL) {
            func = (dsSetHdmiPreference_t) dlsym(dllib, "dsSetHdmiPreference");
            if (func != NULL) {
                printf("dsSRV: dsSetHdmiPreference(int handle, dsHdcpProtocolVersion_t *hdcpCurrentProtocol) is defined and loaded\r\n");
            }
            else {
                printf("dsSRV: dsSetHdmiPreference(int handle, dsHdcpProtocolVersion_t *hdcpCurrentProtocol) is not defined\r\n");
            }
	    dlclose(dllib);  //CID:83238 - Resource leak
        }
        else {
            printf("dsSRV: Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    if (param != NULL) {
        param->result = dsERR_GENERAL;

        if (func != NULL) {
            param->result = func(param->handle, &param->hdcpCurrentProtocol);
        }
    }


    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;

}

IARM_Result_t _dsGetHdmiPreference(void *arg)
{
    dsGetHdmiPreferenceParam_t *param = (dsGetHdmiPreferenceParam_t*) arg;
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    printf("dsSRV::_dsGetHdmiPreference\r\n");

    typedef dsError_t (*dsGetHdmiPreference_t)(int handle, dsHdcpProtocolVersion_t *hdcpCurrentProtocol);
    static dsGetHdmiPreference_t func = NULL;
    if (func == NULL) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib != NULL) {
            func = (dsGetHdmiPreference_t) dlsym(dllib, "dsGetHdmiPreference");
            if (func != NULL) {
                printf("dsSRV: dsGetHdmiPreference(int handle, dsHdcpProtocolVersion_t *hdcpCurrentProtocol) is defined and loaded\r\n");
            }
            else {
                printf("dsSRV: dsGetHdmiPreference(int handle, dsHdcpProtocolVersion_t *hdcpCurrentProtocol) is not defined\r\n");
            }
	    dlclose(dllib);   //CID:82165 - Resource leak
        }
        else {
            printf("dsSRV: Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    if (param != NULL) {
        param->result = dsERR_GENERAL;

        if (func != NULL) {
            param->result = func(param->handle, &param->hdcpCurrentProtocol);
        }
    }


    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;

}

IARM_Result_t _dsSetBackgroundColor(void *arg)
{
    dsSetBackgroundColorParam_t *param = (dsSetBackgroundColorParam_t*) arg;
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsSetBackgroundColor_t)(int handle, dsVideoBackgroundColor_t color);
    static dsSetBackgroundColor_t func = NULL;
    if (func == NULL) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib != NULL) {
            func = (dsSetBackgroundColor_t) dlsym(dllib, "dsSetBackgroundColor");
            if (func != NULL) {
                printf("dsSRV: dsError_t dsSetBackgroundColor(int handle, dsVideoBackgroundColor_t color)  is defined and loaded\r\n");
            }
            else {
                printf("dsSRV: dsError_t dsSetBackgroundColor(int handle, dsVideoBackgroundColor_t color) is not defined\r\n");
            }
	    dlclose(dllib);  //CID:86640 - Resource leak
        }
        else {
            printf("dsSRV: Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    if (param != NULL && func != NULL) {
          func(param->handle, param->color);
    }

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;

}


bool isComponentPortPresent()
{
    bool componentPortPresent = false;
    int numPorts,i;

    numPorts = dsUTL_DIM(kSupportedPortTypes);
    for(i=0; i< numPorts; i++)
    {
        if (kSupportedPortTypes[i] == dsVIDEOPORT_TYPE_COMPONENT)
        {
            componentPortPresent = true;;
        }
    }
    printf(" componentPortPresent :%d\n",componentPortPresent);
    return componentPortPresent;
}
/** @} */
/** @} */
