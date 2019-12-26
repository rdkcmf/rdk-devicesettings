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


#include "dsAudio.h"

#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <pthread.h>
#include <dlfcn.h>
#include "dsError.h"
#include "dsUtl.h"
#include "dsTypes.h"
#include "pthread.h"
#include "libIARM.h"
#include "libIBus.h"
#include "iarmUtil.h"
#include "dsRpc.h"
#include "dsMgr.h"
#include "hostPersistence.hpp"
#include "dsserverlogger.h"
#include "dsAudioSettings.h"

static int m_isInitialized = 0;
static int m_isPlatInitialized = 0;

static bool m_MS12DAPV2Enabled = 0;
static bool m_MS12DEEnabled = 0;
static bool m_LEEnabled = 0;

static pthread_mutex_t dsLock = PTHREAD_MUTEX_INITIALIZER;
int _srv_AudioAuto  = 0;
dsAudioStereoMode_t _srv_HDMI_Audiomode = dsAUDIO_STEREO_STEREO;
dsAudioStereoMode_t _srv_SPDIF_Audiomode = dsAUDIO_STEREO_STEREO;

#define IARM_BUS_Lock(lock) pthread_mutex_lock(&dsLock)
#define IARM_BUS_Unlock(lock) pthread_mutex_unlock(&dsLock)

IARM_Result_t _dsAudioPortInit(void *arg);
IARM_Result_t _dsGetAudioPort(void *arg);
IARM_Result_t _dsSetStereoMode(void *arg);
IARM_Result_t _dsSetStereoAuto(void *arg);
IARM_Result_t _dsGetStereoAuto(void *arg);
IARM_Result_t _dsSetAudioMute(void *arg);
IARM_Result_t _dsAudioPortTerm(void *arg);
IARM_Result_t _dsGetStereoMode(void *arg);
IARM_Result_t _dsGetEncoding(void *arg);
IARM_Result_t _dsIsAudioMSDecode(void *arg);
IARM_Result_t _dsIsAudioPortEnabled(void *arg);
IARM_Result_t _dsEnableAudioPort(void *arg);

IARM_Result_t _dsEnableMS12Config(void *arg);
IARM_Result_t _dsEnableLEConfig(void *arg);
IARM_Result_t _dsGetLEConfig(void *arg);
IARM_Result_t _dsSetAudioDelay(void *arg);
IARM_Result_t _dsGetAudioDelay(void *arg);
IARM_Result_t _dsSetAudioDelayOffset(void *arg);
IARM_Result_t _dsGetAudioDelayOffset(void *arg);
IARM_Result_t _dsGetSinkDeviceAtmosCapability(void *arg);
IARM_Result_t _dsSetAudioAtmosOutputMode(void *arg);

static void _GetAudioModeFromPersistent(void *arg);
static dsAudioPortType_t _GetAudioPortType(int handle);
static bool Is_Audio_MS12_Decode()
{
#ifndef RDK_DSHAL_NAME
    #warning   "RDK_DSHAL_NAME is not defined"
    #define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    bool isMS12Decode = false;
    typedef dsError_t  (*dsIsAudioMS12DecodeConfig_t)(bool *enable);
    static dsIsAudioMS12DecodeConfig_t func = NULL;
    if (func == NULL) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsIsAudioMS12DecodeConfig_t) dlsym(dllib, "dsIsAudioMS12Decode");
            if (func) {
                __TIMESTAMP();printf("dsIsAudioMS12Decode(bool *) is defined and loaded\r\n");
            }
            else {
                __TIMESTAMP();printf("dsIsAudioMS12Decode(bool *) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            __TIMESTAMP();printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    if (func != NULL)
    {
        dsError_t ret = func(&isMS12Decode);
    }

    __TIMESTAMP();printf("Is_Audio_MS12_Decode status:%d",isMS12Decode);
    return isMS12Decode;
}

void ms12ConfigInit()
{
    typedef dsError_t  (*dsEnableMS12Config_t)(int handle, dsMS12FEATURE_t feature,const bool enable);
    int handle = 0;
    if(Is_Audio_MS12_Decode() == false)
    {
    	return;
    }
    dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle);
    static dsEnableMS12Config_t func = NULL;
    if (func == NULL) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsEnableMS12Config_t) dlsym(dllib, "dsEnableMS12Config");
            if (func) {
                __TIMESTAMP();printf("dsEnableMS12Config(int, enum, bool) is defined and loaded\r\n");
            	std::string _MS12DAPV2Enable("FALSE");
            	try
            	{
            	    _MS12DAPV2Enable = device::HostPersistence::getInstance().getProperty("MS12.DAPV2Enable");
            	}
            	catch(...)
            	{
             		__TIMESTAMP();printf("MS12 : Persisting defaultMS12.DAPV2Enable  status: %s \r\n",_MS12DAPV2Enable.c_str());
            		device::HostPersistence::getInstance().persistHostProperty("MS12.DAPV2Enable",_MS12DAPV2Enable);
            	}
            	__TIMESTAMP();printf("MS12 : Persistence DAPV2 status: %s \r\n",_MS12DAPV2Enable.c_str());
            	if(_MS12DAPV2Enable == "TRUE")
            	{
            		m_MS12DAPV2Enabled = 1;
            	}
            	else
            	{
            		m_MS12DAPV2Enabled = 0;
            	}
            	func(handle,dsMS12FEATURE_DAPV2,m_MS12DAPV2Enabled);

            	std::string _MS12DEEnable("FALSE");
            	try
            	{
            	    _MS12DEEnable = device::HostPersistence::getInstance().getProperty("MS12.DEEnable");
            	}
            	catch(...)
            	{
             		__TIMESTAMP();printf("MS12 : Persisting default MS12.DEEnable  status: %s \r\n",_MS12DEEnable.c_str());
            		device::HostPersistence::getInstance().persistHostProperty("MS12.DEEnable",_MS12DEEnable);
            	}
            	__TIMESTAMP();printf("MS12 : Persistence DE status: %s \r\n",_MS12DEEnable.c_str());

            	if(_MS12DEEnable == "TRUE")
            	{
            		m_MS12DEEnabled = 1;
            	}
            	else
            	{
            		m_MS12DEEnabled = 0;
            	}
            	func(handle,dsMS12FEATURE_DE,m_MS12DEEnabled);
            }
            else {
                __TIMESTAMP();printf("dsEnableMS12Config(int, enum, bool) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            __TIMESTAMP();printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

}

void LEConfigInit()
{
    typedef dsError_t  (*dsEnableLEConfig_t)(int handle, const bool enable);
    int handle = 0;
    dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle);
    static dsEnableLEConfig_t func = NULL;
    if (func == NULL) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsEnableLEConfig_t) dlsym(dllib, "dsEnableLEConfig");
            if (func) {
                __TIMESTAMP();printf("dsEnableLEConfig(int, bool) is defined and loaded\r\n");
                std::string _LEEnable("FALSE");
                try
                {
                    _LEEnable = device::HostPersistence::getInstance().getProperty("audio.LEEnable");
                }
                catch(...)
                {
#ifndef DS_LE_DEFAULT_DISABLED
                    _LEEnable = "TRUE";
#endif
                   __TIMESTAMP();printf("LE : Persisting default LE status: %s \r\n",_LEEnable.c_str());
                   device::HostPersistence::getInstance().persistHostProperty("audio.LEEnable",_LEEnable);
                }
                if(_LEEnable == "TRUE")
                {
                    m_LEEnabled = 1;
                    func(handle,m_LEEnabled);
                }
                else
                {
                    m_LEEnabled = 0;
                    func(handle,m_LEEnabled);
                }
            }
            else {
                __TIMESTAMP();printf("dsEnableLEConfig(int,  bool) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            __TIMESTAMP();printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

}

IARM_Result_t dsAudioMgr_init()
{
   try
	{
		/* Get the AudioModesettings FOR HDMI from Persistence */
		std::string _AudioModeSettings("STEREO");
		_AudioModeSettings = device::HostPersistence::getInstance().getProperty("HDMI0.AudioMode",_AudioModeSettings);
		__TIMESTAMP();printf("The HDMI Audio Mode Setting on startup  is %s \r\n",_AudioModeSettings.c_str());
		if (_AudioModeSettings.compare("SURROUND") == 0)
		{
			_srv_HDMI_Audiomode = dsAUDIO_STEREO_SURROUND;
		}
		else if (_AudioModeSettings.compare("PASSTHRU") == 0)
		{
			_srv_HDMI_Audiomode = dsAUDIO_STEREO_PASSTHRU;
		}
        else 
        {
			_srv_HDMI_Audiomode = dsAUDIO_STEREO_STEREO;
        }

		/* Get the AutoModesettings FOR HDMI from Persistence */
                /* If HDMI persistence is surround, Auto defaults to true */
            std::string _AudioModeAuto("FALSE");

            #if 0
               /* 
                Commenting this to fix the persistent settings
                Audio mode should not be forced to Auto 
                To enabale this we need to change the DS Mgr implementation 
                which reads the _dsGetStereoMode to  know the persistent value...*/
                if (_srv_HDMI_Audiomode == dsAUDIO_STEREO_SURROUND) 
                {
                    _AudioModeAuto = "TRUE";
                }
            #endif
          
	    _AudioModeAuto = device::HostPersistence::getInstance().getProperty("HDMI0.AudioMode.AUTO",_AudioModeAuto);
	    if (_AudioModeAuto.compare("TRUE") == 0)
	    {
	        _srv_AudioAuto = 1;
	    }
        else 
        {
			_srv_AudioAuto = 0;
        }
		__TIMESTAMP();printf("The HDMI Audio Auto Setting on startup  is %s \r\n",_AudioModeAuto.c_str());
		
		/* Get the AudioModesettings for SPDIF from Persistence */
		std::string _SPDIFModeSettings("STEREO");
		_SPDIFModeSettings = device::HostPersistence::getInstance().getProperty("SPDIF0.AudioMode",_SPDIFModeSettings);
		__TIMESTAMP();printf("The SPDIF Audio Mode Setting on startup  is %s \r\n",_SPDIFModeSettings.c_str());
		if (_SPDIFModeSettings.compare("SURROUND") == 0)
		{
			_srv_SPDIF_Audiomode = dsAUDIO_STEREO_SURROUND;
		}
		else if (_SPDIFModeSettings.compare("PASSTHRU") == 0)
		{
			_srv_SPDIF_Audiomode = dsAUDIO_STEREO_PASSTHRU;
		}
        else 
        {
			_srv_SPDIF_Audiomode = dsAUDIO_STEREO_STEREO;
        }
	}
	catch(...) 
	{
		printf("Exception in Getting the Audio  settings on Startup..... \r\n");
	}
    	if (!m_isPlatInitialized) {
    		dsAudioPortInit();
    		ms12ConfigInit();
                LEConfigInit();
	   	}
        /*coverity[missing_lock]  CID-19380 using Coverity Annotation to ignore error*/
        m_isPlatInitialized ++;

	IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsAudioPortInit, _dsAudioPortInit);
    return IARM_RESULT_SUCCESS;
}



IARM_Result_t dsAudioMgr_term()
{
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsAudioPortInit(void *arg)
{
    IARM_BUS_Lock(lock);

    if (!m_isInitialized) {

        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetAudioPort,_dsGetAudioPort);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetStereoMode,_dsSetStereoMode);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetStereoMode,_dsGetStereoMode);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetStereoAuto,_dsSetStereoAuto);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetStereoAuto,_dsGetStereoAuto);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetAudioMute,_dsSetAudioMute);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetEncoding,_dsGetEncoding);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsIsAudioMSDecode,_dsIsAudioMSDecode);

        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsIsAudioPortEnabled,_dsIsAudioPortEnabled);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsEnableAudioPort,_dsEnableAudioPort);

        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsAudioPortTerm,_dsAudioPortTerm);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsEnableMS12Config,_dsEnableMS12Config);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsEnableLEConfig,_dsEnableLEConfig);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetLEConfig,_dsGetLEConfig);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetAudioDelay, _dsSetAudioDelay);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetAudioDelay, _dsGetAudioDelay);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetAudioDelayOffset, _dsSetAudioDelayOffset);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetAudioDelayOffset, _dsGetAudioDelayOffset);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetSinkDeviceAtmosCapability, _dsGetSinkDeviceAtmosCapability);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetAudioAtmosOutputMode, _dsSetAudioAtmosOutputMode);      

        m_isInitialized = 1;
    }

    if (!m_isPlatInitialized) {
        /* Nexus init, if any here */
        dsAudioPortInit();
        ms12ConfigInit();
        LEConfigInit();
   }
   m_isPlatInitialized++;


    IARM_BUS_Unlock(lock);

 return IARM_RESULT_SUCCESS;

}

IARM_Result_t _dsGetAudioPort(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    dsAudioGetHandleParam_t *param = (dsAudioGetHandleParam_t *)arg;

    if (param != NULL)
    {
        printf("%s..%d-%d \r\n",__func__,param->type,param->index);
        dsGetAudioPort(param->type, param->index, &param->handle);
    }

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}



IARM_Result_t _dsGetStereoMode(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    dsAudioSetStereoModeParam_t *param = (dsAudioSetStereoModeParam_t *)arg;

    if (param != NULL && param->toPersist) {
        _GetAudioModeFromPersistent(arg);
    }
    else if (param != NULL)
    {
        /* In Auto Mode, get the effective mode */
        if (_srv_AudioAuto) {
            dsAudioStereoMode_t stereoMode = dsAUDIO_STEREO_UNKNOWN;
            dsGetStereoMode(param->handle, &stereoMode);
            param->mode = stereoMode;
        }
        else {
            dsAudioPortType_t _APortType = _GetAudioPortType(param->handle);
            if (_APortType == dsAUDIOPORT_TYPE_SPDIF)
            {
                param->mode = _srv_SPDIF_Audiomode;
                printf("The SPDIF Port Audio Settings Mode is %d \r\n",param->mode);
            }
            else if (_APortType == dsAUDIOPORT_TYPE_HDMI) {
                param->mode = _srv_HDMI_Audiomode;
                printf("The HDMI Port Audio Settings Mode is %d \r\n",param->mode);
            } 
        } 
   }

    IARM_BUS_Unlock(lock);    

    return IARM_RESULT_SUCCESS;
}


IARM_Result_t _dsSetStereoMode(void *arg)
{
    _DEBUG_ENTER();
    IARM_Bus_DSMgr_EventData_t eventData;

    IARM_BUS_Lock(lock);

    dsError_t ret = dsERR_NONE;
    dsAudioSetStereoModeParam_t *param = (dsAudioSetStereoModeParam_t *)arg;

    ret = dsSetStereoMode(param->handle, param->mode);
    param->rpcResult = ret;

    if (ret == dsERR_NONE)
    {
        dsAudioPortType_t _APortType = _GetAudioPortType(param->handle);
        try
        {
            if(param->mode == dsAUDIO_STEREO_STEREO)
            {
                __TIMESTAMP();printf("Setting Audio Mode STEREO with persistent value : %d \r\n",param->toPersist);

                if (_APortType == dsAUDIOPORT_TYPE_HDMI)
                {
                    if (param->toPersist)
                    device::HostPersistence::getInstance().persistHostProperty("HDMI0.AudioMode","STEREO");
                
                    _srv_HDMI_Audiomode = dsAUDIO_STEREO_STEREO;
                }
                else if (_APortType == dsAUDIOPORT_TYPE_SPDIF)
                {
                    if (param->toPersist)
                    device::HostPersistence::getInstance().persistHostProperty("SPDIF0.AudioMode","STEREO");
                
                    _srv_SPDIF_Audiomode = dsAUDIO_STEREO_STEREO;
                }
                eventData.data.Audioport.mode = dsAUDIO_STEREO_STEREO;
                eventData.data.Audioport.type = _APortType;
                IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,(IARM_EventId_t)IARM_BUS_DSMGR_EVENT_AUDIO_MODE,(void *)&eventData, sizeof(eventData));

            }
            else if(param->mode == dsAUDIO_STEREO_SURROUND)
            {
                __TIMESTAMP();printf("Setting Audio Mode SURROUND with persistent value %d \r\n",param->toPersist);

                if (_APortType == dsAUDIOPORT_TYPE_HDMI)
                {
                    if (param->toPersist)
                    device::HostPersistence::getInstance().persistHostProperty("HDMI0.AudioMode","SURROUND");
                
                    _srv_HDMI_Audiomode = dsAUDIO_STEREO_SURROUND;
                }
                else if (_APortType == dsAUDIOPORT_TYPE_SPDIF)
                {
                    if (param->toPersist)
                    device::HostPersistence::getInstance().persistHostProperty("SPDIF0.AudioMode","SURROUND");

                    _srv_SPDIF_Audiomode = dsAUDIO_STEREO_SURROUND;
                }

                eventData.data.Audioport.mode = dsAUDIO_STEREO_SURROUND;
                eventData.data.Audioport.type = _APortType;
                IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,(IARM_EventId_t)IARM_BUS_DSMGR_EVENT_AUDIO_MODE,(void *)&eventData, sizeof(eventData));
            }
            else if(param->mode == dsAUDIO_STEREO_PASSTHRU)
            {
                __TIMESTAMP();printf("Setting Audio Mode PASSTHRU with persistent value %d \r\n",param->toPersist);

                if (_APortType == dsAUDIOPORT_TYPE_HDMI)
                {
                    if (param->toPersist)
                    device::HostPersistence::getInstance().persistHostProperty("HDMI0.AudioMode","PASSTHRU");
                    _srv_HDMI_Audiomode = dsAUDIO_STEREO_PASSTHRU;
                }
                else if (_APortType == dsAUDIOPORT_TYPE_SPDIF)
                {
                    if (param->toPersist)
                    device::HostPersistence::getInstance().persistHostProperty("SPDIF0.AudioMode","PASSTHRU");
                    _srv_SPDIF_Audiomode = dsAUDIO_STEREO_PASSTHRU;
                }

                eventData.data.Audioport.mode = dsAUDIO_STEREO_PASSTHRU;
                eventData.data.Audioport.type = _APortType;
                IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,(IARM_EventId_t)IARM_BUS_DSMGR_EVENT_AUDIO_MODE,(void *)&eventData, sizeof(eventData));

            }
        }
        catch(...)
        {
            printf("Error in Setting audio mode... \r\n");
        }

    }

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsGetStereoAuto(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    dsAudioSetStereoAutoParam_t *param = (dsAudioSetStereoAutoParam_t *)arg;

    if (param != NULL)
    {
        param->autoMode = (_srv_AudioAuto ? 1 : 0);
    }

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsSetStereoAuto(void *arg)
{
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    dsAudioSetStereoAutoParam_t *param = (dsAudioSetStereoAutoParam_t *)arg;

    if (param->toPersist) {
        device::HostPersistence::getInstance().persistHostProperty("HDMI0.AudioMode.AUTO", param->autoMode ? "TRUE" : "FALSE");
    }

    _srv_AudioAuto = param->autoMode ? 1 : 0;

    IARM_BUS_Unlock(lock);
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsSetAudioMute(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    dsAudioSetMutedParam_t *param = (dsAudioSetMutedParam_t *)arg;
    dsSetAudioMute(param->handle, param->mute);

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}


IARM_Result_t _dsIsAudioMute(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    dsAudioSetMutedParam_t *param = (dsAudioSetMutedParam_t *)arg;
    bool muted = false;
    
    dsError_t ret = dsIsAudioMute(param->handle, &muted);
    if (ret == dsERR_NONE) {
        param->mute = muted;
    }

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}


IARM_Result_t _dsIsAudioPortEnabled(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    dsAudioPortEnabledParam_t *param = (dsAudioPortEnabledParam_t *)arg;
    bool enabled = false;
    
    dsError_t ret = dsIsAudioPortEnabled(param->handle, &enabled);
    if (ret == dsERR_NONE) {
        param->enabled = enabled;
    }

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}


IARM_Result_t _dsEnableAudioPort(void *arg)
{
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);
   
    dsAudioPortEnabledParam_t *param = (dsAudioPortEnabledParam_t *)arg;
    dsEnableAudioPort(param->handle, param->enabled);
   
    IARM_BUS_Unlock(lock);
    
    return IARM_RESULT_SUCCESS;
}



IARM_Result_t _dsAudioPortTerm(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    m_isPlatInitialized--;

    if (0 == m_isPlatInitialized)
    {
        dsAudioPortTerm();
    }

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsGetEncoding(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    dsAudioGetEncodingModeParam_t *param = (dsAudioGetEncodingModeParam_t *)arg;

    if (param != NULL)
    {

        dsAudioEncoding_t _encoding = dsAUDIO_ENC_NONE;
        dsGetAudioEncoding(param->handle, &_encoding);
        param->encoding = _encoding;


     __TIMESTAMP();printf("param->encoding = %d\r\n",_encoding);
    
    }

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}

static dsAudioPortType_t _GetAudioPortType(int handle)
{
    int numPorts,i;
    int halhandle = 0;

    numPorts = dsUTL_DIM(kSupportedPortTypes);
    
    for(i=0; i< numPorts; i++)
    {
        dsGetAudioPort(kPorts[i].id.type, kPorts[i].id.index, &halhandle);
        if (handle == halhandle)
        {
            return kPorts[i].id.type;
        }
    }
    __TIMESTAMP();printf("Error: The Requested Audio Port is not part of Platform Port Configuration \r\n");
    return dsAUDIOPORT_TYPE_MAX;
}


IARM_Result_t _dsIsAudioMSDecode(void *arg)
{

    IARM_BUS_Unlock(lock);

#ifndef RDK_DSHAL_NAME
    #warning   "RDK_DSHAL_NAME is not defined"
    #define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);
    
    typedef dsError_t  (*dsIsAudioMSDecode_t)(int handle, bool *HasMS11Decode);
    static dsIsAudioMSDecode_t func = NULL;
    if (func == NULL) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsIsAudioMSDecode_t) dlsym(dllib, "dsIsAudioMSDecode");
            if (func) {
                __TIMESTAMP();printf("dsIsAudioMSDecode(int, bool*) is defined and loaded\r\n");
            }   
            else {
                __TIMESTAMP();printf("dsIsAudioMSDecode(int, bool*) is not defined\r\n");
            }   
            dlclose(dllib);
        }   
        else {
            __TIMESTAMP();printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }   
    }   

    dsAudioGetMS11Param_t *param = (dsAudioGetMS11Param_t *)arg;
    if (func != NULL) {
        bool HasMS11Decode = false;
        dsError_t ret = func(param->handle, &HasMS11Decode);
        if (ret == dsERR_NONE) {
            param->ms11Enabled = HasMS11Decode;
        }
    }else {
        param->ms11Enabled = false;
    }

    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsEnableMS12Config(void *arg)
{

#ifndef RDK_DSHAL_NAME
    #warning   "RDK_DSHAL_NAME is not defined"
    #define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;

    IARM_BUS_Lock(lock);

    typedef dsError_t  (*dsEnableMS12Config_t)(int handle, dsMS12FEATURE_t feature,const bool enable);
    static dsEnableMS12Config_t func = NULL;
    if (func == NULL) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsEnableMS12Config_t) dlsym(dllib, "dsEnableMS12Config");
            if (func) {
                __TIMESTAMP();printf("dsEnableMS12Config(int, enum, bool) is defined and loaded\r\n");
            }
            else {
                __TIMESTAMP();printf("dsEnableMS12Config(int, enum, bool) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            __TIMESTAMP();printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    _dsMS12ConfigParam_t *param = (_dsMS12ConfigParam_t *)arg;
    if ((func != NULL) && (Is_Audio_MS12_Decode() == true)) {
    	__TIMESTAMP();printf("MS12: %s feature :%s enable status:%d \r\n",__FUNCTION__,((param->feature==dsMS12FEATURE_DAPV2)?"DAPV2":"DE"),param->enable);

        if((param->feature == dsMS12FEATURE_DAPV2)  && (param->enable != m_MS12DAPV2Enabled) )
        {
            m_MS12DAPV2Enabled = param->enable;
            //Persist DAPV2 setting
            if(m_MS12DAPV2Enabled)
                device::HostPersistence::getInstance().persistHostProperty("MS12.DAPV2Enable","TRUE");
            else
                device::HostPersistence::getInstance().persistHostProperty("MS12.DAPV2Enable","FALSE");

            dsError_t ret = func(param->handle, param->feature, param->enable);
            if (ret == dsERR_NONE) {
            	result = IARM_RESULT_SUCCESS;
            }
        }
        if((param->feature == dsMS12FEATURE_DE)  && (param->enable != m_MS12DEEnabled) )
        {
            m_MS12DEEnabled = param->enable;
            //Persist DE setting
            if(m_MS12DEEnabled)
                device::HostPersistence::getInstance().persistHostProperty("MS12.DEEnable","TRUE");
            else
                device::HostPersistence::getInstance().persistHostProperty("MS12.DEEnable","FALSE");

            dsError_t ret = func(param->handle, param->feature, param->enable);
            if (ret == dsERR_NONE) {
            	result = IARM_RESULT_SUCCESS;
            }
        }

    }else {
    }

    IARM_BUS_Unlock(lock);

    return result;
}

IARM_Result_t _dsSetAudioDelay(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsSetAudioDelay_t)(int handle, uint32_t audioDelayMs);
    static dsSetAudioDelay_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetAudioDelay_t) dlsym(dllib, "dsSetAudioDelay");
            if (func) {
                printf("dsSetAudioDelay_t(int, uint32_t) is defined and loaded\r\n");
            }
            else {
                printf("dsSetAudioDelay_t(int, uint32_t) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsSetAudioDelayParam_t *param = (dsSetAudioDelayParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        if (func(param->handle, param->audioDelayMs) != dsERR_NONE)
        {
            __TIMESTAMP();printf("%s: (SERVER) Unable to set audiodelay\n", __FUNCTION__);
        }
    }

    IARM_BUS_Unlock(lock);
    return IARM_RESULT_SUCCESS;

}

IARM_Result_t _dsGetAudioDelay(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetAudioDelay_t)(int handle, uint32_t *audioDelayMs);
    static dsGetAudioDelay_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetAudioDelay_t) dlsym(dllib, "dsGetAudioDelay");
            if (func) {
                printf("dsGetAudioDelay_t(int, uint32_t*) is defined and loaded\r\n");
            }
            else {
                printf("dsGetAudioDelay_t(int, uint32_t*) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsGetAudioDelayParam_t *param = (dsGetAudioDelayParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        uint32_t audioDelayMs = 0;
        param->audioDelayMs = 0;
        if (func(param->handle, &audioDelayMs) == dsERR_NONE)
        {
            param->audioDelayMs = audioDelayMs;
        }
    }

    IARM_BUS_Unlock(lock);
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsSetAudioDelayOffset(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsSetAudioDelayOffset_t)(int handle, uint32_t audioDelayOffsetMs);
    static dsSetAudioDelayOffset_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetAudioDelayOffset_t) dlsym(dllib, "dsSetAudioDelayOffset");
            if (func) {
                printf("dsSetAudioDelayOffset_t(int, uint32_t) is defined and loaded\r\n");
            }
            else {
                printf("dsSetAudioDelayOffset_t(int, uint32_t) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsAudioDelayOffsetParam_t *param = (dsAudioDelayOffsetParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        if (func(param->handle, param->audioDelayOffsetMs) != dsERR_NONE)
        {
            __TIMESTAMP();printf("%s: (SERVER) Unable to set audiodelay offset\n", __FUNCTION__);
        }
    }

    IARM_BUS_Unlock(lock);
    return IARM_RESULT_SUCCESS;

}

IARM_Result_t _dsGetAudioDelayOffset(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetAudioDelayOffset_t)(int handle, uint32_t *audioDelayOffsetMs);
    static dsGetAudioDelayOffset_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetAudioDelayOffset_t) dlsym(dllib, "dsGetAudioDelayOffset");
            if (func) {
                printf("dsGetAudioDelayOffset_t(int, uint32_t*) is defined and loaded\r\n");
            }
            else {
                printf("dsGetAudioDelayOffset_t(int, uint32_t*) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsAudioDelayOffsetParam_t *param = (dsAudioDelayOffsetParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        uint32_t audioDelayOffsetMs = 0;
        param->audioDelayOffsetMs = 0;
        if (func(param->handle, &audioDelayOffsetMs) == dsERR_NONE)
        {
            param->audioDelayOffsetMs = audioDelayOffsetMs;
        }
    }

    IARM_BUS_Unlock(lock);
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsSetAudioAtmosOutputMode(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsSetAudioAtmosOutputMode_t)(int handle, bool enable);
    static dsSetAudioAtmosOutputMode_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetAudioAtmosOutputMode_t) dlsym(dllib, "dsSetAudioAtmosOutputMode");
            if (func) {
                printf("dsSetAudioAtmosOutputMode_t (int handle, bool enable ) is defined and loaded\r\n");
            }
            else {
                printf("dsSetAudioAtmosOutputMode_t (int handle, bool enable) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsAudioSetAtmosOutputModeParam_t *param = (dsAudioSetAtmosOutputModeParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        func(param->handle, param->enable);
    }

    IARM_BUS_Unlock(lock);
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsGetSinkDeviceAtmosCapability(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetSinkDeviceAtmosCapability_t)(int handle, dsATMOSCapability_t *capability);
    static dsGetSinkDeviceAtmosCapability_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetSinkDeviceAtmosCapability_t) dlsym(dllib, "dsGetSinkDeviceAtmosCapability");
            if (func) {
                printf("dsGetSinkDeviceAtmosCapability_t (int handle, dsATMOSCapability_t *capability ) is defined and loaded\r\n");
            }
            else {
                printf("dsGetSinkDeviceAtmosCapability_t (int handle, dsATMOSCapability_t *capability ) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsGetAudioAtmosCapabilityParam_t *param = (dsGetAudioAtmosCapabilityParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        dsATMOSCapability_t capability = dsAUDIO_ATMOS_NOTSUPPORTED;

        param->capability= dsAUDIO_ATMOS_NOTSUPPORTED;
        if (func(param->handle, &capability) == dsERR_NONE)
        {
            param->capability = capability;
        }
    }

    IARM_BUS_Unlock(lock);
    return IARM_RESULT_SUCCESS;
}


IARM_Result_t _dsEnableLEConfig(void *arg)
{

#ifndef RDK_DSHAL_NAME
    #warning   "RDK_DSHAL_NAME is not defined"
    #define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;

    IARM_BUS_Lock(lock);

    typedef dsError_t  (*dsEnableLEConfig_t)(int handle,const bool enable);
    static dsEnableLEConfig_t func = NULL;
    if (func == NULL) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsEnableLEConfig_t) dlsym(dllib, "dsEnableLEConfig");
            if (func) {
                __TIMESTAMP();printf("dsEnableLEConfig(int, bool) is defined and loaded\r\n");
            }
            else {
                __TIMESTAMP();printf("dsEnableLEConfig(int, bool) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            __TIMESTAMP();printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    _dsLEConfigParam_t *param = (_dsLEConfigParam_t *)arg;
    if (func != NULL) {
        __TIMESTAMP();printf("LE: %s  enable status:%d \r\n",__FUNCTION__,param->enable);

        if(param->enable != m_LEEnabled)
        {
            m_LEEnabled = param->enable;
            //Persist DAPV2 setting
            if(m_LEEnabled)
                device::HostPersistence::getInstance().persistHostProperty("audio.LEEnable","TRUE");
            else
                device::HostPersistence::getInstance().persistHostProperty("audio.LEEnable","FALSE");

            dsError_t ret = func(param->handle, param->enable);
            if (ret == dsERR_NONE) {
                result = IARM_RESULT_SUCCESS;
            }
        }
    }else {
    }

    IARM_BUS_Unlock(lock);

    return result;
}

IARM_Result_t _dsGetLEConfig(void *arg)
{

#ifndef RDK_DSHAL_NAME
    #warning   "RDK_DSHAL_NAME is not defined"
    #define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    typedef dsError_t  (*dsGetLEConfig_t)(int handle, bool *enable);
    static dsGetLEConfig_t func = NULL;
    if (func == NULL) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetLEConfig_t) dlsym(dllib, "dsGetLEConfig");
            if (func) {
                __TIMESTAMP();printf("dsGetLEConfig(int , bool *) is defined and loaded\r\n");
            }
            else {
                __TIMESTAMP();printf("dsGetLEConfig(int , bool *) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            __TIMESTAMP();printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsGetLEConfigParam_t *param = (dsGetLEConfigParam_t*) arg;
    if (param != NULL) {
        param->result = dsERR_GENERAL;

        if (func != NULL) {
            param->result = func(param->handle, &param->enable);
        }
    }


    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;

}

static void _GetAudioModeFromPersistent(void *arg)
{
    _DEBUG_ENTER();

    dsAudioSetStereoModeParam_t *param = (dsAudioSetStereoModeParam_t *)arg;

    if (param != NULL)
    {
        dsAudioPortType_t _APortType = _GetAudioPortType(param->handle);
        std::string _AudioModeSettings("STEREO");

        if (_APortType == dsAUDIOPORT_TYPE_SPDIF)
        {   
           _AudioModeSettings = device::HostPersistence::getInstance().getProperty("SPDIF0.AudioMode",_AudioModeSettings);
           __TIMESTAMP();printf("The SPDIF Audio Mode Setting From Persistent is %s \r\n",_AudioModeSettings.c_str());
        }
        else if (_APortType == dsAUDIOPORT_TYPE_HDMI) {
            _AudioModeSettings = device::HostPersistence::getInstance().getProperty("HDMI0.AudioMode",_AudioModeSettings);
            __TIMESTAMP();printf("The HDMI Audio Mode Setting From Persistent is %s \r\n",_AudioModeSettings.c_str());
        }

        if (_AudioModeSettings.compare("SURROUND") == 0)
        {
            param->mode = dsAUDIO_STEREO_SURROUND;
        }
        else if (_AudioModeSettings.compare("PASSTHRU") == 0)
        {
            param->mode = dsAUDIO_STEREO_PASSTHRU;
        }
        else 
        {
            param->mode = dsAUDIO_STEREO_STEREO;
        } 
    }
}


/** @} */
/** @} */
