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

#include "safec_lib.h"

static int m_isInitialized = 0;
static int m_isPlatInitialized = 0;

static bool m_MS12DAPV2Enabled = 0;
static bool m_MS12DEEnabled = 0;
static bool m_LEEnabled = 0;
static int m_volumeDuckingLevel = 0;
static float m_volumeLevel = 0;
static int m_MuteStatus = false;
static int m_isDuckingInProgress = false;

static pthread_mutex_t dsLock = PTHREAD_MUTEX_INITIALIZER;
int _srv_AudioAuto  = 0;
dsAudioStereoMode_t _srv_HDMI_Audiomode = dsAUDIO_STEREO_STEREO;
dsAudioStereoMode_t _srv_SPDIF_Audiomode = dsAUDIO_STEREO_STEREO;
dsAudioStereoMode_t _srv_HDMI_ARC_Audiomode = dsAUDIO_STEREO_STEREO;

#define IARM_BUS_Lock(lock) pthread_mutex_lock(&dsLock)
#define IARM_BUS_Unlock(lock) pthread_mutex_unlock(&dsLock)

IARM_Result_t _dsAudioPortInit(void *arg);
IARM_Result_t _dsGetAudioPort(void *arg);
IARM_Result_t _dsGetSupportedARCTypes(void *arg);
IARM_Result_t _dsAudioSetSAD(void *arg);
IARM_Result_t _dsAudioEnableARC(void *arg);
IARM_Result_t _dsSetStereoMode(void *arg);
IARM_Result_t _dsSetStereoAuto(void *arg);
IARM_Result_t _dsGetStereoAuto(void *arg);
IARM_Result_t _dsSetAudioMute(void *arg);
IARM_Result_t _dsIsAudioMute(void *arg);
IARM_Result_t _dsAudioPortTerm(void *arg);
IARM_Result_t _dsGetStereoMode(void *arg);
IARM_Result_t _dsGetAudioFormat(void *arg);
IARM_Result_t _dsGetEncoding(void *arg);
IARM_Result_t _dsIsAudioMSDecode(void *arg);
IARM_Result_t _dsIsAudioMS12Decode(void *arg);
IARM_Result_t _dsIsAudioPortEnabled(void *arg);


IARM_Result_t _dsGetEnablePersist(void *arg);
IARM_Result_t _dsSetEnablePersist(void *arg);

IARM_Result_t _dsEnableAudioPort(void *arg);
IARM_Result_t _dsSetAudioDucking(void *arg);
IARM_Result_t _dsGetAudioLevel(void *arg);
IARM_Result_t _dsSetAudioLevel(void *arg);
IARM_Result_t _dsGetAudioGain(void *arg);
IARM_Result_t _dsSetAudioGain(void *arg);
IARM_Result_t _dsEnableLEConfig(void *arg);
IARM_Result_t _dsGetLEConfig(void *arg);
IARM_Result_t _dsSetAudioDelay(void *arg);
IARM_Result_t _dsGetAudioDelay(void *arg);
IARM_Result_t _dsSetAudioDelayOffset(void *arg);
IARM_Result_t _dsGetAudioDelayOffset(void *arg);
IARM_Result_t _dsGetSinkDeviceAtmosCapability(void *arg);
IARM_Result_t _dsSetAudioAtmosOutputMode(void *arg);

IARM_Result_t _dsSetAudioCompression(void *arg);
IARM_Result_t _dsGetAudioCompression(void *arg);
IARM_Result_t _dsSetDialogEnhancement(void *arg);
IARM_Result_t _dsGetDialogEnhancement(void *arg);
IARM_Result_t _dsSetDolbyVolumeMode(void *arg);
IARM_Result_t _dsGetDolbyVolumeMode(void *arg);
IARM_Result_t _dsSetIntelligentEqualizerMode(void *arg);
IARM_Result_t _dsGetIntelligentEqualizerMode(void *arg);
IARM_Result_t _dsSetGraphicEqualizerMode(void *arg);
IARM_Result_t _dsGetGraphicEqualizerMode(void *arg);

IARM_Result_t _dsGetVolumeLeveller(void *arg);
IARM_Result_t _dsSetVolumeLeveller(void *arg);
IARM_Result_t _dsGetBassEnhancer(void *arg);
IARM_Result_t _dsSetBassEnhancer(void *arg);
IARM_Result_t _dsIsSurroundDecoderEnabled(void *arg);
IARM_Result_t _dsEnableSurroundDecoder(void *arg);
IARM_Result_t _dsGetDRCMode(void *arg);
IARM_Result_t _dsSetDRCMode(void *arg);
IARM_Result_t _dsGetSurroundVirtualizer(void *arg);
IARM_Result_t _dsSetSurroundVirtualizer(void *arg);
IARM_Result_t _dsGetMISteering(void *arg);
IARM_Result_t _dsSetMISteering(void *arg);
IARM_Result_t _dsGetMS12AudioProfileList(void *arg);
IARM_Result_t _dsGetMS12AudioProfile(void *arg);
IARM_Result_t _dsSetMS12AudioProfile(void *arg);

IARM_Result_t _dsGetAssociatedAudioMixing(void *arg);
IARM_Result_t _dsSetAssociatedAudioMixing(void *arg);
IARM_Result_t _dsGetFaderControl(void *arg);
IARM_Result_t _dsSetFaderControl(void *arg);
IARM_Result_t _dsGetPrimaryLanguage(void *arg);
IARM_Result_t _dsSetPrimaryLanguage(void *arg);
IARM_Result_t _dsGetSecondaryLanguage(void *arg);
IARM_Result_t _dsSetSecondaryLanguage(void *arg);

IARM_Result_t _dsGetAudioCapabilities(void *arg);
IARM_Result_t _dsGetMS12Capabilities(void *arg);
IARM_Result_t _dsAudioOutIsConnected(void *arg);
IARM_Result_t _dsResetBassEnhancer(void *arg);
IARM_Result_t _dsResetSurroundVirtualizer(void *arg);
IARM_Result_t _dsResetVolumeLeveller(void *arg);
IARM_Result_t _dsResetDialogEnhancement(void *arg);
IARM_Result_t _dsSetMS12SetttingsOverride(void *arg);
IARM_Result_t _dsGetHDMIARCPortId(void *arg);

static void _GetAudioModeFromPersistent(void *arg);
static dsAudioPortType_t _GetAudioPortType(int handle);
void _dsAudioOutPortConnectCB(dsAudioPortType_t portType, unsigned int uiPortNo, bool isPortConnected);
static dsError_t _dsAudioOutRegisterConnectCB (dsAudioOutPortConnectCB_t cbFun);
void _dsAudioFormatUpdateCB(dsAudioFormat_t audioFormat);
static dsError_t _dsAudioFormatUpdateRegisterCB (dsAudioFormatUpdateCB_t cbFun);
static std::string _dsGetCurrentProfileProperty(std::string property);
static void _dsMS12ProfileSettingOverride(int handle);
static std::string _dsGenerateProfileProperty(std::string profile,std::string property);
static bool _dsMs12ProfileSupported(int handle,std::string profile);
static IARM_Result_t _resetDialogEnhancerLevel(int handle);
static IARM_Result_t _resetBassEnhancer(int handle);
static IARM_Result_t _resetVolumeLeveller(int handle);
static IARM_Result_t _resetSurroundVirtualizer(int handle);
static IARM_Result_t _setDialogEnhancement(int handle, int enhancerLevel);
static IARM_Result_t _setBassEnhancer(int handle ,int boost);
static IARM_Result_t _setVolumeLeveller(int handle, int volLevellerMode, int volLevellerLevel);
static IARM_Result_t _setSurroundVirtualizer(int handle , int virtualizerMode , int virtualizerBoost);

void AudioConfigInit()
{
    typedef dsError_t  (*dsEnableLEConfig_t)(int handle, const bool enable);
    int handle = 0;
    void *dllib = NULL;
    static dsEnableLEConfig_t func = NULL;
    if (dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
      if (func == NULL) {
        dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
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
    else {
        __TIMESTAMP();printf("dsEnableLEConfig(int,  bool) is failed. since dsAUDIOPORT_TYPE_HDMI 0 port not available\r\n");
    }

#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
    typedef dsError_t (*dsSetAudioGain_t)(int handle, float gain);
    static dsSetAudioGain_t dsSetAudioGainFunc = 0;
    if (dsSetAudioGainFunc == 0) {
        dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            dsSetAudioGainFunc = (dsSetAudioGain_t) dlsym(dllib, "dsSetAudioGain");
            if (dsSetAudioGainFunc) {
                printf("dsSetAudioGain_t(int, float ) is defined and loaded\r\n");
                std::string _AudioGain("0");
                float m_audioGain = 0;
//SPEAKER init
                handle = 0;
                if (dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                    try {
                        _AudioGain = device::HostPersistence::getInstance().getProperty("SPEAKER0.audio.Gain");
                    }
                    catch(...) {
                            try {
                                printf("SPEAKER0.audio.Gain not found in persistence store. Try system default\n");
                                _AudioGain = device::HostPersistence::getInstance().getDefaultProperty("SPEAKER0.audio.Gain");
                            }
                            catch(...) {
                                _AudioGain = "0";
                            }
                    }
                    m_audioGain = atof(_AudioGain.c_str());
                    if (dsSetAudioGainFunc(handle, m_audioGain) == dsERR_NONE) {
                        printf("Port %s: Initialized audio gain : %f\n","SPEAKER0", m_audioGain);
                    }
                }
//HDMI init
                handle = 0;
                if (dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                    try {
                        _AudioGain = device::HostPersistence::getInstance().getProperty("HDMI0.audio.Gain");
                    }
                    catch(...) {
                            try {
                                printf("HDMI0.audio.Gain not found in persistence store. Try system default\n");
                                _AudioGain = device::HostPersistence::getInstance().getDefaultProperty("HDMI0.audio.Gain");
                            }
                            catch(...) {
                                _AudioGain = "0";
                            }
                    }
                    m_audioGain = atof(_AudioGain.c_str());
                    if (dsSetAudioGainFunc(handle, m_audioGain) == dsERR_NONE) {
                        printf("Port %s: Initialized audio gain : %f\n","HDMI0", m_audioGain);
                    }
                }

            }
            else {
                printf("dsSetAudioGain_t(int, float ) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }


    typedef dsError_t (*dsSetAudioLevel_t)(int handle, float level);
    static dsSetAudioLevel_t dsSetAudioLevelFunc = 0;
    if (dsSetAudioLevelFunc == 0) {
        dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            dsSetAudioLevelFunc = (dsSetAudioLevel_t) dlsym(dllib, "dsSetAudioLevel");
            if (dsSetAudioGainFunc) {
                printf("dsSetAudioLevel_t(int, float ) is defined and loaded\r\n");
                std::string _AudioLevel("0");
                float m_audioLevel = 0;
//SPEAKER init
                handle = 0;
                if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                    try {
                        _AudioLevel = device::HostPersistence::getInstance().getProperty("SPEAKER0.audio.Level");
                    }
                    catch(...) {
                            try {
                                printf("SPEAKER0.audio.Level not found in persistence store. Try system default\n");
                                _AudioLevel = device::HostPersistence::getInstance().getDefaultProperty("SPEAKER0.audio.Level");
                            }
                            catch(...) {
                                _AudioLevel = "40";
                            }
                    }
                    m_audioLevel = atof(_AudioLevel.c_str());
                    if (dsSetAudioLevelFunc(handle, m_audioLevel) == dsERR_NONE) {
                        m_volumeLevel = m_audioLevel;
                        printf("Port %s: Initialized audio level : %f\n","SPEAKER0", m_audioLevel);
                    }
                }
//HEADPHONE init
                handle = 0;
                if(dsGetAudioPort(dsAUDIOPORT_TYPE_HEADPHONE,0,&handle) == dsERR_NONE) {
                    try {
                        _AudioLevel = device::HostPersistence::getInstance().getProperty("HEADPHONE0.audio.Level");
                    }
                    catch(...) {
                            try {
                                printf("HEADPHONE0.audio.Level not found in persistence store. Try system default\n");
                                _AudioLevel = device::HostPersistence::getInstance().getDefaultProperty("HEADPHONE0.audio.Level");
                            }
                            catch(...) {
                                _AudioLevel = "40";
                            }
                    }
                    m_audioLevel = atof(_AudioLevel.c_str());
                    if (dsSetAudioLevelFunc(handle, m_audioLevel) == dsERR_NONE) {
                        printf("Port %s: Initialized audio level : %f\n","HEADPHONE0", m_audioLevel);
                    }
                }
//HDMI init
                handle = 0;
                if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                    try {
                        _AudioLevel = device::HostPersistence::getInstance().getProperty("HDMI0.audio.Level");
                    }
                    catch(...) {
                            try {
                                printf("HDMI0.audio.Level not found in persistence store. Try system default\n");
                                _AudioLevel = device::HostPersistence::getInstance().getDefaultProperty("HDMI0.audio.Level");
                            }
                            catch(...) {
                                _AudioLevel = "40";
                            }
                    }
                    m_audioLevel = atof(_AudioLevel.c_str());
                    if (dsSetAudioLevelFunc(handle, m_audioLevel) == dsERR_NONE) {
                        printf("Port %s: Initialized audio level : %f\n","HDMI0", m_audioLevel);
                        m_volumeLevel = m_audioLevel;
                    }
                }
            }
            else {
                printf("dsSetAudioLevel_t(int, float ) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }


    typedef dsError_t (*dsSetAudioDelay_t)(int handle, uint32_t audioDelayMs);
    static dsSetAudioDelay_t dsSetAudioDelayFunc = 0;
    if (dsSetAudioDelayFunc == 0) {
        dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            dsSetAudioDelayFunc = (dsSetAudioDelay_t) dlsym(dllib, "dsSetAudioDelay");
            if (dsSetAudioDelayFunc) {
                printf("dsSetAudioDelay_t(int, uint32_t) is defined and loaded\r\n");
                std::string _AudioDelay("0");
                int m_audioDelay = 0;
//SPEAKER init
                handle = 0;
                if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                    try {
                        _AudioDelay = device::HostPersistence::getInstance().getProperty("SPEAKER0.audio.Delay");
                    }
                    catch(...) {
                            try {
                                printf("SPEAKER0.audio.Delay not found in persistence store. Try system default\n");
                                _AudioDelay = device::HostPersistence::getInstance().getDefaultProperty("SPEAKER0.audio.Delay");
                            }
                            catch(...) {
                                _AudioDelay = "0";
                            }
                    }
                    m_audioDelay = atoi(_AudioDelay.c_str());
                    if (dsSetAudioDelayFunc(handle, m_audioDelay) == dsERR_NONE) {
                        printf("Port %s: Initialized audio delay : %d\n","SPEAKER0", m_audioDelay);
                    }
                }
//HDMI init
                handle = 0;
                if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                    try {
                        _AudioDelay = device::HostPersistence::getInstance().getProperty("HDMI0.audio.Delay");
                    }
                    catch(...) {
                            try {
                                printf("HDMI0.audio.Delay not found in persistence store. Try system default\n");
                                _AudioDelay = device::HostPersistence::getInstance().getDefaultProperty("HDMI0.audio.Delay");
                            }
                            catch(...) {
                                _AudioDelay = "0";
                            }
                    }
                    m_audioDelay = atoi(_AudioDelay.c_str());
                    if (dsSetAudioDelayFunc(handle, m_audioDelay) == dsERR_NONE) {
                        printf("Port %s: Initialized audio delay : %d\n","HDMI0", m_audioDelay);
                    }
                }
//HDMI ARC init
                handle = 0;
                if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI_ARC,0,&handle) == dsERR_NONE) {
                    try {
                        _AudioDelay = device::HostPersistence::getInstance().getProperty("HDMI_ARC0.audio.Delay");
                    }
                    catch(...) {
                            try {
                                printf("HDMI_ARC0.audio.Delay not found in persistence store. Try system default\n");
                                _AudioDelay = device::HostPersistence::getInstance().getDefaultProperty("HDMI_ARC0.audio.Delay");
                            }
                            catch(...) {
                                _AudioDelay = "0";
                            }
                    }
                    m_audioDelay = atoi(_AudioDelay.c_str());
                    if (dsSetAudioDelayFunc(handle, m_audioDelay) == dsERR_NONE) {
                        printf("Port %s: Initialized audio delay : %d\n","HDMI_ARC0", m_audioDelay);
                    }
                }


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


    typedef dsError_t (*dsSetAudioDelayOffset_t)(int handle, uint32_t audioDelayOffsetMs);
    static dsSetAudioDelayOffset_t dsSetAudioDelayOffsetFunc = 0;
    if (dsSetAudioDelayOffsetFunc == 0) {
        dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            dsSetAudioDelayOffsetFunc = (dsSetAudioDelayOffset_t) dlsym(dllib, "dsSetAudioDelayOffset");
            if (dsSetAudioDelayOffsetFunc) {
                printf("dsSetAudioDelayOffset_t(int, uint32_t) is defined and loaded\r\n");
                std::string _AudioDelayOffset("0");
                int m_audioDelayOffset = 0;
//SPEAKER init
                handle = 0;
                if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                    try {
                        _AudioDelayOffset = device::HostPersistence::getInstance().getProperty("SPEAKER0.audio.DelayOffset");
                    }
                    catch(...) {
                            try {
                                printf("SPEAKER0.audio.DelayOffset not found in persistence store. Try system default\n");
                                _AudioDelayOffset = device::HostPersistence::getInstance().getDefaultProperty("SPEAKER0.audio.DelayOffset");
                            }
                            catch(...) {
                                _AudioDelayOffset = "0";
                            }
                    }
                    m_audioDelayOffset = atoi(_AudioDelayOffset.c_str());
                    if (dsSetAudioDelayOffsetFunc(handle, m_audioDelayOffset) == dsERR_NONE) {
                        printf("Port %s: Initialized audio delay offset : %d\n","SPEAKER0", m_audioDelayOffset);
                    }
                }
//HDMI init
                handle = 0;
                if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                    try {
                        _AudioDelayOffset = device::HostPersistence::getInstance().getProperty("HDMI0.audio.DelayOffset");
                    }
                    catch(...) {
                            try {
                                printf("HDMI0.audio.DelayOffset not found in persistence store. Try system default\n");
                                _AudioDelayOffset = device::HostPersistence::getInstance().getDefaultProperty("HDMI0.audio.DelayOffset");
                            }
                            catch(...) {
                                _AudioDelayOffset = "0";
                            }
                    }
                    m_audioDelayOffset = atoi(_AudioDelayOffset.c_str());
                    if (dsSetAudioDelayOffsetFunc(handle, m_audioDelayOffset) == dsERR_NONE) {
                        printf("Port %s: Initialized audio delay offset : %d\n","HDMI0", m_audioDelayOffset);
                    }
                }
//HDMI ARC init
                handle = 0;
                if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI_ARC,0,&handle) == dsERR_NONE) {
                    try {
                        _AudioDelayOffset = device::HostPersistence::getInstance().getProperty("HDMI_ARC0.audio.DelayOffset");
                    }
                    catch(...) {
                            try {
                                printf("HDMI_ARC0.audio.DelayOffset not found in persistence store. Try system default\n");
                                _AudioDelayOffset = device::HostPersistence::getInstance().getDefaultProperty("HDMI_ARC0.audio.DelayOffset");
                            }
                            catch(...) {
                                _AudioDelayOffset = "0";
                            }
                    }
                    m_audioDelayOffset = atoi(_AudioDelayOffset.c_str());
                    if (dsSetAudioDelayFunc(handle, m_audioDelayOffset) == dsERR_NONE) {
                        printf("Port %s: Initialized audio delay : %d\n","HDMI_ARC0", m_audioDelayOffset);
                    }
                }
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


    typedef dsError_t (*dsSetPrimaryLanguage_t)(int handle, const char* pLang);;
    static dsSetPrimaryLanguage_t dsSetPrimaryLanguageFunc = 0;
    if (dsSetPrimaryLanguageFunc == 0) {
        dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            dsSetPrimaryLanguageFunc = (dsSetPrimaryLanguage_t) dlsym(dllib, "dsSetPrimaryLanguage");
            if (dsSetPrimaryLanguageFunc) {
                printf("dsSetPrimaryLanguage_t(int, char* ) is defined and loaded\r\n");
                std::string _PrimaryLanguage("eng");
                handle = 0;
                try {
                    _PrimaryLanguage = device::HostPersistence::getInstance().getProperty("audio.PrimaryLanguage");
                }
                catch(...) {
                        try {
                            printf("audio.PrimaryLanguage not found in persistence store. Try system default\n");
                            _PrimaryLanguage = device::HostPersistence::getInstance().getDefaultProperty("audio.PrimaryLanguage");
                        }
                        catch(...) {
                            _PrimaryLanguage = "eng";
                        }
                }
                if (dsSetPrimaryLanguageFunc(handle, _PrimaryLanguage.c_str()) == dsERR_NONE) {
                    printf("Initialized Primary Language : %s\n", _PrimaryLanguage.c_str());
                }
            }
            else {
                printf("dsSetPrimaryLanguage_t(int, char* ) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }


    typedef dsError_t (*dsSetSecondaryLanguage_t)(int handle, const char* sLang);;
    static dsSetSecondaryLanguage_t dsSetSecondaryLanguageFunc = 0;
    if (dsSetSecondaryLanguageFunc == 0) {
        dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            dsSetSecondaryLanguageFunc = (dsSetSecondaryLanguage_t) dlsym(dllib, "dsSetSecondaryLanguage");
            if (dsSetSecondaryLanguageFunc) {
                printf("dsSetSecondaryLanguage_t(int, char* ) is defined and loaded\r\n");
                std::string _SecondaryLanguage("eng");
                handle = 0;
                try {
                    _SecondaryLanguage = device::HostPersistence::getInstance().getProperty("audio.SecondaryLanguage");
                }
                catch(...) {
                        try {
                            printf("audio.SecondaryLanguage not found in persistence store. Try system default\n");
                            _SecondaryLanguage = device::HostPersistence::getInstance().getDefaultProperty("audio.SecondaryLanguage");
                        }
                        catch(...) {
                            _SecondaryLanguage = "eng";
                        }
                }
                if (dsSetSecondaryLanguageFunc(handle, _SecondaryLanguage.c_str()) == dsERR_NONE) {
                    printf("Initialized Secondary Language : %s\n", _SecondaryLanguage.c_str());
                }
            }
            else {
                printf("dsSetSecondaryLanguage_t(int, char* ) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }


    typedef dsError_t (*dsSetFaderControl_t)(int handle, int mixerbalance);
    static dsSetFaderControl_t dsSetFaderControlFunc = 0;
    if (dsSetFaderControlFunc == 0) {
        dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            dsSetFaderControlFunc = (dsSetFaderControl_t) dlsym(dllib, "dsSetFaderControl");
            if (dsSetFaderControlFunc) {
                printf("dsSetFaderControl_t(int, int) is defined and loaded\r\n");
                std::string _FaderControl("0");
                int m_faderControl = 0;

                handle = 0;
                try {
                    _FaderControl = device::HostPersistence::getInstance().getProperty("audio.FaderControl");
                }
                catch(...) {
                        try {
                            printf("audio.FaderControl not found in persistence store. Try system default\n");
                            _FaderControl = device::HostPersistence::getInstance().getDefaultProperty("audio.FaderControl");
                        }
                        catch(...) {
                            _FaderControl = "0";
                        }
                }
                m_faderControl = atoi(_FaderControl.c_str());
                if (dsSetFaderControlFunc(handle, m_faderControl) == dsERR_NONE) {
                    printf("Initialized Fader Control, mixing : %d\n", m_faderControl);
                }

            }
            else {
                printf("dsSetFaderControl_t(int, int) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }


    typedef dsError_t (*dsSetAssociatedAudioMixing_t)(int handle, bool mixing);
    static dsSetAssociatedAudioMixing_t dsSetAssociatedAudioMixingFunc = 0;
    if (dsSetAssociatedAudioMixingFunc == 0) {
        dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            dsSetAssociatedAudioMixingFunc = (dsSetAssociatedAudioMixing_t) dlsym(dllib, "dsSetAssociatedAudioMixing");
            if (dsSetAssociatedAudioMixingFunc) {
                printf("dsSetAssociatedAudioMixing_t (int handle, bool mixing ) is defined and loaded\r\n");
                std::string _AssociatedAudioMixing("Disabled");
                bool m_AssociatedAudioMixing = false;
                try {
                    _AssociatedAudioMixing = device::HostPersistence::getInstance().getProperty("audio.AssociatedAudioMixing");
                }
                catch(...) {
                    try {
                        printf("audio.AssociatedAudioMixing not found in persistence store. Try system default\n");
                        _AssociatedAudioMixing = device::HostPersistence::getInstance().getDefaultProperty("audio.AssociatedAudioMixing");
                    }
                    catch(...) {
                        _AssociatedAudioMixing = "Disabled";
                    }
                }
                if (_AssociatedAudioMixing == "Enabled") {
                    m_AssociatedAudioMixing = true;
                }
                else {
                    m_AssociatedAudioMixing = false;
                }
                handle = 0;
                if (dsSetAssociatedAudioMixingFunc(handle, m_AssociatedAudioMixing) == dsERR_NONE) {
                    printf("Initialized AssociatedAudioMixingFunc : %d\n", m_AssociatedAudioMixing);
                }
            }
            else {
                printf("dsSetAssociatedAudioMixing_t (int handle, bool enable) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }


    typedef dsError_t (*dsSetAudioAtmosOutputMode_t)(int handle, bool enable);
    static dsSetAudioAtmosOutputMode_t dsSetAudioAtmosOutputModeFunc = 0;
    if (dsSetAudioAtmosOutputModeFunc == 0) {
        dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            dsSetAudioAtmosOutputModeFunc = (dsSetAudioAtmosOutputMode_t) dlsym(dllib, "dsSetAudioAtmosOutputMode");
            if (dsSetAudioAtmosOutputModeFunc) {
                printf("dsSetAudioAtmosOutputMode_t (int handle, bool enable ) is defined and loaded\r\n");
                std::string _AudioAtmosOutputMode("FALSE");
                bool m_audioAtmosOutputMode = false;
                try {
                    _AudioAtmosOutputMode = device::HostPersistence::getInstance().getProperty("audio.AtmosOutputMode");
                }
                catch(...) {
                    try {
                        printf("audio.AtmosOutputMode not found in persistence store. Try system default\n");
                        _AudioAtmosOutputMode = device::HostPersistence::getInstance().getDefaultProperty("audio.AtmosOutputMode");
                    }
                    catch(...) {
                        _AudioAtmosOutputMode = "FALSE";
                    }
                }
                if (_AudioAtmosOutputMode == "TRUE") { 
                    m_audioAtmosOutputMode = true;
                }
                else {
                    m_audioAtmosOutputMode = false;
                }
//SPEAKER init
                handle = 0;
                if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                    if (dsSetAudioAtmosOutputModeFunc(handle, m_audioAtmosOutputMode) == dsERR_NONE) {
                        printf("Port %s: Initialized AudioAtmosOutputModeFunc : %d\n","SPEAKER0", m_audioAtmosOutputMode);
                    }
                }
//HDMI init
                handle = 0;
                if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                    if (dsSetAudioAtmosOutputModeFunc(handle, m_audioAtmosOutputMode) == dsERR_NONE) {
                        printf("Port %s: Initialized AudioAtmosOutputModeFunc : %d\n","HDMI0", m_audioAtmosOutputMode);
                    }
                }
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

    std::string _AProfileSupport("FALSE");
    std::string _AProfile("Off");
    try {
        _AProfileSupport = device::HostPersistence::getInstance().getDefaultProperty("audio.MS12Profile.supported");
    }
    catch(...) {
        _AProfileSupport = "FALSE";
        printf("audio.MS12Profile.supported setting not found in hostDataDeafult \r\n");
    }
    printf(" audio.MS12Profile.supported = %s ..... \r\n",_AProfileSupport.c_str());


    if(_AProfileSupport == "TRUE") {
        typedef dsError_t (*dsSetMS12AudioProfile_t)(int handle, const char* profile);
        static dsSetMS12AudioProfile_t dsSetMS12AudioProfileFunc = 0;
        if (dsSetMS12AudioProfileFunc == 0) {
            dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
            if (dllib) {
                dsSetMS12AudioProfileFunc = (dsSetMS12AudioProfile_t) dlsym(dllib, "dsSetMS12AudioProfile");
                if (dsSetMS12AudioProfileFunc) {
                    printf("dsSetMS12AudioProfile_t(int, const char*) is defined and loaded\r\n");
                    handle = 0;
                    dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle);

                    try {
                        _AProfile = device::HostPersistence::getInstance().getProperty("audio.MS12Profile");
                    }
                    catch(...) {
                            try {
                                printf("audio.MS12Profile not found in persistence store. Try system default\n");
                                _AProfile = device::HostPersistence::getInstance().getDefaultProperty("audio.MS12Profile");
                            }
                            catch(...) {
                                _AProfile = "Off";
                            }
                    }
        //SPEAKER init
                    handle = 0;
                    if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                        if (dsSetMS12AudioProfileFunc(handle, _AProfile.c_str()) == dsERR_NONE) {
                            printf("Port %s: Initialized MS12 Audio Profile : %s\n","SPEAKER0", _AProfile.c_str());
                            device::HostPersistence::getInstance().persistHostProperty("audio.MS12Profile",_AProfile.c_str());
                        }
                       else {
                            printf("Port %s: Initialization failed !!!  MS12 Audio Profile : %s\n","SPEAKER0", _AProfile.c_str());
                       }
                    }
    #if 0 // No Audio Profile support for STB devices
        //HDMI init
                    handle = 0;
                    if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                        if (dsSetMS12AudioProfileFunc(handle, _AProfile.c_str()) == dsERR_NONE) {
                            printf("Port %s: Initialized MS12 Audio Profile  : %d\n","HDMI0", _AProfile.c_str());
                        }
                    }
    #endif
                }
                else {
                    printf("dsSetMS12AudioProfile_t(int, const char*) is not defined\r\n");
                }
                dlclose(dllib);
            }
            else {
                printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
            }
        }
    }
//All MS12 Settings can be initialised through MS12 audio profiles
//User setting persistence override available for any individual setting on top of profiles
//All MS12 settings can be turned off (DAP Off Mode) by configuring Audio Profile to Off
    if((_AProfileSupport == "TRUE") && (_AProfile != "Off")) {
        std::string _profileOverride = "FALSE";
        try {
            _profileOverride = device::HostPersistence::getInstance().getDefaultProperty("audio.Compression.ms12ProfileOverride");
        }
        catch(...) {
            _profileOverride = "FALSE";
            printf(" audio.Compression.ms12ProfileOverride settings not found in system default ..... \r\n");
        }
        printf(" audio.Compression.ms12ProfileOverride = %s ..... \r\n",_profileOverride.c_str());

        if(_profileOverride == "TRUE") {
            typedef dsError_t (*dsSetAudioCompression_t)(int handle, int compressionLevel);
            static dsSetAudioCompression_t dsSetAudioCompressionFunc = 0;
            if (dsSetAudioCompressionFunc == 0) {
                void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
                if (dllib) {
                    dsSetAudioCompressionFunc = (dsSetAudioCompression_t) dlsym(dllib, "dsSetAudioCompression");
                    if (dsSetAudioCompressionFunc) {
                        printf("dsSetAudioCompression_t(int, int ) is defined and loaded\r\n");
                        std::string _AudioCompression("0");
                        int m_audioCompression = 0;
                        try {
                            _AudioCompression = device::HostPersistence::getInstance().getProperty("audio.Compression");
                            m_audioCompression = atoi(_AudioCompression.c_str());
            //SPEAKER init
                            handle = 0;
                            if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                                if (dsSetAudioCompressionFunc(handle, m_audioCompression) == dsERR_NONE) {
                                    printf("Port %s: Initialized audio compression : %d\n","SPEAKER0", m_audioCompression);
                                }
                            }
            //HDMI init
                            handle = 0;
                            if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                                if (dsSetAudioCompressionFunc(handle, m_audioCompression) == dsERR_NONE) {
                                    printf("Port %s: Initialized audio compression : %d\n","HDMI0", m_audioCompression);
                                }
                            }

                        }
                        catch(...) {
                            printf("audio.Compression not found in persistence store. System Default configured through profiles\n");
                        }
                    }
                    else {
                        printf("dsSetAudioCompression_t(int, int) is not defined\r\n");
                    }
                    dlclose(dllib);
                }
                else {
                    printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
                }
            }
        }

        _profileOverride = "FALSE";
        try {
            _profileOverride = device::HostPersistence::getInstance().getDefaultProperty("audio.DialogEnhancer.ms12ProfileOverride");
        }
        catch(...) {
            _profileOverride = "FALSE";
            printf(" audio.DialogEnhancer.ms12ProfileOverride settings not found in system default ..... \r\n");
        }
        printf(" audio.DialogEnhancer.ms12ProfileOverride = %s ..... \r\n",_profileOverride.c_str());

        if(_profileOverride == "TRUE") {
            typedef dsError_t (*dsSetDialogEnhancement_t)(int handle, int enhancerLevel);
            static dsSetDialogEnhancement_t dsSetDialogEnhancementFunc = 0;
            if (dsSetDialogEnhancementFunc == 0) {
                void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
                if (dllib) {
                    dsSetDialogEnhancementFunc = (dsSetDialogEnhancement_t) dlsym(dllib, "dsSetDialogEnhancement");
                    if (dsSetDialogEnhancementFunc) {
                        printf("dsSetDialogEnhancement_t(int, int) is defined and loaded\r\n");
                        std::string _EnhancerLevel("0");
                        int m_enhancerLevel = 0;
                        std::string _Property = _dsGetCurrentProfileProperty("EnhancerLevel");
                        try {
                            _EnhancerLevel = device::HostPersistence::getInstance().getProperty(_Property);
                            m_enhancerLevel = atoi(_EnhancerLevel.c_str());
            //SPEAKER init
                            handle = 0;
                            if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                                if (dsSetDialogEnhancementFunc(handle, m_enhancerLevel) == dsERR_NONE) {
                                    printf("Port %s: Initialized dialog enhancement level : %d\n","SPEAKER0", m_enhancerLevel);
                                }
                            }
            //HDMI init
                            handle = 0;
                            if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                                if (dsSetDialogEnhancementFunc(handle, m_enhancerLevel) == dsERR_NONE) {
                                    printf("Port %s: Initialized dialog enhancement level : %d\n","HDMI0", m_enhancerLevel);
                                }
                            }
                        }
                        catch(...) {
                            printf("audio.EnhancerLevel not found in persistence store. System Default configured through profiles\n");
                        }
                    }
                    else {
                        printf("dsSetDialogEnhancement_t(int, int ) is not defined\r\n");
                    }
                    dlclose(dllib);
                }
                else {
                    printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
                }
            }
        }

        _profileOverride = "FALSE";
        try {
            _profileOverride = device::HostPersistence::getInstance().getDefaultProperty("audio.DolbyVolumeMode.ms12ProfileOverride");
        }
        catch(...) {
            _profileOverride = "FALSE";
            printf(" audio.DolbyVolumeMode.ms12ProfileOverride settings not found in system default ..... \r\n");
        }
        printf(" audio.DolbyVolumeMode.ms12ProfileOverride = %s ..... \r\n",_profileOverride.c_str());

        if(_profileOverride == "TRUE") {
            typedef dsError_t (*dsSetDolbyVolumeMode_t)(int handle, bool enable);
            static dsSetDolbyVolumeMode_t dsSetDolbyVolumeModeFunc = 0;
            if (dsSetDolbyVolumeModeFunc == 0) {
                dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
                if (dllib) {
                    dsSetDolbyVolumeModeFunc = (dsSetDolbyVolumeMode_t) dlsym(dllib, "dsSetDolbyVolumeMode");
                    if (dsSetDolbyVolumeModeFunc) {
                        printf("dsSetDolbyVolumeMode_t(int, bool) is defined and loaded\r\n");
                        std::string _DolbyVolumeMode("FALSE");
                        bool m_dolbyVolumeMode = false;
                        try {
                            _DolbyVolumeMode = device::HostPersistence::getInstance().getProperty("audio.DolbyVolumeMode");
                            if (_DolbyVolumeMode == "TRUE") {
                                m_dolbyVolumeMode = true;
                            }
                            else {
                                m_dolbyVolumeMode = false;
                            }
            //SPEAKER init
                            handle = 0;
                            if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                                if (dsSetDolbyVolumeModeFunc(handle, m_dolbyVolumeMode) == dsERR_NONE) {
                                    printf("Port %s: Initialized Dolby Volume Mode : %d\n","SPEAKER0", m_dolbyVolumeMode);
                                }
                            }
            //HDMI init
                            handle = 0;
                            if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                                if (dsSetDolbyVolumeModeFunc(handle, m_dolbyVolumeMode) == dsERR_NONE) {
                                    printf("Port %s: Initialized Dolby Volume Mode : %d\n","HDMI0", m_dolbyVolumeMode);
                                }
                            }
                        }
                        catch(...) {
                            printf("audio.DolbyVolumeMode not found in persistence store. System Default configured through profiles\n");
                        }
                    }
                    else {
                        printf("dsSetDolbyVolumeMode_t(int, bool) is not defined\r\n");
                    }
                    dlclose(dllib);
                }
                else {
                    printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
                }
            }
        }

        _profileOverride = "FALSE";
        try {
            _profileOverride = device::HostPersistence::getInstance().getDefaultProperty("audio.IntelligentEQ.ms12ProfileOverride");
        }
        catch(...) {
            _profileOverride = "FALSE";
            printf(" audio.IntelligentEQ.ms12ProfileOverride settings not found in system default ..... \r\n");
        }
        printf(" audio.IntelligentEQ.ms12ProfileOverride = %s ..... \r\n",_profileOverride.c_str());

        if(_profileOverride == "TRUE") {
            typedef dsError_t (*dsSetIntelligentEqualizerMode_t)(int handle, int mode);
            static dsSetIntelligentEqualizerMode_t dsSetIntelligentEqualizerModeFunc = 0;
            if (dsSetIntelligentEqualizerModeFunc == 0) {
                dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
                if (dllib) {
                    dsSetIntelligentEqualizerModeFunc = (dsSetIntelligentEqualizerMode_t) dlsym(dllib, "dsSetIntelligentEqualizerMode");
                    if (dsSetIntelligentEqualizerModeFunc) {
                        printf("dsSetIntelligentEqualizerMode_t(int, int) is defined and loaded\r\n");
                        std::string _IEQMode("0");
                        int m_IEQMode = 0;
                        handle = 0;
                        dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle);
                        try {
                            _IEQMode = device::HostPersistence::getInstance().getProperty("audio.IntelligentEQ");
                            m_IEQMode = atoi(_IEQMode.c_str());
            //SPEAKER init
                            handle = 0;
                            if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                                if (dsSetIntelligentEqualizerModeFunc(handle, m_IEQMode) == dsERR_NONE) {
                                    printf("Port %s: Initialized Intelligent Equalizer mode : %d\n","SPEAKER0", m_IEQMode);
                                }
                            }
            //HDMI init
                            handle = 0;
                            if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                                if (dsSetIntelligentEqualizerModeFunc(handle, m_IEQMode) == dsERR_NONE) {
                                    printf("Port %s: Initialized Intelligent Equalizer mode : %d\n","HDMI0", m_IEQMode);
                                }
                            }
                        }
                        catch(...) {
                            printf("audio.IntelligentEQ not found in persistence store. System Default configured through profiles\n");
                        }
                    }
                    else {
                        printf("dsSetIntelligentEqualizerMode_t(int, int) is not defined\r\n");
                    }
                    dlclose(dllib);
                }
                else {
                    printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
                }
            }
        }

        _profileOverride = "FALSE";
        try {
            _profileOverride = device::HostPersistence::getInstance().getDefaultProperty("audio.VolumeLeveller.ms12ProfileOverride");
        }
        catch(...) {
            _profileOverride = "FALSE";
            printf(" audio.VolumeLeveller.ms12ProfileOverride settings not found in system default ..... \r\n");
        }
        printf(" audio.VolumeLeveller.ms12ProfileOverride = %s ..... \r\n",_profileOverride.c_str());

        if(_profileOverride == "TRUE") {
            typedef dsError_t (*dsSetVolumeLeveller_t)(int handle, dsVolumeLeveller_t volLeveller);
            static dsSetVolumeLeveller_t dsSetVolumeLevellerFunc = 0;
            if (dsSetVolumeLevellerFunc == 0) {
                dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
                if (dllib) {
                    dsSetVolumeLevellerFunc = (dsSetVolumeLeveller_t) dlsym(dllib, "dsSetVolumeLeveller");
                    if (dsSetVolumeLevellerFunc) {
                        printf("dsSetVolumeLeveller_t(int, dsVolumeLeveller_t) is defined and loaded\r\n");
                        std::string _volLevellerMode("0");
                        std::string _volLevellerLevel("0");
                        dsVolumeLeveller_t m_volumeLeveller;
                        std::string _PropertyMode = _dsGetCurrentProfileProperty("VolumeLeveller.mode");
                        std::string _Propertylevel = _dsGetCurrentProfileProperty("VolumeLeveller.level"); 
                        try {
                            _volLevellerMode = device::HostPersistence::getInstance().getProperty(_PropertyMode);
                            _volLevellerLevel = device::HostPersistence::getInstance().getProperty(_Propertylevel);
                            m_volumeLeveller.mode = atoi(_volLevellerMode.c_str());
			    m_volumeLeveller.level = atoi(_volLevellerLevel.c_str());
                //SPEAKER init
                            handle = 0;
                            if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                                if (dsSetVolumeLevellerFunc(handle, m_volumeLeveller) == dsERR_NONE) {
                                    printf("Port %s: Initialized Volume Leveller : Mode: %d, Level: %d\n","SPEAKER0", m_volumeLeveller.mode, m_volumeLeveller.level);
                                }
                            }
                //HDMI init
                            handle = 0;
                            if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                                if (dsSetVolumeLevellerFunc(handle, m_volumeLeveller) == dsERR_NONE) {
                                    printf("Port %s: Initialized Volume Leveller : Mode: %d, Level: %d\n","HDMI0", m_volumeLeveller.mode, m_volumeLeveller.level);
                                }
                            }
                        }
                        catch(...) {
                            printf("audio.VolumeLeveller not found in persistence store. System Default configured through profiles\n");
                        }
                    }
                    else {
                        printf("dsSetVolumeLeveller_t(int, dsVolumeLeveller_t) is not defined\r\n");
                    }
                    dlclose(dllib);
                }
                else {
                    printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
                }
            }
        }

        _profileOverride = "FALSE";
        try {
            _profileOverride = device::HostPersistence::getInstance().getDefaultProperty("audio.BassBoost.ms12ProfileOverride");
        }
        catch(...) {
            _profileOverride = "FALSE";
            printf(" audio.BassBoost.ms12ProfileOverride settings not found in system default ..... \r\n");
        }
        printf(" audio.BassBoost.ms12ProfileOverride = %s ..... \r\n",_profileOverride.c_str());

        if(_profileOverride == "TRUE") {
            typedef dsError_t (*dsSetBassEnhancer_t)(int handle, int boost);
            static dsSetBassEnhancer_t dsSetBassEnhancerFunc = 0;
            if (dsSetBassEnhancerFunc == 0) {
                dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
                if (dllib) {
                    dsSetBassEnhancerFunc = (dsSetBassEnhancer_t) dlsym(dllib, "dsSetBassEnhancer");
                    if (dsSetBassEnhancerFunc) {
                        printf("dsSetBassEnhancer_t(int, int) is defined and loaded\r\n");
                        std::string _BassBoost("0");
                        int m_bassBoost = 0;
                        try {
                            _BassBoost = device::HostPersistence::getInstance().getProperty("audio.BassBoost");
                            m_bassBoost = atoi(_BassBoost.c_str());
            //SPEAKER init
                            handle = 0;
                            if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                                if (dsSetBassEnhancerFunc(handle, m_bassBoost) == dsERR_NONE) {
                                    printf("Port %s: Initialized Bass Boost : %d\n","SPEAKER0", m_bassBoost);
                                }
                            }
            //HDMI init
                            handle = 0;
                            if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                                if (dsSetBassEnhancerFunc(handle, m_bassBoost) == dsERR_NONE) {
                                    printf("Port %s: Initialized Bass Boost : %d\n","HDMI0", m_bassBoost);
                                }
                            }
                        }
                        catch(...) {
                            printf("audio.BassBoost not found in persistence store. System Default configured through profiles\n");
                        }
                    }
                    else {
                        printf("dsSetBassEnhancer_t(int, int) is not defined\r\n");
                    }
                    dlclose(dllib);
                }
                else {
                    printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
                }
            }
        }

        _profileOverride = "FALSE";
        try {
            _profileOverride = device::HostPersistence::getInstance().getDefaultProperty("audio.SurroundDecoder.ms12ProfileOverride");
        }
        catch(...) {
            _profileOverride = "FALSE";
            printf(" audio.SurroundDecoder.ms12ProfileOverride settings not found in system default ..... \r\n");
        }
        printf(" audio.SurroundDecoder.ms12ProfileOverride = %s ..... \r\n",_profileOverride.c_str());

        if(_profileOverride == "TRUE") {
            typedef dsError_t (*dsEnableSurroundDecoder_t)(int handle, bool enabled);
            static dsEnableSurroundDecoder_t dsEnableSurroundDecoderFunc = 0;
            if (dsEnableSurroundDecoderFunc == 0) {
                dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
                if (dllib) {
                    dsEnableSurroundDecoderFunc = (dsEnableSurroundDecoder_t) dlsym(dllib, "dsEnableSurroundDecoder");
                    if (dsEnableSurroundDecoderFunc) {
                        printf("dsEnableSurroundDecoder_t(int, bool) is defined and loaded\r\n");
                        std::string _SurroundDecoder("FALSE");
                        bool m_surroundDecoder = false;
                        try {
                            _SurroundDecoder = device::HostPersistence::getInstance().getProperty("audio.SurroundDecoderEnabled");
                            if (_SurroundDecoder == "TRUE") {
                                m_surroundDecoder = true;
                            }
                            else {
                                m_surroundDecoder = false;
                            }
            //SPEAKER init
                            handle = 0;
                            if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                                if (dsEnableSurroundDecoderFunc(handle, m_surroundDecoder) == dsERR_NONE) {
                                    printf("Port %s: Initialized Surroudn Decoder : %d\n","SPEAKER0", m_surroundDecoder);
                                }
                            }
            //HDMI init
                            handle = 0;
                            if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                                if (dsEnableSurroundDecoderFunc(handle, m_surroundDecoder) == dsERR_NONE) {
                                    printf("Port %s: Initialized Surroudn Decoder : %d\n","HDMI0", m_surroundDecoder);
                                }
                            }
                        }
                        catch(...) {
                            printf("audio.SurroundDecoderEnabled not found in persistence store. System Default configured through profiles\n");
                        }
                    }
                    else {
                        printf("dsEnableSurroundDecoder_t(int, bool) is not defined\r\n");
                    }
                    dlclose(dllib);
                }
                else {
                    printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
                }
            }
	    }

        _profileOverride = "FALSE";
        try {
            _profileOverride = device::HostPersistence::getInstance().getDefaultProperty("audio.DRCMode.ms12ProfileOverride");
        }
        catch(...) {
            _profileOverride = "FALSE";
            printf(" audio.DRCMode.ms12ProfileOverride settings not found in system default ..... \r\n");
        }
        printf(" audio.DRCMode.ms12ProfileOverride = %s ..... \r\n",_profileOverride.c_str());

        if(_profileOverride == "TRUE") {
            typedef dsError_t (*dsSetDRCMode_t)(int handle, int mode);
            static dsSetDRCMode_t dsSetDRCModeFunc = 0;
            if (dsSetDRCModeFunc == 0) {
                dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
                if (dllib) {
                    dsSetDRCModeFunc = (dsSetDRCMode_t) dlsym(dllib, "dsSetDRCMode");
                    if (dsSetDRCModeFunc) {
                        printf("dsSetDRCMode_t(int, int) is defined and loaded\r\n");
                        std::string _DRCMode("Line");
                        int m_DRCMode = 0;
                        try {
                            _DRCMode = device::HostPersistence::getInstance().getProperty("audio.DRCMode");
                            if (_DRCMode == "RF") {
                                m_DRCMode = 1;
                            }
                            else {
                                m_DRCMode = 0;
                            }
                //SPEAKER init
                            handle = 0;
                            if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                                if (dsSetDRCModeFunc(handle, m_DRCMode) == dsERR_NONE) {
                                    printf("Port %s: Initialized DRCMode : %d\n","SPEAKER0", m_DRCMode);
                                }
                            }
                //HDMI init
                            handle = 0;
                            if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                                if (dsSetDRCModeFunc(handle, m_DRCMode) == dsERR_NONE) {
                                    printf("Port %s: Initialized DRCMode : %d\n","HDMI0", m_DRCMode);
                                }
                            }
                        }
                        catch(...) {
                            printf("audio.DRCMode not found in persistence store. System Default configured through profiles\n");
                        }
                    }
                    else {
                        printf("dsSetDRCMode_t(int, int) is not defined\r\n");
                    }
                    dlclose(dllib);
                }
                else {
                    printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
                }
            }
        }

        _profileOverride = "FALSE";
        try {
            _profileOverride = device::HostPersistence::getInstance().getDefaultProperty("audio.SurroundVirtualizer.ms12ProfileOverride");
        }
        catch(...) {
            _profileOverride = "FALSE";
            printf(" audio.SurroundVirtualizer.ms12ProfileOverride settings not found in system default ..... \r\n");
        }
        printf(" audio.SurroundVirtualizer.ms12ProfileOverride = %s ..... \r\n",_profileOverride.c_str());

        if(_profileOverride == "TRUE") {
            typedef dsError_t (*dsSetSurroundVirtualizer_t)(int handle, dsSurroundVirtualizer_t virtualizer);
            static dsSetSurroundVirtualizer_t dsSetSurroundVirtualizerFunc = 0;
            if (dsSetSurroundVirtualizerFunc == 0) {
                dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
                if (dllib) {
                    dsSetSurroundVirtualizerFunc = (dsSetSurroundVirtualizer_t) dlsym(dllib, "dsSetSurroundVirtualizer");
                    if (dsSetSurroundVirtualizerFunc) {
                        printf("dsSetSurroundVirtualizer_t(int, dsSurroundVirtualizer_t virtualizer) is defined and loaded\r\n");
                        std::string _SVMode("0");
                        std::string _SVBoost("0");
                        dsSurroundVirtualizer_t m_virtualizer;
                        std::string _PropertyMode = _dsGetCurrentProfileProperty("SurroundVirtualizer.mode");
                        std::string _PropertyBoost = _dsGetCurrentProfileProperty("SurroundVirtualizer.boost");
                        try {
                            _SVMode = device::HostPersistence::getInstance().getProperty(_PropertyMode);
                            _SVBoost = device::HostPersistence::getInstance().getProperty(_PropertyBoost);
                            m_virtualizer.mode = atoi(_SVMode.c_str());
			    m_virtualizer.boost = atoi(_SVBoost.c_str());
                //SPEAKER init
                            handle = 0;
                            if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                                if (dsSetSurroundVirtualizerFunc(handle, m_virtualizer) == dsERR_NONE) {
                                    printf("Port %s: Initialized Surround Virtualizer : Mode: %d, Boost : %d\n","SPEAKER0", m_virtualizer.mode, m_virtualizer.boost);
                                }
                            }
                //HDMI init
                            handle = 0;
                            if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                                if (dsSetSurroundVirtualizerFunc(handle, m_virtualizer) == dsERR_NONE){
                                    printf("Port %s: Initialized Surround Virtualizer : Mode: %d, Boost : %d\\n","HDMI0", m_virtualizer.mode, m_virtualizer.boost);
				}
                            }
                        }
                        catch(...) {
                            printf("audio.SurroundVirtualizer not found in persistence store. System Default configured through profiles \n");
                        }
                    }
                    else {
                        printf("dsSetSurroundVirtualizer_t(int, dsSurroundVirtualizer_t) is not defined\r\n");
                    }
                    dlclose(dllib);
                }
                else {
                    printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
                }
            }
        }

        _profileOverride = "FALSE";
        try {
            _profileOverride = device::HostPersistence::getInstance().getDefaultProperty("audio.MISteering.ms12ProfileOverride");
        }
        catch(...) {
            _profileOverride = "FALSE";
            printf(" audio.MISteering.ms12ProfileOverride settings not found in system default ..... \r\n");
        }
        printf(" audio.MISteering.ms12ProfileOverride = %s ..... \r\n",_profileOverride.c_str());

        if(_profileOverride == "TRUE") {
            typedef dsError_t (*dsSetMISteering_t)(int handle, bool enabled);
            static dsSetMISteering_t dsSetMISteeringFunc = 0;
            if (dsSetMISteeringFunc == 0) {
                dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
                if (dllib) {
                    dsSetMISteeringFunc = (dsSetMISteering_t) dlsym(dllib, "dsSetMISteering");
                    if (dsSetMISteeringFunc) {
                        printf("dsSetMISteering_t(int, bool) is defined and loaded\r\n");
                        std::string _MISteering("Disabled");
                        bool m_MISteering = false;
                        try {
                            _MISteering = device::HostPersistence::getInstance().getProperty("audio.MISteering");
                            if (_MISteering == "Enabled") {
                                m_MISteering = true;
                            }
                            else {
                                m_MISteering = false;
                            }
                //SPEAKER init
                            handle = 0;
                            dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle);
                            if (dsSetMISteeringFunc(handle, m_MISteering) == dsERR_NONE) {
                                printf("Port %s: Initialized MI Steering : %d\n","SPEAKER0", m_MISteering);
                            }
                //HDMI init
                            handle = 0;
                            if (dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                                if (dsSetMISteeringFunc(handle, m_MISteering) == dsERR_NONE) {
                                    printf("Port %s: Initialized MI Steering : %d\n","HDMI0", m_MISteering);
                                }
                            }
                            else {
                                printf("Port %s: Initialization MI Steering : %d failed. Port not available\n","HDMI0", m_MISteering);
                            }
                        }
                        catch(...) {
                            printf("audio.MISteering not found in persistence store. System Default configured through profiles\n");
                        }
                    }
                    else {
                        printf("dsSetMISteering_t(int, bool) is not defined\r\n");
                    }
                    dlclose(dllib);
                }
                else {
                    printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
                }
            }
        }

        _profileOverride = "FALSE";
        try {
            _profileOverride = device::HostPersistence::getInstance().getDefaultProperty("audio.GraphicEQ.ms12ProfileOverride");
        }
        catch(...) {
            _profileOverride = "FALSE";
            printf(" audio.GraphicEQ.ms12ProfileOverride settings not found in system default ..... \r\n");
        }
        printf(" audio.GraphicEQ.ms12ProfileOverride = %s ..... \r\n",_profileOverride.c_str());

        if(_profileOverride == "TRUE") {
            typedef dsError_t (*dsSetGraphicEqualizerMode_t)(int handle, int mode);
            static dsSetGraphicEqualizerMode_t dsSetGraphicEqualizerModeFunc = 0;
            if (dsSetGraphicEqualizerModeFunc == 0) {
                dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
                if (dllib) {
                    dsSetGraphicEqualizerModeFunc = (dsSetGraphicEqualizerMode_t) dlsym(dllib, "dsSetGraphicEqualizerMode");
                    if (dsSetGraphicEqualizerModeFunc) {
                        printf("dsSetGraphicEqualizerMode_t(int, int) is defined and loaded\r\n");
                        std::string _GEQMode("0");
                        int m_GEQMode = 0;
                        handle = 0;
                        dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle);
                        try {
                            _GEQMode = device::HostPersistence::getInstance().getProperty("audio.GraphicEQ");
                            m_GEQMode = atoi(_GEQMode.c_str());
                //SPEAKER init
                            handle = 0;
                            if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                                if (dsSetGraphicEqualizerModeFunc(handle, m_GEQMode) == dsERR_NONE) {
                                    printf("Port %s: Initialized Graphic Equalizer mode : %d\n","SPEAKER0", m_GEQMode);
                                }
                            }
                //HDMI init
                            handle = 0;
                            if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                                if (dsSetGraphicEqualizerModeFunc(handle, m_GEQMode) == dsERR_NONE) {
                                    printf("Port %s: Initialized Graphic Equalizer mode : %d\n","HDMI0", m_GEQMode);
                                }
                            }
                        }
                        catch(...) {
                            printf("audio.GraphicEQ not found in persistence store. System Default configured through profiles\n");
                        }
                    }
                    else {
                        printf("dsSetGraphicEqualizerMode_t(int, int) is not defined\r\n");
                    }
                    dlclose(dllib);
                }
                else {
                    printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
                }
            }
	}
    }
//MS12 Audio Profile not there
//Initialize individual settings from persistence store / system default if persistence store empty
    else if(_AProfileSupport == "FALSE") {
           typedef dsError_t (*dsSetAudioCompression_t)(int handle, int compressionLevel);
           static dsSetAudioCompression_t dsSetAudioCompressionFunc = 0;
           if (dsSetAudioCompressionFunc == 0) {
               void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
               if (dllib) {
                   dsSetAudioCompressionFunc = (dsSetAudioCompression_t) dlsym(dllib, "dsSetAudioCompression");
                   if (dsSetAudioCompressionFunc) {
                       printf("dsSetAudioCompression_t(int, int ) is defined and loaded\r\n");
                       std::string _AudioCompression("0");
                       int m_audioCompression = 0;
                       try {
                           _AudioCompression = device::HostPersistence::getInstance().getProperty("audio.Compression");
                       }
                       catch(...) {
                           try {
                               printf("audio.Compression not found in persistence store. Try system default\n");
                               _AudioCompression = device::HostPersistence::getInstance().getDefaultProperty("audio.Compression");
                           }
                           catch(...) {
                               _AudioCompression = "0";
                           }
                       }
                       m_audioCompression = atoi(_AudioCompression.c_str());
           //SPEAKER init
                       handle = 0;
                       if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                           if (dsSetAudioCompressionFunc(handle, m_audioCompression) == dsERR_NONE) {
                               printf("Port %s: Initialized audio compression : %d\n","SPEAKER0", m_audioCompression);
                           }
                       }
           //HDMI init
                       handle = 0;
                       if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                           if (dsSetAudioCompressionFunc(handle, m_audioCompression) == dsERR_NONE) {
                               printf("Port %s: Initialized audio compression : %d\n","HDMI0", m_audioCompression);
                           }
                       }
                   }
                   else {
                       printf("dsSetAudioCompression_t(int, int) is not defined\r\n");
                   }
                   dlclose(dllib);
               }
               else {
                   printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
               }
           }


           typedef dsError_t (*dsSetDialogEnhancement_t)(int handle, int enhancerLevel);
           static dsSetDialogEnhancement_t dsSetDialogEnhancementFunc = 0;
           if (dsSetDialogEnhancementFunc == 0) {
               void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
               if (dllib) {
                   dsSetDialogEnhancementFunc = (dsSetDialogEnhancement_t) dlsym(dllib, "dsSetDialogEnhancement");
                   if (dsSetDialogEnhancementFunc) {
                       printf("dsSetDialogEnhancement_t(int, int) is defined and loaded\r\n");
                       std::string _EnhancerLevel("0");
                       int m_enhancerLevel = 0;
                       try {
                           _EnhancerLevel = device::HostPersistence::getInstance().getProperty("audio.EnhancerLevel");
                       }
                       catch(...) {
                           try {
                               printf("audio.EnhancerLevel not found in persistence store. Try system default\n");
                               _EnhancerLevel = device::HostPersistence::getInstance().getDefaultProperty("audio.EnhancerLevel");
                           }
                           catch(...) {
                               _EnhancerLevel = "0";
                           }
                       }
                       m_enhancerLevel = atoi(_EnhancerLevel.c_str());
           //SPEAKER init
                       handle = 0;
                       if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                           if (dsSetDialogEnhancementFunc(handle, m_enhancerLevel) == dsERR_NONE) {
                               printf("Port %s: Initialized dialog enhancement level : %d\n","SPEAKER0", m_enhancerLevel);
                           }
                       }
           //HDMI init
                       handle = 0;
                       if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                           if (dsSetDialogEnhancementFunc(handle, m_enhancerLevel) == dsERR_NONE) {
                               printf("Port %s: Initialized dialog enhancement level : %d\n","HDMI0", m_enhancerLevel);
                           }
                       }
                   }
                   else {
                       printf("dsSetDialogEnhancement_t(int, int ) is not defined\r\n");
                   }
                   dlclose(dllib);
               }
               else {
                   printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
               }
           }


           typedef dsError_t (*dsSetDolbyVolumeMode_t)(int handle, bool enable);
           static dsSetDolbyVolumeMode_t dsSetDolbyVolumeModeFunc = 0;
           if (dsSetDolbyVolumeModeFunc == 0) {
               dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
               if (dllib) {
                   dsSetDolbyVolumeModeFunc = (dsSetDolbyVolumeMode_t) dlsym(dllib, "dsSetDolbyVolumeMode");
                   if (dsSetDolbyVolumeModeFunc) {
                       printf("dsSetDolbyVolumeMode_t(int, bool) is defined and loaded\r\n");
                       std::string _DolbyVolumeMode("FALSE");
                       bool m_dolbyVolumeMode = false;
                       try {
                           _DolbyVolumeMode = device::HostPersistence::getInstance().getProperty("audio.DolbyVolumeMode");
                       }
                       catch(...) {
                           try {
                               printf("audio.DolbyVolumeMode not found in persistence store. Try system default\n");
                               _DolbyVolumeMode = device::HostPersistence::getInstance().getDefaultProperty("audio.DolbyVolumeMode");
                           }
                           catch(...) {
                               _DolbyVolumeMode = "FALSE";
                           }
                       }
                       if (_DolbyVolumeMode == "TRUE") {
                           m_dolbyVolumeMode = true;
                       }
                       else {
                           m_dolbyVolumeMode = false;
                       }
           //SPEAKER init
                       handle = 0;
                       if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                           if (dsSetDolbyVolumeModeFunc(handle, m_dolbyVolumeMode) == dsERR_NONE) {
                               printf("Port %s: Initialized Dolby Volume Mode : %d\n","SPEAKER0", m_dolbyVolumeMode);
                           }
                       }
           //HDMI init
                       handle = 0;
                       if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                           if (dsSetDolbyVolumeModeFunc(handle, m_dolbyVolumeMode) == dsERR_NONE) {
                               printf("Port %s: Initialized Dolby Volume Mode : %d\n","HDMI0", m_dolbyVolumeMode);
                           }
                       }
                   }
                   else {
                       printf("dsSetDolbyVolumeMode_t(int, bool) is not defined\r\n");
                   }
                   dlclose(dllib);
               }
               else {
                   printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
               }
           }

           typedef dsError_t (*dsSetIntelligentEqualizerMode_t)(int handle, int mode);
           static dsSetIntelligentEqualizerMode_t dsSetIntelligentEqualizerModeFunc = 0;
           if (dsSetIntelligentEqualizerModeFunc == 0) {
               dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
               if (dllib) {
                   dsSetIntelligentEqualizerModeFunc = (dsSetIntelligentEqualizerMode_t) dlsym(dllib, "dsSetIntelligentEqualizerMode");
                   if (dsSetIntelligentEqualizerModeFunc) {
                       printf("dsSetIntelligentEqualizerMode_t(int, int) is defined and loaded\r\n");
                       std::string _IEQMode("0");
                       int m_IEQMode = 0;
                       handle = 0;
                       dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle);
                       try {
                           _IEQMode = device::HostPersistence::getInstance().getProperty("audio.IntelligentEQ");
                       }
                       catch(...) {
                           try {
                               printf("audio.IntelligentEQ not found in persistence store. Try system default\n");
                               _IEQMode = device::HostPersistence::getInstance().getDefaultProperty("audio.IntelligentEQ");
                           }
                           catch(...) {
                               _IEQMode = "0";
                           }
                       }
                       m_IEQMode = atoi(_IEQMode.c_str());

           //SPEAKER init
                       handle = 0;
                       if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                           if (dsSetIntelligentEqualizerModeFunc(handle, m_IEQMode) == dsERR_NONE) {
                               printf("Port %s: Initialized Intelligent Equalizer mode : %d\n","SPEAKER0", m_IEQMode);
                           }
                       }
           //HDMI init
                       handle = 0;
                       if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                           if (dsSetIntelligentEqualizerModeFunc(handle, m_IEQMode) == dsERR_NONE) {
                               printf("Port %s: Initialized Intelligent Equalizer mode : %d\n","HDMI0", m_IEQMode);
                           }
                       }
                   }
                   else {
                       printf("dsSetIntelligentEqualizerMode_t(int, int) is not defined\r\n");
                   }
                   dlclose(dllib);
               }
               else {
                   printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
               }
           }

           typedef dsError_t (*dsSetVolumeLeveller_t)(int handle, dsVolumeLeveller_t volLeveller);
           static dsSetVolumeLeveller_t dsSetVolumeLevellerFunc = 0;
           if (dsSetVolumeLevellerFunc == 0) {
               dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
               if (dllib) {
                   dsSetVolumeLevellerFunc = (dsSetVolumeLeveller_t) dlsym(dllib, "dsSetVolumeLeveller");
                   if (dsSetVolumeLevellerFunc) {
                       printf("dsSetVolumeLeveller_t(int, dsVolumeLeveller_t) is defined and loaded\r\n");
		       std::string _volLevellerMode("0");
		       std::string _volLevellerLevel("0");
                       dsVolumeLeveller_t m_volumeLeveller;
                       try {
                           _volLevellerMode = device::HostPersistence::getInstance().getProperty("audio.VolumeLeveller.mode");
			   _volLevellerLevel = device::HostPersistence::getInstance().getProperty("audio.VolumeLeveller.level");
                       }
                       catch(...) {
                           try {
                               printf("audio.VolumeLeveller not found in persistence store. Try system default\n");
                               _volLevellerMode = device::HostPersistence::getInstance().getDefaultProperty("audio.VolumeLeveller.mode");
                               _volLevellerLevel = device::HostPersistence::getInstance().getDefaultProperty("audio.VolumeLeveller.level");
                           }
                           catch(...) {
                               _volLevellerMode = "0";
			       _volLevellerLevel = "0";
                           }
                       }
                       m_volumeLeveller.mode = atoi(_volLevellerMode.c_str());
		       m_volumeLeveller.level = atoi(_volLevellerLevel.c_str());
           //SPEAKER init
                       handle = 0;
                       if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                           if (dsSetVolumeLevellerFunc(handle, m_volumeLeveller) == dsERR_NONE) {
                               printf("Port %s: Initialized Volume Leveller : Mode: %d, Level: %d\n","SPEAKER0", m_volumeLeveller.mode, m_volumeLeveller.level);
                           }
                       }
           //HDMI init
                       handle = 0;
                       if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                           if (dsSetVolumeLevellerFunc(handle, m_volumeLeveller) == dsERR_NONE) {
                               printf("Port %s: Initialized Volume Leveller : Mode: %d, Level: %d\n","HDMI0", m_volumeLeveller.mode, m_volumeLeveller.level);
                           }
                       }
                   }
                   else {
                       printf("dsSetVolumeLeveller_t(int, dsVolumeLeveller_t) is not defined\r\n");
                   }
                   dlclose(dllib);
               }
               else {
                   printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
               }
           }


           typedef dsError_t (*dsSetBassEnhancer_t)(int handle, int boost);
           static dsSetBassEnhancer_t dsSetBassEnhancerFunc = 0;
           if (dsSetBassEnhancerFunc == 0) {
               dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
               if (dllib) {
                   dsSetBassEnhancerFunc = (dsSetBassEnhancer_t) dlsym(dllib, "dsSetBassEnhancer");
                   if (dsSetBassEnhancerFunc) {
                       printf("dsSetBassEnhancer_t(int, int) is defined and loaded\r\n");
                       std::string _BassBoost("0");
                       int m_bassBoost = 0;
                       try {
                           _BassBoost = device::HostPersistence::getInstance().getProperty("audio.BassBoost");
                       }
                       catch(...) {
                           try {
                               printf("audio.BassBoost not found in persistence store. Try system default\n");
                               std::string _Property = _dsGetCurrentProfileProperty("BassBoost");
                               _BassBoost = device::HostPersistence::getInstance().getDefaultProperty(_Property);
                           }
                           catch(...) {
                               _BassBoost = "0";
                           }
                       }
                       m_bassBoost = atoi(_BassBoost.c_str());
           //SPEAKER init
                       handle = 0;
                       if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                           if (dsSetBassEnhancerFunc(handle, m_bassBoost) == dsERR_NONE) {
                               printf("Port %s: Initialized Bass Boost : %d\n","SPEAKER0", m_bassBoost);
                           }
                       }
           //HDMI init
                       handle = 0;
                       if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                           if (dsSetBassEnhancerFunc(handle, m_bassBoost) == dsERR_NONE) {
                               printf("Port %s: Initialized Bass Boost : %d\n","HDMI0", m_bassBoost);
                           }
                       }
                   }
                   else {
                       printf("dsSetBassEnhancer_t(int, int) is not defined\r\n");
                   }
                   dlclose(dllib);
               }
               else {
                   printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
               }
           }


           typedef dsError_t (*dsEnableSurroundDecoder_t)(int handle, bool enabled);
           static dsEnableSurroundDecoder_t dsEnableSurroundDecoderFunc = 0;
           if (dsEnableSurroundDecoderFunc == 0) {
               dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
               if (dllib) {
                   dsEnableSurroundDecoderFunc = (dsEnableSurroundDecoder_t) dlsym(dllib, "dsEnableSurroundDecoder");
                   if (dsEnableSurroundDecoderFunc) {
                       printf("dsEnableSurroundDecoder_t(int, bool) is defined and loaded\r\n");
                       std::string _SurroundDecoder("FALSE");
                       bool m_surroundDecoder = false;
                       try {
                           _SurroundDecoder = device::HostPersistence::getInstance().getProperty("audio.SurroundDecoderEnabled");
                       }
                       catch(...) {
                           try {
                               printf("audio.SurroundDecoderEnabled not found in persistence store. Try system default\n");
                               _SurroundDecoder = device::HostPersistence::getInstance().getDefaultProperty("audio.SurroundDecoderEnabled");
                           }
                           catch(...) {
                               _SurroundDecoder = "FALSE";
                           }
                       }
                       if (_SurroundDecoder == "TRUE") {
                           m_surroundDecoder = true;
                       }
                       else {
                           m_surroundDecoder = false;
                       }
           //SPEAKER init
                       handle = 0;
                       if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                           if (dsEnableSurroundDecoderFunc(handle, m_surroundDecoder) == dsERR_NONE) {
                               printf("Port %s: Initialized Surroudn Decoder : %d\n","SPEAKER0", m_surroundDecoder);
                           }
                       }
           //HDMI init
                       handle = 0;
                       if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                           if (dsEnableSurroundDecoderFunc(handle, m_surroundDecoder) == dsERR_NONE) {
                               printf("Port %s: Initialized Surroudn Decoder : %d\n","HDMI0", m_surroundDecoder);
                           }
                       }
                   }
                   else {
                       printf("dsEnableSurroundDecoder_t(int, bool) is not defined\r\n");
                   }
                   dlclose(dllib);
               }
               else {
                   printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
               }
           }


           typedef dsError_t (*dsSetDRCMode_t)(int handle, int mode);
           static dsSetDRCMode_t dsSetDRCModeFunc = 0;
           if (dsSetDRCModeFunc == 0) {
               dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
               if (dllib) {
                   dsSetDRCModeFunc = (dsSetDRCMode_t) dlsym(dllib, "dsSetDRCMode");
                   if (dsSetDRCModeFunc) {
                       printf("dsSetDRCMode_t(int, int) is defined and loaded\r\n");
                       std::string _DRCMode("Line");
                       int m_DRCMode = 0;
                       try {
                           _DRCMode = device::HostPersistence::getInstance().getProperty("audio.DRCMode");
                       }
                       catch(...) {
                           try {
                               printf("audio.DRCMode not found in persistence store. Try system default\n");
                               _DRCMode = device::HostPersistence::getInstance().getDefaultProperty("audio.DRCMode");
                           }
                           catch(...) {
                               _DRCMode = "Line";
                           }
                       }
                       if (_DRCMode == "RF") {
                           m_DRCMode = 1;
                       }
                       else {
                           m_DRCMode = 0;
                       }
           //SPEAKER init
                       handle = 0;
                       if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                           if (dsSetDRCModeFunc(handle, m_DRCMode) == dsERR_NONE) {
                               printf("Port %s: Initialized DRCMode : %d\n","SPEAKER0", m_DRCMode);
                           }
                       }
           //HDMI init
                       handle = 0;
                       if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                           if (dsSetDRCModeFunc(handle, m_DRCMode) == dsERR_NONE) {
                               printf("Port %s: Initialized DRCMode : %d\n","HDMI0", m_DRCMode);
                           }
                       }
                   }
                   else {
                       printf("dsSetDRCMode_t(int, int) is not defined\r\n");
                   }
                   dlclose(dllib);
               }
               else {
                   printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
               }
           }


           typedef dsError_t (*dsSetSurroundVirtualizer_t)(int handle, dsSurroundVirtualizer_t virtualizer);
           static dsSetSurroundVirtualizer_t dsSetSurroundVirtualizerFunc = 0;
           if (dsSetSurroundVirtualizerFunc == 0) {
               dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
               if (dllib) {
                   dsSetSurroundVirtualizerFunc = (dsSetSurroundVirtualizer_t) dlsym(dllib, "dsSetSurroundVirtualizer");
                   if (dsSetSurroundVirtualizerFunc) {
                       printf("dsSetSurroundVirtualizer_t(int, dsSurroundVirtualizer_t) is defined and loaded\r\n");
                        std::string _SVMode("0");
                        std::string _SVBoost("0");
                        dsSurroundVirtualizer_t m_virtualizer;		 

                       try {
                            _SVMode = device::HostPersistence::getInstance().getProperty("audio.SurroundVirtualizer.mode");
                            _SVBoost = device::HostPersistence::getInstance().getProperty("audio.SurroundVirtualizer.boost");
                            m_virtualizer.mode = atoi(_SVMode.c_str());
                            m_virtualizer.boost = atoi(_SVBoost.c_str());			       
                       }
                       catch(...) {
                           try {
                               printf("audio.SurroundVirtualizer.mode/audio.SurroundVirtualizer.boost not found in persistence store. Try system default\n");
                               _SVMode = device::HostPersistence::getInstance().getProperty("audio.SurroundVirtualizer.mode");
                               _SVBoost = device::HostPersistence::getInstance().getProperty("audio.SurroundVirtualizer.boost"); 
                           }
                           catch(...) {
                               _SVMode = "0";
                               _SVBoost = "0";
                           }
                       }
                       m_virtualizer.mode = atoi(_SVMode.c_str());
		       m_virtualizer.boost = atoi(_SVBoost.c_str());
           //SPEAKER init
                       handle = 0;
                       if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                           if (dsSetSurroundVirtualizerFunc(handle, m_virtualizer) == dsERR_NONE) {
                               printf("Port %s: Initialized Surround Virtualizer : Mode: %d, Boost : %d\n","SPEAKER0", m_virtualizer.mode, m_virtualizer.boost);
                           }
                       }
           //HDMI init
                       handle = 0;
                       if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                           if (dsSetSurroundVirtualizerFunc(handle, m_virtualizer) == dsERR_NONE) {
                               printf("Port %s: Initialized Surround Virtualizer : Mode: %d, Boost : %d\\n","HDMI0", m_virtualizer.mode, m_virtualizer.boost);
                           }
                       }
                   }
                   else {
                       printf("dsSetSurroundVirtualizer_t(int, dsSurroundVirtualizer_t) is not defined\r\n");
                   }
                   dlclose(dllib);
               }
               else {
                   printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
               }
           }


           typedef dsError_t (*dsSetMISteering_t)(int handle, bool enabled);
           static dsSetMISteering_t dsSetMISteeringFunc = 0;
           if (dsSetMISteeringFunc == 0) {
               dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
               if (dllib) {
                   dsSetMISteeringFunc = (dsSetMISteering_t) dlsym(dllib, "dsSetMISteering");
                   if (dsSetMISteeringFunc) {
                       printf("dsSetMISteering_t(int, bool) is defined and loaded\r\n");
                       std::string _MISteering("Disabled");
                       bool m_MISteering = false;
                       try {
                           _MISteering = device::HostPersistence::getInstance().getProperty("audio.MISteering");
                       }
                       catch(...) {
                           try {
                               printf("audio.MISteering not found in persistence store. Try system default\n");
                               _MISteering = device::HostPersistence::getInstance().getDefaultProperty("audio.MISteering");
                           }
                           catch(...) {
                               _MISteering = "Disabled";
                           }
                       }
                       if (_MISteering == "Enabled") {
                           m_MISteering = true;
                       }
                       else {
                           m_MISteering = false;
                       }

           //SPEAKER init
                       handle = 0;
                       dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle);
                       if (dsSetMISteeringFunc(handle, m_MISteering) == dsERR_NONE) {
                           printf("Port %s: Initialized MI Steering : %d\n","SPEAKER0", m_MISteering);
                       }
           //HDMI init
                       handle = 0;
                       if (dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                           if (dsSetMISteeringFunc(handle, m_MISteering) == dsERR_NONE) {
                               printf("Port %s: Initialized MI Steering : %d\n","HDMI0", m_MISteering);
                           }
                       }
                       else {
                           printf("Port %s: Initialization MI Steering : %d failed. Port not available\n","HDMI0", m_MISteering);
                       }
                   }
                   else {
                       printf("dsSetMISteering_t(int, bool) is not defined\r\n");
                   }
                   dlclose(dllib);
               }
               else {
                   printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
               }
           }


           typedef dsError_t (*dsSetGraphicEqualizerMode_t)(int handle, int mode);
           static dsSetGraphicEqualizerMode_t dsSetGraphicEqualizerModeFunc = 0;
           if (dsSetGraphicEqualizerModeFunc == 0) {
               dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
               if (dllib) {
                   dsSetGraphicEqualizerModeFunc = (dsSetGraphicEqualizerMode_t) dlsym(dllib, "dsSetGraphicEqualizerMode");
                   if (dsSetGraphicEqualizerModeFunc) {
                       printf("dsSetGraphicEqualizerMode_t(int, int) is defined and loaded\r\n");
                       std::string _GEQMode("0");
                       int m_GEQMode = 0;
                       handle = 0;
                       dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle);
                       try {
                           _GEQMode = device::HostPersistence::getInstance().getProperty("audio.GraphicEQ");
                       }
                       catch(...) {
                           try {
                               printf("audio.GraphicEQ not found in persistence store. Try system default\n");
                               _GEQMode = device::HostPersistence::getInstance().getDefaultProperty("audio.GraphicEQ");
                           }
                           catch(...) {
                               _GEQMode = "0";
                           }
                       }
                       m_GEQMode = atoi(_GEQMode.c_str());

           //SPEAKER init
                       handle = 0;
                       if(dsGetAudioPort(dsAUDIOPORT_TYPE_SPEAKER,0,&handle) == dsERR_NONE) {
                           if (dsSetGraphicEqualizerModeFunc(handle, m_GEQMode) == dsERR_NONE) {
                               printf("Port %s: Initialized Graphic Equalizer mode : %d\n","SPEAKER0", m_GEQMode);
                           }
                       }
           //HDMI init
                       handle = 0;
                       if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI,0,&handle) == dsERR_NONE) {
                           if (dsSetGraphicEqualizerModeFunc(handle, m_GEQMode) == dsERR_NONE) {
                               printf("Port %s: Initialized Graphic Equalizer mode : %d\n","HDMI0", m_GEQMode);
                           }
                       }
                   }
                   else {
                       printf("dsSetGraphicEqualizerMode_t(int, int) is not defined\r\n");
                   }
                   dlclose(dllib);
               }
               else {
                   printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
               }
           }
    }
    /* HDMI ARC digital audio mode settings */
    handle = 0;
    if(dsGetAudioPort(dsAUDIOPORT_TYPE_HDMI_ARC,0,&handle) == dsERR_NONE) {
       std::string _ARCAudioModeAuto("FALSE");
       try {
          _ARCAudioModeAuto = device::HostPersistence::getInstance().getProperty("HDMI_ARC0.AudioMode.AUTO");
       }
       catch(...) {
          try {
                printf("HDMI_ARC0.AudioMode.AUTO not found in persistence store. Try system default\n");
                _ARCAudioModeAuto = device::HostPersistence::getInstance().getDefaultProperty("HDMI_ARC0.AudioMode.AUTO");
          }
          catch(...) {
                   _ARCAudioModeAuto = "FALSE";
         }
      }
      if (_ARCAudioModeAuto.compare("TRUE") == 0)
      {
         typedef dsError_t (*dsSetStereoAuto_t)(int handle, int autoMode);
         static dsSetStereoAuto_t func = 0;
         if (func == 0) {
            void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
            if (dllib) {
                func = (dsSetStereoAuto_t) dlsym(dllib, "dsSetStereoAuto");
                if (func) {
                    printf("dsSetStereoAuto_t(int, int *) is defined and loaded\r\n");
                }
                else {
                    printf("dsSetStereoAuto_t(int, int *) is not defined\r\n");
                }
                dlclose(dllib);
            }
            else {
                    printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
            }
        }

        if (func != 0 && handle != NULL)
        {
            if (func(handle, 1) == dsERR_NONE)
            {
                 printf("dsSetStereoAuto Port HDMI_0ARC Audio Mode is set to Auto \n");
            }
        }

    }
    else if (_ARCAudioModeAuto.compare("FALSE") == 0)
    {
       if (NULL != handle) {
           if (dsSetStereoMode(handle, _srv_HDMI_ARC_Audiomode) == dsERR_NONE)
           {
              printf("dsSetStereoMode The HDMI ARC Port Audio Settings Mode is %d \r\n",_srv_HDMI_ARC_Audiomode);
           }
      }
   }
 }
 

#endif //DS_AUDIO_SETTINGS_PERSISTENCE
}

IARM_Result_t dsAudioMgr_init()
{
   IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsAudioPortInit, _dsAudioPortInit);
   IARM_BUS_Lock(lock);
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
                else if (_AudioModeSettings.compare("DOLBYDIGITAL") == 0)
                {
                        _srv_HDMI_Audiomode = dsAUDIO_STEREO_DD;
                }
                else if (_AudioModeSettings.compare("DOLBYDIGITALPLUS") == 0)
                {
                        _srv_HDMI_Audiomode = dsAUDIO_STEREO_DDPLUS;
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

            try {
		_AudioModeAuto = device::HostPersistence::getInstance().getProperty("HDMI0.AudioMode.AUTO",_AudioModeAuto);
	    }
	    catch(...) {
	        _AudioModeAuto = false;
	    }
           std::string _ARCAudioModeAuto("FALSE");
	   std::string _SPDIFAudioModeAuto("FALSE");
	   try {
		_ARCAudioModeAuto = device::HostPersistence::getInstance().getProperty("HDMI_ARC0.AudioMode.AUTO");
	   }
	   catch(...) {
               try {
                   printf("HDMI_ARC0.AudioMode.AUTO not found in persistence store. Try system default\n");
                   _ARCAudioModeAuto = device::HostPersistence::getInstance().getDefaultProperty("HDMI_ARC0.AudioMode.AUTO");
               }
               catch(...) {
                   _ARCAudioModeAuto = "FALSE";
               }

	   }

           try {
                _SPDIFAudioModeAuto = device::HostPersistence::getInstance().getProperty("SPDIF0.AudioMode.AUTO");
           }
           catch(...) {
               try {
                   printf("SPDIF0.AudioMode.AUTO not found in persistence store. Try system default\n");
                   _SPDIFAudioModeAuto = device::HostPersistence::getInstance().getDefaultProperty("SPDIF0.AudioMode.AUTO");
               }
               catch(...) {
                   _SPDIFAudioModeAuto = "FALSE";
               }
           }

           if ((_AudioModeAuto.compare("TRUE") == 0) || (_ARCAudioModeAuto.compare("TRUE") == 0) || (_SPDIFAudioModeAuto.compare("TRUE") == 0))
	    {
	        _srv_AudioAuto = 1;
	    }
        else
        {
			_srv_AudioAuto = 0;
        }
		__TIMESTAMP();printf("The HDMI Audio Auto Setting on startup  is %s \r\n",_AudioModeAuto.c_str());
                __TIMESTAMP();printf("The HDMI ARC Audio Auto Setting on startup  is %s \r\n",_ARCAudioModeAuto.c_str());
                __TIMESTAMP();printf("The SPDIF Audio Auto Setting on startup  is %s \r\n",_SPDIFAudioModeAuto.c_str());
		
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
                /* Get the AudioModesettings for HDMI_ARC from Persistence */
                std::string _ARCModeSettings("STEREO");
                _ARCModeSettings = device::HostPersistence::getInstance().getProperty("HDMI_ARC0.AudioMode",_ARCModeSettings);
                __TIMESTAMP();printf("The HDMI ARC Audio Mode Setting on startup  is %s \r\n",_ARCModeSettings.c_str());
                if (_ARCModeSettings.compare("SURROUND") == 0)
                {
                        _srv_HDMI_ARC_Audiomode = dsAUDIO_STEREO_SURROUND;
                }
                else if (_ARCModeSettings.compare("PASSTHRU") == 0)
                {
                        _srv_HDMI_ARC_Audiomode = dsAUDIO_STEREO_PASSTHRU;
                }
                else
                {
                        _srv_HDMI_ARC_Audiomode = dsAUDIO_STEREO_STEREO;
                }
	}
	catch(...) 
	{
		printf("Exception in Getting the Audio  settings on Startup..... \r\n");
	}
    	if (!m_isPlatInitialized) {
    		dsAudioPortInit();
                AudioConfigInit();
	   	}
        /*coverity[missing_lock]  CID-19380 using Coverity Annotation to ignore error*/
        m_isPlatInitialized ++;
        {
           IARM_Bus_DSMgr_EventData_t audio_portstate_event_data;
           audio_portstate_event_data.data.AudioPortStateInfo.audioPortState = dsAUDIOPORT_STATE_INITIALIZED;
           printf("%s: AudioOutPort PortInitState:%d \r\n", __FUNCTION__, audio_portstate_event_data.data.AudioPortStateInfo.audioPortState);
           IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,
                           (IARM_EventId_t)IARM_BUS_DSMGR_EVENT_AUDIO_PORT_STATE,
                           (void *)&audio_portstate_event_data,
                           sizeof(audio_portstate_event_data));

       }

        IARM_BUS_Unlock(lock);  //CID:136568 - Data race condition
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
	IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetSupportedARCTypes,_dsGetSupportedARCTypes);
	IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsAudioSetSAD,_dsAudioSetSAD);
	IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsAudioEnableARC,_dsAudioEnableARC);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetStereoMode,_dsSetStereoMode);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetStereoMode,_dsGetStereoMode);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetStereoAuto,_dsSetStereoAuto);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetStereoAuto,_dsGetStereoAuto);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetAudioMute,_dsSetAudioMute);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsIsAudioMute,_dsIsAudioMute);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetAudioDucking,_dsSetAudioDucking);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetAudioLevel,_dsSetAudioLevel);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetAudioLevel,_dsGetAudioLevel);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetAudioGain,_dsSetAudioGain);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetAudioGain,_dsGetAudioGain);
	IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetAudioFormat,_dsGetAudioFormat);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetEncoding,_dsGetEncoding);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsIsAudioMSDecode,_dsIsAudioMSDecode);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsIsAudioMS12Decode,_dsIsAudioMS12Decode);

        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsIsAudioPortEnabled,_dsIsAudioPortEnabled);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsEnableAudioPort,_dsEnableAudioPort);

        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetEnablePersist, _dsGetEnablePersist);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetEnablePersist, _dsSetEnablePersist);

        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsAudioPortTerm,_dsAudioPortTerm);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsEnableLEConfig,_dsEnableLEConfig);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetLEConfig,_dsGetLEConfig);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetAudioDelay, _dsSetAudioDelay);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetAudioDelay, _dsGetAudioDelay);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetAudioDelayOffset, _dsSetAudioDelayOffset);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetAudioDelayOffset, _dsGetAudioDelayOffset);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetSinkDeviceAtmosCapability, _dsGetSinkDeviceAtmosCapability);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetAudioAtmosOutputMode, _dsSetAudioAtmosOutputMode);      
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetAudioAtmosOutputMode, _dsSetAudioAtmosOutputMode);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetAudioCompression, _dsSetAudioCompression);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetAudioCompression, _dsGetAudioCompression);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetDialogEnhancement, _dsSetDialogEnhancement);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetDialogEnhancement, _dsGetDialogEnhancement);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetDolbyVolumeMode, _dsSetDolbyVolumeMode);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetDolbyVolumeMode	, _dsGetDolbyVolumeMode);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetIntelligentEqualizerMode, _dsSetIntelligentEqualizerMode);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetIntelligentEqualizerMode, _dsGetIntelligentEqualizerMode);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetVolumeLeveller, _dsGetVolumeLeveller);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetVolumeLeveller, _dsSetVolumeLeveller);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetBassEnhancer, _dsGetBassEnhancer);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetBassEnhancer, _dsSetBassEnhancer);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsIsSurroundDecoderEnabled, _dsIsSurroundDecoderEnabled);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsEnableSurroundDecoder, _dsEnableSurroundDecoder);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetDRCMode, _dsGetDRCMode);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetDRCMode, _dsSetDRCMode);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetSurroundVirtualizer, _dsGetSurroundVirtualizer);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetSurroundVirtualizer, _dsSetSurroundVirtualizer);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetMISteering, _dsGetMISteering);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetMISteering, _dsSetMISteering);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetGraphicEqualizerMode, _dsGetGraphicEqualizerMode);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetGraphicEqualizerMode, _dsSetGraphicEqualizerMode);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetMS12AudioProfileList, _dsGetMS12AudioProfileList);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetMS12AudioProfile, _dsGetMS12AudioProfile);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetMS12AudioProfile, _dsSetMS12AudioProfile);

        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetAssociatedAudioMixing, _dsSetAssociatedAudioMixing);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetAssociatedAudioMixing, _dsGetAssociatedAudioMixing);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetFaderControl, _dsSetFaderControl);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetFaderControl, _dsGetFaderControl);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetPrimaryLanguage, _dsSetPrimaryLanguage);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetPrimaryLanguage, _dsGetPrimaryLanguage);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetSecondaryLanguage, _dsSetSecondaryLanguage);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetSecondaryLanguage, _dsGetSecondaryLanguage);

        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetAudioCapabilities,_dsGetAudioCapabilities); 
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetMS12Capabilities,_dsGetMS12Capabilities); 
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsAudioOutIsConnected, _dsAudioOutIsConnected); 
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsSetMS12SetttingsOverride, _dsSetMS12SetttingsOverride);

        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsResetDialogEnhancement,_dsResetDialogEnhancement);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsResetBassEnhancer,_dsResetBassEnhancer);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsResetSurroundVirtualizer,_dsResetSurroundVirtualizer);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsResetVolumeLeveller,_dsResetVolumeLeveller);
        IARM_Bus_RegisterCall(IARM_BUS_DSMGR_API_dsGetHDMIARCPortId, _dsGetHDMIARCPortId);

        dsError_t eRet = _dsAudioOutRegisterConnectCB (_dsAudioOutPortConnectCB);
        if (dsERR_NONE != eRet) {
            printf ("%s: _dsAudioOutRegisterConnectCB eRet:%04x", __FUNCTION__, eRet);
        }

        eRet = _dsAudioFormatUpdateRegisterCB (_dsAudioFormatUpdateCB) ;
        if (dsERR_NONE != eRet) {
            printf ("%s: _dsAudioFormatUpdateRegisterCB eRet:%04x", __FUNCTION__, eRet);
        }

        m_isInitialized = 1;
    }
    
    if (!m_isPlatInitialized) {
        /* Nexus init, if any here */
        dsAudioPortInit();
        AudioConfigInit();
    }
    m_isPlatInitialized++;
 

 IARM_BUS_Unlock(lock);

 return IARM_RESULT_SUCCESS;

}

IARM_Result_t _dsGetAudioPort(void *arg)
{
    _DEBUG_ENTER();

    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    dsError_t ret = dsERR_NONE;

    IARM_BUS_Lock(lock);

    dsAudioGetHandleParam_t *param = (dsAudioGetHandleParam_t *)arg;

    if (param != NULL)
    {
        printf("%s..%d-%d \r\n",__func__,param->type,param->index);
        ret = dsGetAudioPort(param->type, param->index, &param->handle);
        if(ret == dsERR_NONE) {
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);

    return result;
}



IARM_Result_t _dsGetStereoMode(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    dsError_t ret = dsERR_NONE;
    dsAudioSetStereoModeParam_t *param = (dsAudioSetStereoModeParam_t *)arg;

    if (param != NULL && param->toPersist && NULL != param->handle) {
        _GetAudioModeFromPersistent(arg);
        result = IARM_RESULT_SUCCESS;
    }
    else if (param != NULL && NULL != param->handle)
    {
        /* In Auto Mode, get the effective mode */
        if (_srv_AudioAuto) {
            dsAudioStereoMode_t stereoMode = dsAUDIO_STEREO_UNKNOWN;
            ret = dsGetStereoMode(param->handle, &stereoMode);
            if(ret == dsERR_NONE) {
                result = IARM_RESULT_SUCCESS;
            }
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
            else if (_APortType == dsAUDIOPORT_TYPE_HDMI_ARC) {
                param->mode = _srv_HDMI_ARC_Audiomode;
                printf("The HDMI ARC Port Audio Settings Mode is %d \r\n",param->mode);
            }

            result = IARM_RESULT_SUCCESS;
        } 
   }

    IARM_BUS_Unlock(lock);    

    return result;
}


IARM_Result_t _dsSetStereoMode(void *arg)
{
    _DEBUG_ENTER();
    IARM_Bus_DSMgr_EventData_t eventData;

    IARM_BUS_Lock(lock);

    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    dsError_t ret = dsERR_NONE;
    dsAudioSetStereoModeParam_t *param = (dsAudioSetStereoModeParam_t *)arg;
    if (NULL != param->handle) {
        ret = dsSetStereoMode(param->handle, param->mode);
        param->rpcResult = ret;
    }
    else {
        ret = dsERR_INVALID_PARAM;
        param->rpcResult = dsERR_INVALID_PARAM;
    }

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
                else if (_APortType == dsAUDIOPORT_TYPE_HDMI_ARC)
                {
                    if (param->toPersist)
                    device::HostPersistence::getInstance().persistHostProperty("HDMI_ARC0.AudioMode","STEREO");

                    _srv_HDMI_ARC_Audiomode = dsAUDIO_STEREO_STEREO;
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
                else if (_APortType == dsAUDIOPORT_TYPE_HDMI_ARC)
                {
                    if (param->toPersist)
                    device::HostPersistence::getInstance().persistHostProperty("HDMI_ARC0.AudioMode","SURROUND");

                    _srv_HDMI_ARC_Audiomode = dsAUDIO_STEREO_SURROUND;
                }

                eventData.data.Audioport.mode = dsAUDIO_STEREO_SURROUND;
                eventData.data.Audioport.type = _APortType;
                IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,(IARM_EventId_t)IARM_BUS_DSMGR_EVENT_AUDIO_MODE,(void *)&eventData, sizeof(eventData));
            }
            else if(param->mode == dsAUDIO_STEREO_DD)
            {
                __TIMESTAMP();printf("Setting Audio Mode Dolby Digital with persistent value %d \r\n",param->toPersist);

                if (_APortType == dsAUDIOPORT_TYPE_HDMI)
                {
                    if (param->toPersist)
                    device::HostPersistence::getInstance().persistHostProperty("HDMI0.AudioMode","DOLBYDIGITAL");

                    _srv_HDMI_Audiomode = dsAUDIO_STEREO_DD;
                }
                eventData.data.Audioport.mode = dsAUDIO_STEREO_DD;
                eventData.data.Audioport.type = _APortType;
                IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,(IARM_EventId_t)IARM_BUS_DSMGR_EVENT_AUDIO_MODE,(void *)&eventData, sizeof(eventData));
            }
            else if(param->mode == dsAUDIO_STEREO_DDPLUS)
            {
                __TIMESTAMP();printf("Setting Audio Mode Dolby Digital Plus with persistent value %d \r\n",param->toPersist);

                if (_APortType == dsAUDIOPORT_TYPE_HDMI)
                {
                    if (param->toPersist)
                    device::HostPersistence::getInstance().persistHostProperty("HDMI0.AudioMode","DOLBYDIGITALPLUS");

                    _srv_HDMI_Audiomode = dsAUDIO_STEREO_DDPLUS;
                }
                eventData.data.Audioport.mode = dsAUDIO_STEREO_DDPLUS;
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
                else if (_APortType == dsAUDIOPORT_TYPE_HDMI_ARC)
                {
                    if (param->toPersist)
                    device::HostPersistence::getInstance().persistHostProperty("HDMI_ARC0.AudioMode","PASSTHRU");

                    _srv_HDMI_ARC_Audiomode = dsAUDIO_STEREO_PASSTHRU;
                }

                eventData.data.Audioport.mode = dsAUDIO_STEREO_PASSTHRU;
                eventData.data.Audioport.type = _APortType;
                IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,(IARM_EventId_t)IARM_BUS_DSMGR_EVENT_AUDIO_MODE,(void *)&eventData, sizeof(eventData));

            }

            result = IARM_RESULT_SUCCESS;
        }
        catch(...)
        {
            printf("Error in Setting audio mode... \r\n");
        }

    }

    IARM_BUS_Unlock(lock);

    return result;
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

    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    dsAudioSetStereoAutoParam_t *param = (dsAudioSetStereoAutoParam_t *)arg;

    if(param == NULL)
    {
	printf("Param is  null\r\n");
    }

    dsAudioPortType_t _APortType = _GetAudioPortType(param->handle);

    if (param->toPersist) {
	switch(_APortType) {
	    case dsAUDIOPORT_TYPE_HDMI:
	        device::HostPersistence::getInstance().persistHostProperty("HDMI0.AudioMode.AUTO", param->autoMode ? "TRUE" : "FALSE");
		break;

	    case dsAUDIOPORT_TYPE_HDMI_ARC:
	        device::HostPersistence::getInstance().persistHostProperty("HDMI_ARC0.AudioMode.AUTO", param->autoMode ? "TRUE" : "FALSE");
		break;

	    case dsAUDIOPORT_TYPE_SPDIF:
		device::HostPersistence::getInstance().persistHostProperty("SPDIF0.AudioMode.AUTO", param->autoMode ? "TRUE" : "FALSE");
		break;

	    default:
		break;
	}
    }

    if ((_APortType == dsAUDIOPORT_TYPE_HDMI_ARC) || (_APortType == dsAUDIOPORT_TYPE_SPDIF)) {
        typedef dsError_t (*dsSetStereoAuto_t)(int handle, int autoMode);
        static dsSetStereoAuto_t func = 0;
        if (func == 0) {
            void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
            if (dllib) {
                func = (dsSetStereoAuto_t) dlsym(dllib, "dsSetStereoAuto");
                if (func) {
                    printf("dsSetStereoAuto_t(int, int *) is defined and loaded\r\n");
                }
                else {
                    printf("dsSetStereoAuto_t(int, int *) is not defined\r\n");
                }
                dlclose(dllib);
            }
            else {
                printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
            }
        }

        if (func != 0 && param != NULL)
        {
            if (func(param->handle, param->autoMode) == dsERR_NONE)
            {
               result = IARM_RESULT_SUCCESS;
            }
        }
    }

    _srv_AudioAuto = param->autoMode ? 1 : 0;

    IARM_BUS_Unlock(lock);
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsSetAudioDucking(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);
    int volume = 0;
    dsAudioSetDuckingParam_t *param = (dsAudioSetDuckingParam_t *)arg;
    IARM_Bus_DSMgr_EventData_t eventData;
    printf("%s action : %d type :%d val :%d m_volumeLevel:%f \n",__FUNCTION__,param->action,param->type,param->level,m_volumeLevel );
    if(m_MuteStatus)
    {
        printf("%s mute on so ignore the duckig request\n",__FUNCTION__);
        IARM_BUS_Unlock(lock);
        return IARM_RESULT_SUCCESS;
    }

    if(param->action == dsAUDIO_DUCKINGACTION_START)
    {
        m_isDuckingInProgress = true;
	if(param->type == dsAUDIO_DUCKINGTYPE_RELATIVE )
	{
             volume = (m_volumeLevel * param->level) / 100;
	}
	else
	{
           if(param->level > m_volumeLevel)
           {
		 volume =  m_volumeLevel;
	   }
           else
	   {
        	 volume = param->level;
           }
	}
    }
    else
    {
	m_isDuckingInProgress = false;
	volume = m_volumeLevel;
    }
    printf(":%s adjusted volume volume :%d m_volumeDuckingLevel :%d\n",__FUNCTION__,volume,m_volumeDuckingLevel );

    // apply volume to hal layer
    dsAudioPortType_t _APortType = _GetAudioPortType(param->handle);

    typedef dsError_t (*dsSetAudioLevel_t)(int handle, float level);
    static dsSetAudioLevel_t func = 0;
    if (func == 0)
    {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetAudioLevel_t) dlsym(dllib, "dsSetAudioLevel");
            if (func) {
                printf("dsSetAudioLevel_t(int, float ) is defined and loaded \r\n");
	    }
            else {
                printf("dsSetAudioLevel_t(int, float ) is not defined \r\n");
	    }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed \r\n", RDK_DSHAL_NAME);
	}
    }
    if (func != 0 )
    {
        func(param->handle, volume);
    }

    if(volume != m_volumeDuckingLevel)
    {
        m_volumeDuckingLevel = volume;

        dsAudioStereoMode_t mode = dsAUDIO_STEREO_STEREO;
        if (_APortType == dsAUDIOPORT_TYPE_SPDIF)
        {
                mode = _srv_SPDIF_Audiomode;
        }
        else if (_APortType == dsAUDIOPORT_TYPE_HDMI) {
                mode = _srv_HDMI_Audiomode;
        }
        printf("The Port type is :%d  Audio Settings Mode is %d \r\n",_APortType, mode);

        if(mode == dsAUDIO_STEREO_PASSTHRU && volume != 100)
        {
            eventData.data.AudioLevelInfo.level = 0;
            printf(" IARM_BUS_DSMGR_EVENT_AUDIO_LEVEL_CHANGED PASSTHRU mode volume:%d \n",eventData.data.AudioLevelInfo.level);
        }
        else
        {
            eventData.data.AudioLevelInfo.level = volume;
            printf(" IARM_BUS_DSMGR_EVENT_AUDIO_LEVEL_CHANGED  volume:%d \n ",eventData.data.AudioLevelInfo.level);
        }
        IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,(IARM_EventId_t)IARM_BUS_DSMGR_EVENT_AUDIO_LEVEL_CHANGED,(void *)&eventData, sizeof(eventData));
    }
    IARM_BUS_Unlock(lock);

    return IARM_RESULT_SUCCESS;
}

IARM_Result_t _dsGetAudioGain(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetAudioGain_t)(int handle, float *gain);
    static dsGetAudioGain_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetAudioGain_t) dlsym(dllib, "dsGetAudioGain");
            if (func) {
                printf("dsGetAudioGain_t(int, float *) is defined and loaded\r\n");
            }
            else {
                printf("dsGetAudioGain_t(int, float *) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsAudioGainParam_t *param = (dsAudioGainParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        float gain = 0;
        param->gain = 0;
        if (func(param->handle, &gain) == dsERR_NONE)
        {
           param->gain = gain;
           result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}

IARM_Result_t _dsGetAudioLevel(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetAudioLevel_t)(int handle, float *level);
    static dsGetAudioLevel_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetAudioLevel_t) dlsym(dllib, "dsGetAudioLevel");
            if (func) {
                printf("dsGetAudioLevel_t(int, float *) is defined and loaded\r\n");
            }
            else {
                printf("dsGetAudioLevel_t(int, float *) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsAudioSetLevelParam_t *param = (dsAudioSetLevelParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        float level = 0;
        param->level = 0;
        if (func(param->handle, &level) == dsERR_NONE)
        {
           param->level = level;
           result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}

IARM_Result_t _dsSetAudioGain(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsSetAudioGain_t)(int handle, float gain);
    static dsSetAudioGain_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetAudioGain_t) dlsym(dllib, "dsSetAudioGain");
            if (func) {
                printf("dsSetAudioGain_t(int, float ) is defined and loaded\r\n");
            }
            else {
                printf("dsSetAudioGain_t(int, float ) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsAudioGainParam_t *param = (dsAudioGainParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        if (func(param->handle, param->gain) == dsERR_NONE)
        {
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
            std::string _AudioGain = std::to_string(param->gain);
            dsAudioPortType_t _APortType = _GetAudioPortType(param->handle);
            switch(_APortType) {
                case dsAUDIOPORT_TYPE_SPDIF:
                    printf("%s: port: %s , persist audio gain: %f\n",__func__,"SPDIF0", param->gain);
                    device::HostPersistence::getInstance().persistHostProperty("SPDIF0.audio.Gain",_AudioGain);
                    break;
                case dsAUDIOPORT_TYPE_HDMI:
                    printf("%s: port: %s , persist audio gain: %f\n",__func__,"HDMI0", param->gain);
                    device::HostPersistence::getInstance().persistHostProperty("HDMI0.audio.Gain",_AudioGain);
                    break;
                case dsAUDIOPORT_TYPE_SPEAKER:
                    printf("%s: port: %s , persist audio gain: %f\n",__func__,"SPEAKER0", param->gain);
                    device::HostPersistence::getInstance().persistHostProperty("SPEAKER0.audio.Gain",_AudioGain);
                    break;
                default:
                    break; 
            }
#endif
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);

    return result;

}

IARM_Result_t _dsSetAudioLevel(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsSetAudioLevel_t)(int handle, float level);
    static dsSetAudioLevel_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetAudioLevel_t) dlsym(dllib, "dsSetAudioLevel");
            if (func) {
                printf("dsSetAudioLevel_t(int, float ) is defined and loaded\r\n");
            }
            else {
                printf("dsSetAudioLevel_t(int, float ) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsAudioSetLevelParam_t *param = (dsAudioSetLevelParam_t *)arg;

    if (func != 0 && param != NULL)
    {
	printf("_dsSetAudioLevel param->level :%f m_isDuckingInProgress :%d  \n",param->level,m_isDuckingInProgress);
        if (m_isDuckingInProgress || func(param->handle, param->level) == dsERR_NONE)
        {
		m_volumeLevel = (int) param->level;
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
            std::string _AudioLevel = std::to_string(param->level);
            dsAudioPortType_t _APortType = _GetAudioPortType(param->handle);
            switch(_APortType) {
                case dsAUDIOPORT_TYPE_SPDIF:
                    printf("%s: port: %s , persist audio level: %f\n",__func__,"SPDIF0", param->level);
                    device::HostPersistence::getInstance().persistHostProperty("SPDIF0.audio.Level",_AudioLevel);
                    break;
                case dsAUDIOPORT_TYPE_HDMI:
                    printf("%s: port: %s , persist audio level: %f\n",__func__,"HDMI0", param->level);
                    device::HostPersistence::getInstance().persistHostProperty("HDMI0.audio.Level",_AudioLevel);
                    break;
                case dsAUDIOPORT_TYPE_SPEAKER:
                    printf("%s: port: %s , persist audio level: %f\n",__func__,"SPEAKER0", param->level);
                    device::HostPersistence::getInstance().persistHostProperty("SPEAKER0.audio.Level",_AudioLevel);
                    break;
                case dsAUDIOPORT_TYPE_HEADPHONE:
                    printf("%s: port: %s , persist audio level: %f\n",__func__,"HEADPHONE0", param->level);
                    device::HostPersistence::getInstance().persistHostProperty("HEADPHONE0.audio.Level",_AudioLevel);
                    break;
                default:
                    break;
            }
#endif
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);

    return result;

}

IARM_Result_t _dsSetAudioMute(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    dsError_t ret = dsERR_NONE;

    dsAudioSetMutedParam_t *param = (dsAudioSetMutedParam_t *)arg;
    ret = dsSetAudioMute(param->handle, param->mute);
    if (ret == dsERR_NONE) {
	    m_MuteStatus = param->mute;
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
            std::string _mute = param->mute ? "TRUE" : "FALSE";
            dsAudioPortType_t _APortType = _GetAudioPortType(param->handle);
            switch(_APortType) {
                case dsAUDIOPORT_TYPE_SPDIF:
                    printf("%s: port: %s , persist audio mute: %s\n",__func__,"SPDIF0", param->mute ? "TRUE" : "FALSE");
                    device::HostPersistence::getInstance().persistHostProperty("SPDIF0.audio.mute", _mute);
                    break;
                case dsAUDIOPORT_TYPE_HDMI:
                    printf("%s: port: %s , persist audio mute: %s\n",__func__,"HDMI0", param->mute ? "TRUE" : "FALSE");
                    device::HostPersistence::getInstance().persistHostProperty("HDMI0.audio.mute", _mute);
                    break;
                case dsAUDIOPORT_TYPE_SPEAKER:
                    printf("%s: port: %s , persist audio mute: %s\n",__func__,"SPEAKER0", param->mute ? "TRUE" : "FALSE");
                    device::HostPersistence::getInstance().persistHostProperty("SPEAKER0.audio.mute", _mute);
                    break;
                case dsAUDIOPORT_TYPE_HEADPHONE:
                    printf("%s: port: %s , persist audio mute: %s\n",__func__,"HEADPHONE0", param->mute ? "TRUE" : "FALSE");
                    device::HostPersistence::getInstance().persistHostProperty("HEADPHONE0.audio.mute", _mute);
                    break;
                default:
                    break;
            }
#endif
        result = IARM_RESULT_SUCCESS;
    }

    IARM_BUS_Unlock(lock);

    return result;
}


IARM_Result_t _dsIsAudioMute(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    IARM_Result_t result = IARM_RESULT_INVALID_STATE;

    dsAudioSetMutedParam_t *param = (dsAudioSetMutedParam_t *)arg;
    bool muted = false;
    
    dsError_t ret = dsIsAudioMute(param->handle, &muted);
    if (ret == dsERR_NONE) {
        param->mute = muted;

#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
        std::string isMuteKey("");
        std::string _mute("FALSE");
        dsAudioPortType_t _APortType = _GetAudioPortType(param->handle);
            switch(_APortType) {
                case dsAUDIOPORT_TYPE_SPDIF:
                    isMuteKey.append("SPDIF0.audio.mute");
                    break;
                case dsAUDIOPORT_TYPE_HDMI:
                    isMuteKey.append("HDMI0.audio.mute");
                    break;
                case dsAUDIOPORT_TYPE_SPEAKER:
                    isMuteKey.append("SPEAKER0.audio.mute");
                    break;
                case dsAUDIOPORT_TYPE_HEADPHONE:
                    isMuteKey.append("HEADPHONE0.audio.mute");
                    break;
                default:
                    break;
            }
        try {
            _mute = device::HostPersistence::getInstance().getProperty(isMuteKey);
        }
        catch(...) {
            printf("%s : Exception in getting the %s from persistence storage\n", __FUNCTION__, isMuteKey.c_str());
            _mute = "FALSE";
        }
        if ("TRUE" == _mute) {
            param->mute = true;
        }
        printf("%s: persist value:%s for :%s\n", __FUNCTION__, _mute.c_str(), isMuteKey.c_str());
#endif //DS_AUDIO_SETTINGS_PERSISTENCE end

        result = IARM_RESULT_SUCCESS;
    }

    IARM_BUS_Unlock(lock);

    return result;
}


IARM_Result_t _dsIsAudioPortEnabled(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    IARM_Result_t result = IARM_RESULT_INVALID_STATE;

    dsAudioPortEnabledParam_t *param = (dsAudioPortEnabledParam_t *)arg;
    bool enabled = false;
    
    dsError_t ret = dsIsAudioPortEnabled(param->handle, &enabled);
    if (ret == dsERR_NONE) {
        param->enabled = enabled;
        result = IARM_RESULT_SUCCESS;
    }
    printf("%s : returned ret: %04x enabled: %s\n", __FUNCTION__, ret, param->enabled? "TRUE":"FALSE");

    IARM_BUS_Unlock(lock);

    return result;
}


IARM_Result_t _dsEnableAudioPort(void *arg)
{
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    dsError_t ret = dsERR_NONE;

    dsAudioPortEnabledParam_t *param = (dsAudioPortEnabledParam_t *)arg;
    ret = dsEnableAudioPort(param->handle, param->enabled);
    if(ret == dsERR_NONE) {
        result = IARM_RESULT_SUCCESS;
    }

    std::string isEnabledAudioPortKey("audio.");
    isEnabledAudioPortKey.append (param->portName);
    isEnabledAudioPortKey.append (".isEnabled");


    /*Ensure settings is enabled properly in HAL*/
    bool bAudioPortEnableVerify = false;
    ret = dsIsAudioPortEnabled (param->handle, &bAudioPortEnableVerify);
    if(dsERR_NONE == ret) {
        if (bAudioPortEnableVerify != param->enabled) {
            printf("%s : %s Audio port status verification failed. param->enabled: %d bAudioPortEnableVerify:%d\n", 
                    __FUNCTION__, isEnabledAudioPortKey.c_str(), param->enabled, bAudioPortEnableVerify);
        }
        else {
            printf("%s : %s Audio port status verification passed. status %d\n", __FUNCTION__, isEnabledAudioPortKey.c_str(), param->enabled); 
        }
    }
    else {
        printf("%s : %s Audio port status:%s verification step: dsIsAudioPortEnabled call failed\n", 
               __FUNCTION__, isEnabledAudioPortKey.c_str(), param->enabled? "TRUE":"FALSE");
    }
 
    IARM_BUS_Unlock(lock);
    
    return result;
}

IARM_Result_t _dsGetEnablePersist(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    IARM_Result_t result = IARM_RESULT_INVALID_STATE;

    dsAudioPortEnabledParam_t *param = (dsAudioPortEnabledParam_t *)arg;
    //By default all the ports are enabled.
    bool enabled = true;

    std::string isEnabledAudioPortKey("audio.");
    isEnabledAudioPortKey.append (param->portName);
    isEnabledAudioPortKey.append (".isEnabled");
    std::string _AudioPortEnable("TRUE");
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
    try {
        _AudioPortEnable = device::HostPersistence::getInstance().getProperty(isEnabledAudioPortKey);
    }
    catch(...) {
        try {
            printf("Init: %s : %s port enable settings not found in persistence store. Try system default\n",__FUNCTION__, isEnabledAudioPortKey.c_str());
            _AudioPortEnable = device::HostPersistence::getInstance().getDefaultProperty(isEnabledAudioPortKey);
        }
        catch(...) {
            /*By default enable all the ports*/
            _AudioPortEnable = "TRUE";
        }
    }
    if ("FALSE" == _AudioPortEnable) { 
        printf("%s: persist dsEnableAudioPort value: _AudioPortEnable:%s:\n", __FUNCTION__, _AudioPortEnable.c_str());  
        enabled = false;
    }
    else {
        printf("%s: persist dsEnableAudioPort value: _AudioPortEnable:%s:\n", __FUNCTION__, _AudioPortEnable.c_str());  
        enabled = true;
    }

#endif //DS_AUDIO_SETTINGS_PERSISTENCE end
    
    param->enabled = enabled;
    result = IARM_RESULT_SUCCESS;
    printf("%s: persist dsEnableAudioPort value: %s for the port %s AudioPortEnable: %s result:%d \n", 
           __FUNCTION__, param->enabled? "TRUE":"FALSE", isEnabledAudioPortKey.c_str(), _AudioPortEnable.c_str(), result);

    IARM_BUS_Unlock(lock);

    return result;
}


IARM_Result_t _dsSetEnablePersist(void *arg)
{
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    dsError_t ret = dsERR_NONE;

    dsAudioPortEnabledParam_t *param = (dsAudioPortEnabledParam_t *)arg;
    result = IARM_RESULT_SUCCESS;

    std::string isEnabledAudioPortKey("audio.");
    isEnabledAudioPortKey.append (param->portName);
    isEnabledAudioPortKey.append (".isEnabled");
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
    printf("%s: persist dsEnableAudioPort value: %s for the port %s\n", __FUNCTION__, param->enabled? "TRUE":"FALSE", isEnabledAudioPortKey.c_str());
    device::HostPersistence::getInstance().persistHostProperty(isEnabledAudioPortKey.c_str(), param->enabled? ("TRUE"):("FALSE"));
#endif //DS_AUDIO_SETTINGS_PERSISTENCE end
 
    IARM_BUS_Unlock(lock);
    
    return result;
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


IARM_Result_t _dsGetAudioFormat(void *arg)
{
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetAudioFormat_t)(int handle, dsAudioFormat_t *audioFormat);
    static dsGetAudioFormat_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetAudioFormat_t) dlsym(dllib, "dsGetAudioFormat");
            if (func) {
                printf("dsGetAudioFormat_t(int, dsAudioFormat_t *) is defined and loaded\r\n");
            }
            else {
                printf("dsGetAudioFormat_t(int, dsAudioFormat_t *) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsAudioFormatParam_t *param = (dsAudioFormatParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        dsAudioFormat_t aFormat = dsAUDIO_FORMAT_NONE;
        param->audioFormat = dsAUDIO_FORMAT_NONE;

        if (func(param->handle, &aFormat) == dsERR_NONE)
        {
           param->audioFormat = aFormat;
           result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}


IARM_Result_t _dsGetEncoding(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);

    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    dsError_t ret = dsERR_NONE;

    dsAudioGetEncodingModeParam_t *param = (dsAudioGetEncodingModeParam_t *)arg;

    if (param != NULL)
    {

        dsAudioEncoding_t _encoding = dsAUDIO_ENC_NONE;
        ret = dsGetAudioEncoding(param->handle, &_encoding);

        if(ret == dsERR_NONE) {
            result = IARM_RESULT_SUCCESS;
        }
        param->encoding = _encoding;


     __TIMESTAMP();printf("param->encoding = %d\r\n",_encoding);
    
    }

    IARM_BUS_Unlock(lock);

    return result;
}

static dsAudioPortType_t _GetAudioPortType(int handle)
{
    int numPorts,i;
    int halhandle = 0;

    numPorts = dsUTL_DIM(kSupportedPortTypes);
    
    for(i=0; i< numPorts; i++)
    {
        if(dsGetAudioPort (kPorts[i].id.type, kPorts[i].id.index, &halhandle) == dsERR_NONE) {
            if (handle == halhandle)
            {
                return kPorts[i].id.type;
            }
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

    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
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
            result = IARM_RESULT_SUCCESS;
        }
    }else {
        param->ms11Enabled = false;
    }

    IARM_BUS_Unlock(lock);

    return result;
}

IARM_Result_t _dsIsAudioMS12Decode(void *arg)
{

    IARM_BUS_Unlock(lock);

#ifndef RDK_DSHAL_NAME
    #warning   "RDK_DSHAL_NAME is not defined"
    #define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);
    
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    typedef dsError_t  (*dsIsAudioMS12Decode_t)(int handle, bool *HasMS12Decode);
    static dsIsAudioMS12Decode_t func = NULL;
    if (func == NULL) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsIsAudioMS12Decode_t) dlsym(dllib, "dsIsAudioMS12Decode");
            if (func) {
                __TIMESTAMP();printf("dsIsAudioMS12Decode(int, bool*) is defined and loaded\r\n");
            }   
            else {
                __TIMESTAMP();printf("dsIsAudioMS12Decode(int, bool*) is not defined\r\n");
            }   
            dlclose(dllib);
        }   
        else {
            __TIMESTAMP();printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }   
    }   

    dsAudioGetMS12Param_t *param = (dsAudioGetMS12Param_t *)arg;
    if (func != NULL) {
        bool HasMS12Decode = false;
        dsError_t ret = func(param->handle, &HasMS12Decode);
        if (ret == dsERR_NONE) {
            param->ms12Enabled = HasMS12Decode;
            result = IARM_RESULT_SUCCESS;
        }
    }else {
        param->ms12Enabled = false;
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

    IARM_Result_t result = IARM_RESULT_SUCCESS;
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
            result = IARM_RESULT_INVALID_STATE;
        }

#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
        std::string _AudioDelay = std::to_string(param->audioDelayMs);
        dsAudioPortType_t _APortType = _GetAudioPortType(param->handle);
        switch(_APortType) {
            case dsAUDIOPORT_TYPE_SPDIF:
                printf("%s: port: %s , persist audio delay: %d\n",__func__,"SPDIF0", param->audioDelayMs);
                device::HostPersistence::getInstance().persistHostProperty("SPDIF0.audio.Delay",_AudioDelay);
                break;
            case dsAUDIOPORT_TYPE_HDMI:
                printf("%s: port: %s , persist audio delay: %d\n",__func__,"HDMI0", param->audioDelayMs);
                device::HostPersistence::getInstance().persistHostProperty("HDMI0.audio.Delay",_AudioDelay);
                break;
            case dsAUDIOPORT_TYPE_SPEAKER:
                printf("%s: port: %s , persist audio delay: %d\n",__func__,"SPEAKER0", param->audioDelayMs);
                device::HostPersistence::getInstance().persistHostProperty("SPEAKER0.audio.Delay",_AudioDelay);
                break;
            case dsAUDIOPORT_TYPE_HDMI_ARC:
                printf("%s: port: %s , persist audio delay: %d\n",__func__,"HDMI_ARC0", param->audioDelayMs);
                device::HostPersistence::getInstance().persistHostProperty("HDMI_ARC0.audio.Delay",_AudioDelay);
                break;
            default:
                printf("%s: port: UNKNOWN , persist audio delay: %d : NOT SET\n",__func__, param->audioDelayMs);
                break;
        }
#endif
    }
    else {
        result = IARM_RESULT_INVALID_STATE;
    }

    IARM_BUS_Unlock(lock);
    return result;

}

IARM_Result_t _dsGetAudioDelay(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
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
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}

IARM_Result_t _dsSetAudioDelayOffset(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    IARM_Result_t result = IARM_RESULT_SUCCESS;
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
            result = IARM_RESULT_INVALID_STATE;
        }
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
        std::string _AudioDelayOffset = std::to_string(param->audioDelayOffsetMs);
        dsAudioPortType_t _APortType = _GetAudioPortType(param->handle);
        switch(_APortType) {
            case dsAUDIOPORT_TYPE_SPDIF:
                printf("%s: port: %s , persist audio delay offset ms: %d\n",__func__,"SPDIF0", param->audioDelayOffsetMs);
                device::HostPersistence::getInstance().persistHostProperty("SPDIF0.audio.DelayOffset",_AudioDelayOffset);
                break;
            case dsAUDIOPORT_TYPE_HDMI:
                printf("%s: port: %s , persist audio delay offset ms: %d\n",__func__,"HDMI0", param->audioDelayOffsetMs);
                device::HostPersistence::getInstance().persistHostProperty("HDMI0.audio.DelayOffset",_AudioDelayOffset);
                break;
            case dsAUDIOPORT_TYPE_SPEAKER:
                printf("%s: port: %s , persist audio delay offset ms: %d\n",__func__,"SPEAKER0", param->audioDelayOffsetMs);
                device::HostPersistence::getInstance().persistHostProperty("SPEAKER0.audio.DelayOffset",_AudioDelayOffset);
                break;
	    case dsAUDIOPORT_TYPE_HDMI_ARC:
                printf("%s: port: %s , persist audio delay: %d\n",__func__,"HDMI_ARC0", param->audioDelayOffsetMs);
                device::HostPersistence::getInstance().persistHostProperty("HDMI_ARC0.audio.DelayOffset",_AudioDelayOffset);
                break;	
            default:
                break;
        }
#endif
    }
    else {
        result = IARM_RESULT_INVALID_STATE;
    }

    IARM_BUS_Unlock(lock);
    return result;

}

IARM_Result_t _dsGetAudioDelayOffset(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
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
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}

IARM_Result_t _dsSetAudioAtmosOutputMode(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
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
        if(func(param->handle, param->enable) == dsERR_NONE) {
            result = IARM_RESULT_SUCCESS;
        }
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
        printf("%s: persist atmos output mode: %s\n",__func__, param->enable ? "TRUE":"FALSE");
        device::HostPersistence::getInstance().persistHostProperty("audio.AtmosOutputMode",param->enable ? "TRUE":"FALSE");
#endif
    }

    IARM_BUS_Unlock(lock);
    return result;
}

IARM_Result_t _dsGetSinkDeviceAtmosCapability(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
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
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}

IARM_Result_t _dsSetAudioCompression(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsSetAudioCompression_t)(int handle, int compressionLevel);
    static dsSetAudioCompression_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetAudioCompression_t) dlsym(dllib, "dsSetAudioCompression");
            if (func) {
                printf("dsSetAudioCompression_t(int, int ) is defined and loaded\r\n");
            }
            else {
                printf("dsSetAudioCompression_t(int, int) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsAudioCompressionParam_t *param = (dsAudioCompressionParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        if (func(param->handle, param->compression) == dsERR_NONE)
        {
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
            std::string _Compression = std::to_string(param->compression);
            printf("%s: persist audio compression: %d\n",__func__, param->compression);
            device::HostPersistence::getInstance().persistHostProperty("audio.Compression",_Compression);
#endif
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}

IARM_Result_t _dsGetAudioCompression(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetAudioCompression_t)(int handle, int *compressionLevel);
    static dsGetAudioCompression_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetAudioCompression_t) dlsym(dllib, "dsGetAudioCompression");
            if (func) {
                printf("dsGetAudioCompression_t(int, int *) is defined and loaded\r\n");
            }
            else {
                printf("dsGetAudioCompression_t(int, int *) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsAudioCompressionParam_t *param = (dsAudioCompressionParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        int compression = 0;
        param->compression= 0;
        if (func(param->handle, &compression) == dsERR_NONE)
        {
           param->compression = compression;
           result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}

IARM_Result_t _dsSetDialogEnhancement(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);
    dsDialogEnhancementParam_t *param = (dsDialogEnhancementParam_t *)arg;
    if(param != NULL)
        result = _setDialogEnhancement(param->handle, param->enhancerLevel);
    IARM_BUS_Unlock(lock);
    return result;
}

static IARM_Result_t _setDialogEnhancement(int handle, int enhancerLevel)
{
    typedef dsError_t (*dsSetDialogEnhancement_t)(int handle, int enhancerLevel);
    static dsSetDialogEnhancement_t func = 0;
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;

    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetDialogEnhancement_t) dlsym(dllib, "dsSetDialogEnhancement");
            if (func) {
                printf("dsSetDialogEnhancement_t(int, int) is defined and loaded\r\n");
            }
            else {
                printf("dsSetDialogEnhancement_t(int, int ) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    std::string _Property = _dsGetCurrentProfileProperty("EnhancerLevel");
    if (func != 0)
    {
        if (func(handle, enhancerLevel) == dsERR_NONE)
        {
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
            std::string _EnhancerLevel = std::to_string(enhancerLevel);
            printf("%s: persist enhancer level: %d\n",__func__, enhancerLevel);
            device::HostPersistence::getInstance().persistHostProperty(_Property ,_EnhancerLevel);
#endif
            result = IARM_RESULT_SUCCESS;
        }
    }
    return result;
}


IARM_Result_t _dsGetDialogEnhancement(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetDialogEnhancement_t)(int handle, int *enhancerLevel);
    static dsGetDialogEnhancement_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetDialogEnhancement_t) dlsym(dllib, "dsGetDialogEnhancement");
            if (func) {
                printf("dsGetDialogEnhancement_t(int, int *) is defined and loaded\r\n");
            }
            else {
                printf("dsGetDialogEnhancement_t(int, int *) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsDialogEnhancementParam_t *param = (dsDialogEnhancementParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        int enhancerLevel = 0;
        param->enhancerLevel = 0;
        if (func(param->handle, &enhancerLevel) == dsERR_NONE)
        {
           param->enhancerLevel = enhancerLevel;
           result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}


IARM_Result_t _dsSetDolbyVolumeMode(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsSetDolbyVolumeMode_t)(int handle, bool enable);
    static dsSetDolbyVolumeMode_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetDolbyVolumeMode_t) dlsym(dllib, "dsSetDolbyVolumeMode");
            if (func) {
                printf("dsSetDolbyVolumeMode_t(int, bool) is defined and loaded\r\n");
            }
            else {
                printf("dsSetDolbyVolumeMode_t(int, bool) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsSetDolbyVolumeParam_t *param = (dsSetDolbyVolumeParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        if (func(param->handle, param->enable) == dsERR_NONE)
        {
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
            printf("%s: persist dolby volume mode: %s\n",__func__, param->enable ? "TRUE":"FALSE");
            device::HostPersistence::getInstance().persistHostProperty("audio.DolbyVolumeMode",param->enable ? "TRUE":"FALSE");
#endif
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}

IARM_Result_t _dsGetDolbyVolumeMode(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetDolbyVolumeMode_t)(int handle, bool *enable);
    static dsGetDolbyVolumeMode_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetDolbyVolumeMode_t) dlsym(dllib, "dsGetDolbyVolumeMode");
            if (func) {
                printf("dsGetDolbyVolumeMode_t(int, bool *) is defined and loaded\r\n");
            }
            else {
                printf("dsGetDolbyVolumeMode_t(int, bool *) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsSetDolbyVolumeParam_t *param = (dsSetDolbyVolumeParam_t *)arg;
    bool enable = false;

    if (func != 0 && param != NULL)
    {
	param->enable = false;
        if (func(param->handle, &enable) == dsERR_NONE)
        {
            param->enable = enable;
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}

IARM_Result_t _dsSetIntelligentEqualizerMode(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsSetIntelligentEqualizerMode_t)(int handle, int mode);
    static dsSetIntelligentEqualizerMode_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetIntelligentEqualizerMode_t) dlsym(dllib, "dsSetIntelligentEqualizerMode");
            if (func) {
                printf("dsSetIntelligentEqualizerMode_t(int, int) is defined and loaded\r\n");
            }
            else {
                printf("dsSetIntelligentEqualizerMode_t(int, int) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsIntelligentEqualizerModeParam_t *param = (dsIntelligentEqualizerModeParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        if (func(param->handle, param->mode) == dsERR_NONE)
        {
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
            std::string _IntelligentEQ = std::to_string(param->mode);
            printf("%s: persist intelligent equalizer value: %d\n",__func__, param->mode);
            device::HostPersistence::getInstance().persistHostProperty("audio.IntelligentEQ",_IntelligentEQ);
#endif
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}


IARM_Result_t _dsGetIntelligentEqualizerMode(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetIntelligentEqualizerMode_t)(int handle, int *mode);
    static dsGetIntelligentEqualizerMode_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetIntelligentEqualizerMode_t) dlsym(dllib, "dsGetIntelligentEqualizerMode");
            if (func) {
                printf("dsGetIntelligentEqualizerMode_t(int, int *) is defined and loaded\r\n");
            }
            else {
                printf("dsGetIntelligentEqualizerMode_t(int, int *) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsIntelligentEqualizerModeParam_t *param = (dsIntelligentEqualizerModeParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        int  mode = 0;
        param->mode = 0;
        if (func(param->handle, &mode) == dsERR_NONE)
        {
            param->mode = mode;
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}


IARM_Result_t _dsGetVolumeLeveller(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetVolumeLeveller_t)(int handle, dsVolumeLeveller_t *volLeveller);
    static dsGetVolumeLeveller_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetVolumeLeveller_t) dlsym(dllib, "dsGetVolumeLeveller");
            if (func) {
                printf("dsGetVolumeLeveller_t(int, dsVolumeLeveller_t *) is defined and loaded\r\n");
            }
            else {
                printf("dsGetVolumeLeveller_t(int, dsVolumeLeveller_t *) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsVolumeLevellerParam_t *param = (dsVolumeLevellerParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        dsVolumeLeveller_t volLeveller;
	volLeveller.mode = 0;
	volLeveller.level = 0;
	param->volLeveller.mode = 0;
        param->volLeveller.level = 0;
        if (func(param->handle, &volLeveller) == dsERR_NONE)
        {
	    param->volLeveller.mode = volLeveller.mode;
            param->volLeveller.level = volLeveller.level;
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}


IARM_Result_t _dsSetVolumeLeveller(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    dsVolumeLevellerParam_t *param = (dsVolumeLevellerParam_t *)arg;
    if( param != NULL )
       result = _setVolumeLeveller(param->handle, param->volLeveller.mode, param->volLeveller.level);
    IARM_BUS_Unlock(lock);
    return result;
}

static IARM_Result_t _setVolumeLeveller(int handle, int volLevellerMode, int volLevellerLevel)
{
    typedef dsError_t (*dsSetVolumeLeveller_t)(int handle, dsVolumeLeveller_t volLeveller);
    static dsSetVolumeLeveller_t func = 0;
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;

    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetVolumeLeveller_t) dlsym(dllib, "dsSetVolumeLeveller");
            if (func) {
                printf("dsSetVolumeLeveller_t(int, dsVolumeLeveller_t) is defined and loaded\r\n");
            }
            else {
                printf("dsSetVolumeLeveller_t(int, dsVolumeLeveller_t) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsVolumeLevellerParam_t param;

    param.handle = handle;
    param.volLeveller.mode = volLevellerMode;
    param.volLeveller.level = volLevellerLevel;

    if (func != 0 )
    {
        if (func(param.handle, param.volLeveller) == dsERR_NONE)
        {
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
            std::string _PropertyMode = _dsGetCurrentProfileProperty("VolumeLeveller.mode");
            std::string _Propertylevel = _dsGetCurrentProfileProperty("VolumeLeveller.level");
            std::string _mode = std::to_string(param.volLeveller.mode);
            printf("%s: persist volume leveller mode: %d\n",__func__, param.volLeveller.mode);
            device::HostPersistence::getInstance().persistHostProperty(_PropertyMode,_mode);

	    if((param.volLeveller.mode == 0) || (param.volLeveller.mode == 1)) {
                std::string _level = std::to_string(param.volLeveller.level);
                printf("%s: persist volume leveller value: %d\n",__func__, param.volLeveller.level);
                device::HostPersistence::getInstance().persistHostProperty(_Propertylevel,_level);
	    }
#endif
            result = IARM_RESULT_SUCCESS;
        }
    }

    return result;
}


IARM_Result_t _dsGetBassEnhancer(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetBassEnhancer_t)(int handle, int *boost);
    static dsGetBassEnhancer_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetBassEnhancer_t) dlsym(dllib, "dsGetBassEnhancer");
            if (func) {
                printf("dsGetBassEnhancer_t(int, int *) is defined and loaded\r\n");
            }
            else {
                printf("dsGetBassEnhancer_t(int, int *) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsBassEnhancerParam_t *param = (dsBassEnhancerParam_t *)arg;
    int boost = 0;
    if (func != 0 && param != NULL)
    {
        param->boost = 0;  //CID:155155 - Rverse_inull
        if (func(param->handle, &boost) == dsERR_NONE)
        {
            param->boost = boost;
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}


IARM_Result_t _dsSetBassEnhancer(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);
    dsBassEnhancerParam_t *param = (dsBassEnhancerParam_t *)arg;
    if( param != NULL )
       result = _setBassEnhancer(param->handle, param->boost);
    IARM_BUS_Unlock(lock);
    return result;
}

static IARM_Result_t _setBassEnhancer(int handle ,int boost)
{
    typedef dsError_t (*dsSetBassEnhancer_t)(int handle, int boost);
    static dsSetBassEnhancer_t func = 0;
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;

    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetBassEnhancer_t) dlsym(dllib, "dsSetBassEnhancer");
            if (func) {
                printf("dsSetBassEnhancer_t(int, int) is defined and loaded\r\n");
            }
            else {
                printf("dsSetBassEnhancer_t(int, int) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    if (func != 0)
    {
        if (func(handle, boost) == dsERR_NONE)
        {
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
            std::string _BassBoost = std::to_string(boost);
            printf("%s: persist boost value: %d\n",__func__, boost);
            device::HostPersistence::getInstance().persistHostProperty("audio.BassBoost", _BassBoost);
#endif
            result = IARM_RESULT_SUCCESS;
        }
    }

    return result;
}


IARM_Result_t _dsIsSurroundDecoderEnabled(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsIsSurroundDecoderEnabled_t)(int handle, bool *enabled);
    static dsIsSurroundDecoderEnabled_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsIsSurroundDecoderEnabled_t) dlsym(dllib, "dsIsSurroundDecoderEnabled");
            if (func) {
                printf("dsIsSurroundDecoderEnabled_t(int, bool *) is defined and loaded\r\n");
            }
            else {
                printf("dsIsSurroundDecoderEnabled_t(int, bool *) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsSurroundDecoderParam_t *param = (dsSurroundDecoderParam_t *)arg;
    bool enable = false;

    if (func != 0 && param != NULL)
    {
        param->enable = false;   //CID:155170 - Reverse_inull
        if (func(param->handle, &enable) == dsERR_NONE)
        {
            param->enable = enable;
            result = IARM_RESULT_SUCCESS;

        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}

IARM_Result_t _dsEnableSurroundDecoder(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsEnableSurroundDecoder_t)(int handle, bool enabled);
    static dsEnableSurroundDecoder_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsEnableSurroundDecoder_t) dlsym(dllib, "dsEnableSurroundDecoder");
            if (func) {
                printf("dsEnableSurroundDecoder_t(int, bool) is defined and loaded\r\n");
            }
            else {
                printf("dsEnableSurroundDecoder_t(int, bool) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsSurroundDecoderParam_t *param = (dsSurroundDecoderParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        if (func(param->handle, param->enable) == dsERR_NONE)
        {
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
            printf("%s: persist surround decoder value: %s\n",__func__, param->enable ? "TRUE":"FALSE");
            device::HostPersistence::getInstance().persistHostProperty("audio.SurroundDecoderEnabled",param->enable ? "TRUE":"FALSE");
#endif
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}


IARM_Result_t _dsGetDRCMode(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetDRCMode_t)(int handle, int *mode);
    static dsGetDRCMode_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetDRCMode_t) dlsym(dllib, "dsGetDRCMode");
            if (func) {
                printf("dsGetDRCMode_t(int, int *) is defined and loaded\r\n");
            }
            else {
                printf("dsGetDRCMode_t(int, int *) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsDRCModeParam_t *param = (dsDRCModeParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        int mode = 0;
        param->mode = 0;
        if (func(param->handle, &mode) == dsERR_NONE)
        {
            param->mode = mode;
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}


IARM_Result_t _dsSetDRCMode(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsSetDRCMode_t)(int handle, int mode);
    static dsSetDRCMode_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetDRCMode_t) dlsym(dllib, "dsSetDRCMode");
            if (func) {
                printf("dsSetDRCMode_t(int, int) is defined and loaded\r\n");
            }
            else {
                printf("dsSetDRCMode_t(int, int) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsDRCModeParam_t *param = (dsDRCModeParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        if (func(param->handle, param->mode) == dsERR_NONE)
        {
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
            printf("%s: persist DRC Mode value: %s\n",__func__, param->mode ? "RF":"Line");
            device::HostPersistence::getInstance().persistHostProperty("audio.DRCMode",param->mode ? "RF":"Line");
#endif
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}

IARM_Result_t _dsGetSurroundVirtualizer(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetSurroundVirtualizer_t)(int handle, dsSurroundVirtualizer_t *virtualizer);
    static dsGetSurroundVirtualizer_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetSurroundVirtualizer_t) dlsym(dllib, "dsGetSurroundVirtualizer");
            if (func) {
                printf("dsGetSurroundVirtualizer_t(int, dsSurroundVirtualizer_t *) is defined and loaded\r\n");
            }
            else {
                printf("dsGetSurroundVirtualizer_t(int, dsSurroundVirtualizer_t *) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsSurroundVirtualizerParam_t *param = (dsSurroundVirtualizerParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        dsSurroundVirtualizer_t virtualizer;
	virtualizer.mode = 0;
	virtualizer.boost = 0;	
        param->virtualizer.mode = 0;
	param->virtualizer.boost = 0;
        if (func(param->handle, &virtualizer) == dsERR_NONE)
        {
            param->virtualizer.mode = virtualizer.mode;
	    param->virtualizer.boost = virtualizer.boost;
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}


IARM_Result_t _dsSetSurroundVirtualizer(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    dsSurroundVirtualizerParam_t *param = (dsSurroundVirtualizerParam_t *)arg;
    if( param != NULL )
       result = _setSurroundVirtualizer(param->handle, param->virtualizer.mode, param->virtualizer.boost);
    IARM_BUS_Unlock(lock);
    return result;
}

static IARM_Result_t _setSurroundVirtualizer(int handle , int virtualizerMode , int virtualizerBoost)
{
    typedef dsError_t (*dsSetSurroundVirtualizer_t)(int handle, dsSurroundVirtualizer_t virtualizer);
    static dsSetSurroundVirtualizer_t func = 0;
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;

    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetSurroundVirtualizer_t) dlsym(dllib, "dsSetSurroundVirtualizer");
            if (func) {
                printf("dsSetSurroundVirtualizer_t(int, dsSurroundVirtualizer_t) is defined and loaded\r\n");
            }
            else {
                printf("dsSetSurroundVirtualizer_t(int, dsSurroundVirtualizer_t) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsSurroundVirtualizerParam_t param;
    param.handle = handle;
    param.virtualizer.mode = virtualizerMode;
    param.virtualizer.boost = virtualizerBoost;

    if (func != 0)
    {
        if (func(param.handle, param.virtualizer) == dsERR_NONE)
        {
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
            std::string _mode = std::to_string(param.virtualizer.mode);
            printf("%s: persist surround virtualizer mode: %d\n",__func__, param.virtualizer.mode);
            std::string _PropertyMode = _dsGetCurrentProfileProperty("SurroundVirtualizer.mode");
            std::string _Propertylevel = _dsGetCurrentProfileProperty("SurroundVirtualizer.boost"); 
            device::HostPersistence::getInstance().persistHostProperty(_PropertyMode,_mode);

            if(((param.virtualizer.mode) >= 0) && ((param.virtualizer.mode) <= 2)) {
                std::string _boost = std::to_string(param.virtualizer.boost);
                printf("%s: persist surround virtualizer boost value: %d\n",__func__, param.virtualizer.boost);
                device::HostPersistence::getInstance().persistHostProperty(_Propertylevel,_boost);
            }
#endif
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}


IARM_Result_t _dsGetMISteering(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetMISteering_t)(int handle, bool *enabled);
    static dsGetMISteering_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetMISteering_t) dlsym(dllib, "dsGetMISteering");
            if (func) {
                printf("dsGetMISteering_t(int, bool *) is defined and loaded\r\n");
            }
            else {
                printf("dsGetMISteering_t(int, bool *) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsMISteeringParam_t *param = (dsMISteeringParam_t *)arg;
    bool enable = false;
    if (func != 0 && param != NULL)
    {
        param->enable = false;  //CID:155153 - Reverse_inull
        if (func(param->handle, &enable) == dsERR_NONE)
        {
            param->enable = enable;
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}


IARM_Result_t _dsSetMISteering(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsSetMISteering_t)(int handle, bool enabled);
    static dsSetMISteering_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetMISteering_t) dlsym(dllib, "dsSetMISteering");
            if (func) {
                printf("dsSetMISteering_t(int, bool) is defined and loaded\r\n");
            }
            else {
                printf("dsSetMISteering_t(int, bool) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsMISteeringParam_t *param = (dsMISteeringParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        if (func(param->handle, param->enable) == dsERR_NONE)
        {
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
            printf("%s: persist MISteering value: %s\n", __func__, param->enable ? "Enabled":"Disabled");
            device::HostPersistence::getInstance().persistHostProperty("audio.MISteering",param->enable ? "Enabled":"Disabled");
#endif
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}


IARM_Result_t _dsSetGraphicEqualizerMode(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsSetGraphicEqualizerMode_t)(int handle, int mode);
    static dsSetGraphicEqualizerMode_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetGraphicEqualizerMode_t) dlsym(dllib, "dsSetGraphicEqualizerMode");
            if (func) {
                printf("dsSetGraphicEqualizerMode_t(int, int) is defined and loaded\r\n");
            }
            else {
                printf("dsSetGraphicEqualizerMode_t(int, int) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsGraphicEqualizerModeParam_t *param = (dsGraphicEqualizerModeParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        if (func(param->handle, param->mode) == dsERR_NONE)
        {
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
            std::string _GraphicEQ = std::to_string(param->mode);
            printf("%s: persist graphic equalizer value: %d\n",__func__, param->mode);
            device::HostPersistence::getInstance().persistHostProperty("audio.GraphicEQ",_GraphicEQ);
#endif
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}


IARM_Result_t _dsGetGraphicEqualizerMode(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetGraphicEqualizerMode_t)(int handle, int *mode);
    static dsGetGraphicEqualizerMode_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetGraphicEqualizerMode_t) dlsym(dllib, "dsGetGraphicEqualizerMode");
            if (func) {
                printf("dsGetGraphicEqualizerMode_t(int, int *) is defined and loaded\r\n");
            }
            else {
                printf("dsGetGraphicEqualizerMode_t(int, int *) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsGraphicEqualizerModeParam_t *param = (dsGraphicEqualizerModeParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        int  mode = 0;
        param->mode = 0;
        if (func(param->handle, &mode) == dsERR_NONE)
        {
            param->mode = mode;
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}


IARM_Result_t _dsGetMS12AudioProfileList(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetMS12AudioProfileList_t)(int handle, dsMS12AudioProfileList_t* profiles);
    static dsGetMS12AudioProfileList_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetMS12AudioProfileList_t) dlsym(dllib, "dsGetMS12AudioProfileList");
            if (func) {
                printf("dsGetMS12AudioProfileList_t(int, dsMS12AudioProfileList_t*) is defined and loaded\r\n");
            }
            else {
                printf("dsGetMS12AudioProfileList_t(int, dsMS12AudioProfileList_t*) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsMS12AudioProfileListParam_t *param = (dsMS12AudioProfileListParam_t *)arg;
    dsMS12AudioProfileList_t pList;
    dsError_t ret = dsERR_NONE;
    if (func != 0 && param != NULL)
    {
	ret = func(param->handle, &pList);
        if (ret == dsERR_NONE)
        {
	    printf("%s: Total number of supported profiles: %d\n",__FUNCTION__, pList.audioProfileCount);
	    printf("%s: Profile List: %s\n",__FUNCTION__, pList.audioProfileList);

	    param->profileList.audioProfileCount = pList.audioProfileCount;
	    strncpy(param->profileList.audioProfileList,pList.audioProfileList,MAX_PROFILE_LIST_BUFFER_LEN);
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}


IARM_Result_t _dsGetMS12AudioProfile(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    errno_t rc = -1;
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetMS12AudioProfile_t)(int handle, char* profile);
    static dsGetMS12AudioProfile_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetMS12AudioProfile_t) dlsym(dllib, "dsGetMS12AudioProfile");
            if (func) {
                printf("dsGetMS12AudioProfile_t(int, char* ) is defined and loaded\r\n");
            }
            else {
                printf("dsGetMS12AudioProfile_t(int, char*) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsMS12AudioProfileParam_t *param = (dsMS12AudioProfileParam_t *)arg;
    char m_profile[MAX_PROFILE_STRING_LEN] = {0};
    if (func != 0 && param != NULL)
    {
        if (func(param->handle, m_profile) == dsERR_NONE)
        {
            rc = strcpy_s(param->profile,sizeof(param->profile), m_profile);
	    if(rc!=EOK)
	    {
		    ERR_CHK(rc);
	    }
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}


IARM_Result_t _dsSetMS12AudioProfile(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsSetMS12AudioProfile_t)(int handle, const char* profile);
    static dsSetMS12AudioProfile_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetMS12AudioProfile_t) dlsym(dllib, "dsSetMS12AudioProfile");
            if (func) {
                printf("dsSetMS12AudioProfile_t(int, const char*) is defined and loaded\r\n");
            }
            else {
                printf("dsSetMS12AudioProfile_t(int, const char*) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsMS12AudioProfileParam_t *param = (dsMS12AudioProfileParam_t*)arg;

    if (func != 0 && param != NULL)
    {
        if (func(param->handle, param->profile) == dsERR_NONE)
        {
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
            printf("%s: persist MS12 Audio Profile selection : %s\n", __func__, param->profile);
            device::HostPersistence::getInstance().persistHostProperty("audio.MS12Profile",param->profile);
#endif
            result = IARM_RESULT_SUCCESS;
        }
    }
    
    _dsMS12ProfileSettingOverride(param->handle);
   
    IARM_BUS_Unlock(lock);
    return result;
}

IARM_Result_t _dsSetAssociatedAudioMixing(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsSetAssociatedAudioMixing_t)(int handle, bool mixing);
    static dsSetAssociatedAudioMixing_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetAssociatedAudioMixing_t) dlsym(dllib, "dsSetAssociatedAudioMixing");
            if (func) {
                printf("dsSetAssociatedAudioMixing_t(int, bool) is defined and loaded\r\n");
            }
            else {
                printf("dsSetMS12AudioProfile_t(int, bool) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsAssociatedAudioMixingParam_t *param = (dsAssociatedAudioMixingParam_t*)arg;

    if (func != 0 && param != NULL)
    {
        if (func(param->handle, param->mixing) == dsERR_NONE)
        {
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
            printf("%s: persist Associated Audio Mixing status : %s\n", __func__, param->mixing ? "Enabled":"Disabled");
            device::HostPersistence::getInstance().persistHostProperty("audio.AssociatedAudioMixing",param->mixing ? "Enabled":"Disabled");
#endif
            IARM_Bus_DSMgr_EventData_t associated_audio_mixing_event_data;
            printf("%s: Associated Audio Mixing status changed :%d \r\n", __FUNCTION__, param->mixing);
            associated_audio_mixing_event_data.data.AssociatedAudioMixingInfo.mixing = param->mixing;

            IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,
                                   (IARM_EventId_t)IARM_BUS_DSMGR_EVENT_AUDIO_ASSOCIATED_AUDIO_MIXING_CHANGED,
                                   (void *)&associated_audio_mixing_event_data,
                                   sizeof(associated_audio_mixing_event_data));

            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}

IARM_Result_t _dsGetAssociatedAudioMixing(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetAssociatedAudioMixing_t)(int handle, bool *mixing);
    static dsGetAssociatedAudioMixing_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetAssociatedAudioMixing_t) dlsym(dllib, "dsGetAssociatedAudioMixing");
            if (func) {
                printf("dsGetAssociatedAudioMixing_t(int, bool *) is defined and loaded\r\n");
            }
            else {
                printf("dsGetAssociatedAudioMixing_t(int, bool *) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsAssociatedAudioMixingParam_t *param = (dsAssociatedAudioMixingParam_t *)arg;
    bool mixing = false;
    if (func != 0 && param != NULL)
    {
        param->mixing = false;
        if (func(param->handle, &mixing) == dsERR_NONE)
        {
            param->mixing = mixing;
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}

IARM_Result_t _dsSetFaderControl(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsSetFaderControl_t)(int handle, int mixerbalance);
    static dsSetFaderControl_t func = 0;

    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetFaderControl_t) dlsym(dllib, "dsSetFaderControl");
            if (func) {
                printf("dsSetFaderControl_t(int, int) is defined and loaded\r\n");
            }
            else {
                printf("dsSetFaderControl_t(int, int) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsFaderControlParam_t *param = (dsFaderControlParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        if (func(param->handle, param->mixerbalance) == dsERR_NONE)
        {
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
            std::string _mixerbalance = std::to_string(param->mixerbalance);
            printf("%s: persist fader control level: %d\n",__func__, param->mixerbalance);
            device::HostPersistence::getInstance().persistHostProperty("audio.FaderControl",_mixerbalance);
#endif
            IARM_Bus_DSMgr_EventData_t fader_control_event_data;
            printf("%s: Fader Control changed :%d \r\n", __FUNCTION__, param->mixerbalance);
            fader_control_event_data.data.FaderControlInfo.mixerbalance = param->mixerbalance;

            IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,
                                   (IARM_EventId_t)IARM_BUS_DSMGR_EVENT_AUDIO_FADER_CONTROL_CHANGED,
                                   (void *)&fader_control_event_data,
                                   sizeof(fader_control_event_data));

            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}




IARM_Result_t _dsGetFaderControl(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetFaderControl_t)(int handle, int *mixerbalance);
    static dsGetFaderControl_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetFaderControl_t) dlsym(dllib, "dsGetFaderControl");
            if (func) {
                printf("dsGetFaderControl_t(int, int *) is defined and loaded\r\n");
            }
            else {
                printf("dsGetFaderControl_t(int, int *) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsFaderControlParam_t *param = (dsFaderControlParam_t *)arg;
    int mixerbalance = 0;
    if (func != 0 && param != NULL)
    {
        param->mixerbalance = 0;
        if (func(param->handle, &mixerbalance) == dsERR_NONE)
        {
            param->mixerbalance = mixerbalance;
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}


IARM_Result_t _dsSetPrimaryLanguage(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsSetPrimaryLanguage_t)(int handle, const char* pLang);
    static dsSetPrimaryLanguage_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetPrimaryLanguage_t) dlsym(dllib, "dsSetPrimaryLanguage");
            if (func) {
                printf("dsSetPrimaryLanguage_t(int, const char*) is defined and loaded\r\n");
            }
            else {
                printf("dsSetPrimaryLanguage_t(int, const char*) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsPrimaryLanguageParam_t *param = (dsPrimaryLanguageParam_t*)arg;

    if (func != 0 && param != NULL)
    {
        if (func(param->handle, param->primaryLanguage) == dsERR_NONE)
        {
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
            printf("%s: persist Primary Language : %s\n", __func__, param->primaryLanguage);
            device::HostPersistence::getInstance().persistHostProperty("audio.PrimaryLanguage",param->primaryLanguage);
#endif
            IARM_Bus_DSMgr_EventData_t primary_language_event_data;
            printf("%s: Primary Language changed :%s \r\n", __FUNCTION__, param->primaryLanguage);
            strncpy(primary_language_event_data.data.AudioLanguageInfo.audioLanguage, param->primaryLanguage, MAX_LANGUAGE_LEN);
 
            IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,
                                   (IARM_EventId_t)IARM_BUS_DSMGR_EVENT_AUDIO_PRIMARY_LANGUAGE_CHANGED,
                                   (void *)&primary_language_event_data,
                                   sizeof(primary_language_event_data));

            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}


IARM_Result_t _dsGetPrimaryLanguage(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    errno_t rc = -1;
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetPrimaryLanguage_t)(int handle, char* pLang);
    static dsGetPrimaryLanguage_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetPrimaryLanguage_t) dlsym(dllib, "dsGetPrimaryLanguage");
            if (func) {
                printf("dsGetPrimaryLanguage_t(int, char* ) is defined and loaded\r\n");
            }
            else {
                printf("dsGetPrimaryLanguage_t(int, char*) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsPrimaryLanguageParam_t *param = (dsPrimaryLanguageParam_t *)arg;
    char primaryLanguage[MAX_LANGUAGE_LEN] = {0};
    if (func != 0 && param != NULL)
    {
        if (func(param->handle, primaryLanguage) == dsERR_NONE)
        {
            rc = strcpy_s(param->primaryLanguage,sizeof(param->primaryLanguage), primaryLanguage);
            if(rc!=EOK)
            {
                    ERR_CHK(rc);
            }
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}


IARM_Result_t _dsSetSecondaryLanguage(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsSetSecondaryLanguage_t)(int handle, const char* sLang);
    static dsSetSecondaryLanguage_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetSecondaryLanguage_t) dlsym(dllib, "dsSetSecondaryLanguage");
            if (func) {
                printf("dsSetSecondaryLanguage_t(int, const char*) is defined and loaded\r\n");
            }
            else {
                printf("dsSetSecondaryLanguage_t(int, const char*) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsSecondaryLanguageParam_t *param = (dsSecondaryLanguageParam_t*)arg;

    if (func != 0 && param != NULL)
    {
        if (func(param->handle, param->secondaryLanguage) == dsERR_NONE)
        {
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
            printf("%s: persist Secondary Language : %s\n", __func__, param->secondaryLanguage);
            device::HostPersistence::getInstance().persistHostProperty("audio.SecondaryLanguage",param->secondaryLanguage);
#endif
            IARM_Bus_DSMgr_EventData_t secondary_language_event_data;
            printf("%s: Secondary Language changed :%s \r\n", __FUNCTION__, param->secondaryLanguage);
            strncpy(secondary_language_event_data.data.AudioLanguageInfo.audioLanguage, param->secondaryLanguage, MAX_LANGUAGE_LEN);

            IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,
                                   (IARM_EventId_t)IARM_BUS_DSMGR_EVENT_AUDIO_SECONDARY_LANGUAGE_CHANGED,
                                   (void *)&secondary_language_event_data,
                                   sizeof(secondary_language_event_data));

            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}


IARM_Result_t _dsGetSecondaryLanguage(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    errno_t rc = -1;
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetSecondaryLanguage_t)(int handle, char* sLang);
    static dsGetSecondaryLanguage_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetSecondaryLanguage_t) dlsym(dllib, "dsGetSecondaryLanguage");
            if (func) {
                printf("dsGetSecondaryLanguage_t(int, char* ) is defined and loaded\r\n");
            }
            else {
                printf("dsGetSecondaryLanguage_t(int, char*) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsSecondaryLanguageParam_t *param = (dsSecondaryLanguageParam_t *)arg;
    char secondaryLanguage[MAX_LANGUAGE_LEN] = {0};
    if (func != 0 && param != NULL)
    {
        if (func(param->handle, secondaryLanguage) == dsERR_NONE)
        {
            rc = strcpy_s(param->secondaryLanguage,sizeof(param->secondaryLanguage), secondaryLanguage);
            if(rc!=EOK)
            {
                    ERR_CHK(rc);
            }
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}

IARM_Result_t _dsSetMS12SetttingsOverride(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);
    dsMS12SetttingsOverrideParam_t *param = (dsMS12SetttingsOverrideParam_t*)arg;
    std::string _hostProperty;
    std::string _value;
    std::string _AProfile("Off");
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
    if(!(_dsMs12ProfileSupported(param->handle,param->profileName))) {
         printf("%s: Unknow MS12 profile %s \n",__func__, param->profileName);
         IARM_BUS_Unlock(lock);
         return result;
    }
    try {
          _AProfile = device::HostPersistence::getInstance().getProperty("audio.MS12Profile");
    }
    catch(...) {
         try {
             printf("audio.MS12Profile not found in persistence store. Try system default\n");
            _AProfile = device::HostPersistence::getInstance().getDefaultProperty("audio.MS12Profile");
        }
        catch(...) {
            _AProfile = "Off";
        }
    }

    if(strcmp(param->profileName,_AProfile.c_str()) == 0) {
         if(strcmp(param->profileSettingsName, "DialogEnhance") == 0) {
            if(strcmp(param->profileState, "ADD") == 0) {
               result = _setDialogEnhancement(param->handle,atoi(param->profileSettingValue));
            }
            else if(strcmp(param->profileState, "REMOVE") == 0) {
               result =  _resetDialogEnhancerLevel(param->handle);
            }
         }
         else if(strcmp(param->profileSettingsName, "VolumeLevellerMode") == 0) {
            std::string _PropertyMode = _dsGetCurrentProfileProperty("VolumeLeveller.mode");
            if((atoi(param->profileSettingValue) == 0) || (atoi(param->profileSettingValue) == 1)) {
                device::HostPersistence::getInstance().persistHostProperty(_PropertyMode,param->profileSettingValue);
                result = IARM_RESULT_SUCCESS;
            }else {
                printf("%s: Unknow MS12 property value %s %s \n",__func__, param->profileSettingsName,param->profileSettingValue);
                result = IARM_RESULT_INVALID_STATE;
            }
         }
         else if(strcmp(param->profileSettingsName, "VolumeLevellerLevel") == 0) {
            if(strcmp(param->profileState, "ADD") == 0) {
               std::string _volLevellerMode("0");
               std::string _PropertyMode = _dsGetCurrentProfileProperty("VolumeLeveller.mode");
               _volLevellerMode = device::HostPersistence::getInstance().getProperty(_PropertyMode);
               result = _setVolumeLeveller(param->handle,atoi(_volLevellerMode.c_str()),atoi(param->profileSettingValue));
            }
            else if(strcmp(param->profileState, "REMOVE") == 0) {
               result = _resetVolumeLeveller(param->handle);
            }
         }
         else if(strcmp(param->profileSettingsName, "BassEnhancer") == 0) {
            if(strcmp(param->profileState, "ADD") == 0) {
               result = _setBassEnhancer(param->handle,atoi(param->profileSettingValue));
            }
            else if(strcmp(param->profileState, "REMOVE") == 0) {
               result = _resetBassEnhancer(param->handle);
            }
         }
         else if(strcmp(param->profileSettingsName, "SurroundVirtualizerMode") == 0) {
             std::string _PropertyMode = _dsGetCurrentProfileProperty("SurroundVirtualizer.mode");
             if((atoi(param->profileSettingValue) >= 0) && (atoi(param->profileSettingValue) <= 2)) {
                device::HostPersistence::getInstance().persistHostProperty(_PropertyMode,param->profileSettingValue);
                result = IARM_RESULT_SUCCESS;
             }
             else {
                printf("%s: Unknow MS12 property value %s %s \n",__func__, param->profileSettingsName,param->profileSettingValue);
                result = IARM_RESULT_INVALID_STATE;
             }
         }
         else if(strcmp(param->profileSettingsName, "SurroundVirtualizerLevel") == 0) {
            if(strcmp(param->profileState, "ADD") == 0) {
               std::string _SVMode("0");
               std::string _PropertyMode = _dsGetCurrentProfileProperty("SurroundVirtualizer.mode");
               _SVMode = device::HostPersistence::getInstance().getProperty(_PropertyMode);
               result = _setSurroundVirtualizer(param->handle,atoi(_SVMode.c_str()),atoi(param->profileSettingValue));
            }
            else if(strcmp(param->profileState, "REMOVE") == 0) {
               result = _resetSurroundVirtualizer(param->handle);
            }
         }
         else {
           printf("%s: Unknow MS12 property %s \n",__func__, param->profileSettingsName);
           result = IARM_RESULT_INVALID_STATE;
         }
         if(result != IARM_RESULT_SUCCESS)
         {
            IARM_BUS_Unlock(lock);
            return result;
         }
    }
    else {
       if(strcmp(param->profileSettingsName, "DialogEnhance") == 0) {
          _hostProperty = _dsGenerateProfileProperty(param->profileName,"EnhancerLevel");
       }
       else if(strcmp(param->profileSettingsName, "VolumeLevellerMode") == 0) {
          _hostProperty = _dsGenerateProfileProperty(param->profileName,"VolumeLeveller.mode");
       }
       else if(strcmp(param->profileSettingsName, "VolumeLevellerLevel") == 0) {
          _hostProperty = _dsGenerateProfileProperty(param->profileName,"VolumeLeveller.level");
       }
       else if(strcmp(param->profileSettingsName, "BassEnhancer") == 0) {
          _hostProperty = "audio.BassBoost";
       }
       else if(strcmp(param->profileSettingsName, "SurroundVirtualizerMode") == 0) {
          _hostProperty = _dsGenerateProfileProperty(param->profileName,"SurroundVirtualizer.mode");
       }
       else if(strcmp(param->profileSettingsName, "SurroundVirtualizerLevel") == 0) {
          _hostProperty = _dsGenerateProfileProperty(param->profileName,"SurroundVirtualizer.boost");
       }
       else {
          printf("%s: Unknow MS12 property %s \n",__func__, param->profileSettingsName);
          IARM_BUS_Unlock(lock);
          return result;
       }
       if(strcmp(param->profileState, "ADD") == 0) {
          printf("%s: Profile %s property %s persist value: %s\n",__func__,param->profileName, param->profileSettingsName , param->profileSettingValue);
          device::HostPersistence::getInstance().persistHostProperty(_hostProperty ,param->profileSettingValue);
       }
       else if(strcmp(param->profileState, "REMOVE") == 0) {
          try {
               _value = device::HostPersistence::getInstance().getDefaultProperty(_hostProperty);
          }
          catch(...) {
              _value = "0";
          }
          printf("%s: Profile %s property %s persist value: %s\n",__func__,param->profileName, param->profileSettingsName , _value.c_str());
          device::HostPersistence::getInstance().persistHostProperty(_hostProperty,_value);
       }
       else {
          printf("%s: Unknow State %s \n",__func__, param->profileState);
          IARM_BUS_Unlock(lock);
          return result;
       }
    }
    result = IARM_RESULT_SUCCESS;
#endif
    IARM_BUS_Unlock(lock);
    return result;
}

IARM_Result_t _dsGetSupportedARCTypes(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsGetSupportedARCTypes_t)(int handle, int *types);
    static dsGetSupportedARCTypes_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetSupportedARCTypes_t) dlsym(dllib, "dsGetSupportedARCTypes");
            if (func) {
                printf("dsGetSupportedARCTypes_t(int, int*) is defined and loaded\r\n");
            }
            else {
                printf("dsGetSupportedARCTypes_t(int, int*) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsGetSupportedARCTypesParam_t *param = (dsGetSupportedARCTypesParam_t *)arg;
    int types = dsAUDIOARCSUPPORT_NONE;

    if (func != 0 && param != NULL)
    {
        param->types = dsAUDIOARCSUPPORT_NONE;   //CID:163840 - Reverse_inull
        if (func(param->handle, &types) == dsERR_NONE)
        {
            param->types = types;
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}


IARM_Result_t _dsAudioSetSAD(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    typedef dsError_t (*dsAudioSetSAD_t)(int handle, dsAudioSADList_t sad_list);
    static dsAudioSetSAD_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsAudioSetSAD_t) dlsym(dllib, "dsAudioSetSAD");
            if (func) {
                printf("dsAudioSetSAD_t(int, dsAudioSADList_t) is defined and loaded\r\n");
            }
            else {
                printf("dsAudioSetSAD_t(int, dsAudioSADList_t) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsAudioSetSADParam_t *param = (dsAudioSetSADParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        if (func(param->handle, param->list) == dsERR_NONE)
        {
            result = IARM_RESULT_SUCCESS;
        }
    }

    IARM_BUS_Unlock(lock);
    return result;
}

IARM_Result_t _dsAudioEnableARC(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    IARM_BUS_Lock(lock);

    dsError_t ret = dsERR_GENERAL;
    std::string _isEnabledAudoARCPortKey("audio.hdmiArc0.isEnabled");
    std::string _audoARCPortTypeKey("audio.hdmiArc0.type");
    //Default is eARC
    std::string _audoARCPortiCapVal("eARC");


    typedef dsError_t (*dsAudioEnableARC_t)(int handle, dsAudioARCStatus_t arcStatus);
    static dsAudioEnableARC_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsAudioEnableARC_t) dlsym(dllib, "dsAudioEnableARC");
            if (func) {
                printf("dsAudioEnableARC_t(int, dsAudioARCStatus_t) is defined and loaded\r\n");
            }
            else {
                printf("dsAudioEnableARC_t(int, dsAudioARCStatus_t) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsAudioEnableARCParam_t *param = (dsAudioEnableARCParam_t *)arg;

    if (func != 0 && param != NULL)
    {
        if (func(param->handle, param->arcStatus) == dsERR_NONE)
        {
            result = IARM_RESULT_SUCCESS;
        }



        /*Ensure settings is enabled properly in HAL*/
        ret = dsERR_NONE;
        bool bAudioPortEnableVerify = false;
        ret = (dsError_t) dsIsAudioPortEnabled (param->handle, &bAudioPortEnableVerify);
        if(dsERR_NONE == ret) {
            if (bAudioPortEnableVerify != param->arcStatus.status) {
                printf("Init: %s : %s Audio port status:%s verification failed. bAudioPortEnable: %d bAudioPortEnableVerify:%d\n",
                        __FUNCTION__, _audoARCPortTypeKey.c_str(), _isEnabledAudoARCPortKey.c_str(), param->arcStatus.status, bAudioPortEnableVerify);
            }
            else {
                printf("%s : %s Audio port status verification passed. status %d\n", 
                       __FUNCTION__, _isEnabledAudoARCPortKey.c_str(), param->arcStatus.status); 
            }
        }
        else {
            printf("Init: %s : %s Audio port status:%s verification step: dsIsAudioPortEnabled call failed\n", 
                    __FUNCTION__, _audoARCPortTypeKey.c_str(), _isEnabledAudoARCPortKey.c_str());
        }

    }

    IARM_BUS_Unlock(lock);
    return result;
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
        else
        {
            __TIMESTAMP();printf("LE: %s Current enable status is same as requested:%d \r\n",__FUNCTION__,param->enable);
            result = IARM_RESULT_SUCCESS;
        }
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

    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
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
            if(param->result == dsERR_NONE) {
                result = IARM_RESULT_SUCCESS;
            }
        }
    }


    IARM_BUS_Unlock(lock);

    return result;

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
	else if (_APortType == dsAUDIOPORT_TYPE_HDMI_ARC){
	    _AudioModeSettings = device::HostPersistence::getInstance().getProperty("HDMI_ARC0.AudioMode",_AudioModeSettings);
	    __TIMESTAMP();printf("The HDMI_ARC Audio Mode Setting From Persistent is %s \r\n",_AudioModeSettings.c_str());
	}

        if (_AudioModeSettings.compare("SURROUND") == 0)
        {
            param->mode = dsAUDIO_STEREO_SURROUND;
        }
        else if (_AudioModeSettings.compare("PASSTHRU") == 0)
        {
            param->mode = dsAUDIO_STEREO_PASSTHRU;
        }
        else if (_AudioModeSettings.compare("DOLBYDIGITAL") == 0)
        {
            param->mode = dsAUDIO_STEREO_DD;
        }
        else if (_AudioModeSettings.compare("DOLBYDIGITALPLUS") == 0)
        {
            param->mode = dsAUDIO_STEREO_DDPLUS;
        }
        else
        {
            param->mode = dsAUDIO_STEREO_STEREO;
        } 
    }
}

IARM_Result_t _dsGetAudioCapabilities(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);
    
    typedef dsError_t (*dsGetAudioCapabilitiesFunc_t)(int handle, int *capabilities);
    static dsGetAudioCapabilitiesFunc_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetAudioCapabilitiesFunc_t)dlsym(dllib, "dsGetAudioCapabilities");
            if (func) {
                printf("dsGetAudioCapabilities() is defined and loaded\r\n");
            }
            else {
                printf("dsGetAudioCapabilities() is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }
    dsGetAudioCapabilitiesParam_t *param = (dsGetAudioCapabilitiesParam_t *)arg;
    if(0 != func) {
        param->result = func(param->handle, &param->capabilities);
    }
    else {
        param->capabilities = dsAUDIOSUPPORT_NONE;
    }

    IARM_BUS_Unlock(lock);
    return IARM_RESULT_SUCCESS;
}


IARM_Result_t _dsGetMS12Capabilities(void *arg)
{
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);
    
    typedef dsError_t (*dsGetMS12CapabilitiesFunc_t)(int handle, int *capabilities);
    static dsGetMS12CapabilitiesFunc_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetMS12CapabilitiesFunc_t)dlsym(dllib, "dsGetMS12Capabilities");
            if (func) {
                printf("dsGetMS12Capabilities() is defined and loaded\r\n");
            }
            else {
                printf("dsGetMS12Capabilities() is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }
    dsGetMS12CapabilitiesParam_t *param = (dsGetMS12CapabilitiesParam_t *)arg;
    if(0 != func) {
        param->result = func(param->handle, &param->capabilities);
    }
    else {
        param->capabilities = dsMS12SUPPORT_NONE;
    }

    IARM_BUS_Unlock(lock);
    return IARM_RESULT_SUCCESS;
}

void _dsAudioOutPortConnectCB(dsAudioPortType_t portType, unsigned int uiPortNo, bool isPortConnected)
{
    IARM_Bus_DSMgr_EventData_t audio_out_hpd_eventData;
    printf("%s: AudioOutPort type:%d portNo:%d Hotplug happened\r\n", 
            __FUNCTION__, portType, uiPortNo);
    audio_out_hpd_eventData.data.audio_out_connect.portType = portType;
    audio_out_hpd_eventData.data.audio_out_connect.uiPortNo = uiPortNo;
    audio_out_hpd_eventData.data.audio_out_connect.isPortConnected = isPortConnected;
        
    IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,
                           (IARM_EventId_t)IARM_BUS_DSMGR_EVENT_AUDIO_OUT_HOTPLUG,
                           (void *)&audio_out_hpd_eventData, 
                           sizeof(audio_out_hpd_eventData));
    printf ("%s portType%d uiPortNo:%d isPortConnected:%d", 
            __FUNCTION__, portType, uiPortNo, isPortConnected);           
}

static dsError_t _dsAudioOutRegisterConnectCB (dsAudioOutPortConnectCB_t cbFun) {
    dsError_t eRet = dsERR_GENERAL; 
    printf("%s: %d - Inside \n", __FUNCTION__, __LINE__);

    typedef dsError_t (*dsAudioOutRegisterConnectCB_t)(dsAudioOutPortConnectCB_t cbFunArg);
    static dsAudioOutRegisterConnectCB_t dsAudioOutRegisterConnectCBFun = 0;
    if (dsAudioOutRegisterConnectCBFun == 0) {
        printf("%s: %d - dlerror: %s\n", __FUNCTION__, __LINE__, dlerror());
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            dsAudioOutRegisterConnectCBFun = (dsAudioOutRegisterConnectCB_t) dlsym(dllib, "dsAudioOutRegisterConnectCB");
            if(dsAudioOutRegisterConnectCBFun == 0) {
                printf("%s: dsAudioOutRegisterConnectCB (int) is not defined %s\r\n", __FUNCTION__, dlerror());
                eRet = dsERR_GENERAL;
            }
            else {
                printf("%s: dsAudioOutRegisterConnectCB is loaded\r\n", __FUNCTION__);
            }
            dlclose(dllib);
        }
        else {
            printf("%s: Opening RDK_DSHAL_NAME [%s] failed %s\r\n", 
                   __FUNCTION__, RDK_DSHAL_NAME, dlerror());
            eRet = dsERR_GENERAL;
        }
    }
    if (0 != dsAudioOutRegisterConnectCBFun) { 
        eRet = dsAudioOutRegisterConnectCBFun (cbFun);
        printf("%s: dsAudioOutRegisterConnectCBFun registered\r\n", __FUNCTION__);
    }
    else {
        printf("%s: dsAudioOutRegisterConnectCBFun NULL\r\n", __FUNCTION__);
    }
    return eRet;
}

IARM_Result_t _dsAudioOutIsConnected (void *arg) {
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);

    IARM_Result_t eIarmRet = IARM_RESULT_INVALID_PARAM;
    dsAudioOutIsConnectedParam_t* param = (dsAudioOutIsConnectedParam_t*) arg; 
    dsError_t eRet = dsERR_GENERAL; 
    //By default all audio ports are connected
    bool isConnected = true;
    param->isCon = true;

    printf("%s: %d - Inside \n", __FUNCTION__, __LINE__);

    typedef dsError_t (*dsAudioOutIsConnected_t)(int handleArg, bool* pisConArg);
    static dsAudioOutIsConnected_t dsAudioOutIsConFunc = 0;
    if (dsAudioOutIsConFunc == 0) {
        printf("%s: %d -  dlerror:%s\n", __FUNCTION__, __LINE__, dlerror());
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            dsAudioOutIsConFunc = (dsAudioOutIsConnected_t) dlsym(dllib, "dsAudioOutIsConnected");
            if(dsAudioOutIsConFunc == 0) {
                printf("%s: dsAudioOutIsConnected is not defined %s\r\n", __FUNCTION__, dlerror());
                eRet = dsERR_GENERAL;
            }
            else {
                printf("%s: dsAudioOutIsConnected is loaded\r\n", __FUNCTION__);
            }
            dlclose(dllib);
        }
        else {
            printf("%s: Opening RDK_DSHAL_NAME [%s] failed %s\r\n", 
                   __FUNCTION__, RDK_DSHAL_NAME, dlerror());
            eRet = dsERR_GENERAL;
        }
    }
    if (0 != dsAudioOutIsConFunc) { 
        eRet = dsAudioOutIsConFunc (param->handle, &isConnected);
        printf("%s: pisCon:%d eRet:%04x\r\n", 
               __FUNCTION__, isConnected, eRet);
    }
    else {
        printf("%s: dsAudioOutIsConFunc NULL\n", __FUNCTION__);
    }
    
    param->result = eRet;
    if (dsERR_NONE == eRet) {
        param->isCon = isConnected;
        eIarmRet = IARM_RESULT_SUCCESS;
    }


    IARM_BUS_Unlock(lock);
    return eIarmRet;
}

void _dsAudioFormatUpdateCB(dsAudioFormat_t audioFormat)
{
    IARM_Bus_DSMgr_EventData_t audio_format_event_data;
    printf("%s: AudioOutPort format:%d \r\n", __FUNCTION__, audioFormat);
    audio_format_event_data.data.AudioFormatInfo.audioFormat = audioFormat;

    IARM_Bus_BroadcastEvent(IARM_BUS_DSMGR_NAME,
                           (IARM_EventId_t)IARM_BUS_DSMGR_EVENT_AUDIO_FORMAT_UPDATE,
                           (void *)&audio_format_event_data,
                           sizeof(audio_format_event_data));
}

static dsError_t _dsAudioFormatUpdateRegisterCB (dsAudioFormatUpdateCB_t cbFun) {
    dsError_t eRet = dsERR_GENERAL;
    printf("%s: %d - Inside \n", __FUNCTION__, __LINE__);

    typedef dsError_t (*dsAudioFormatUpdateRegisterCB_t)(dsAudioFormatUpdateCB_t cbFunArg);
    static dsAudioFormatUpdateRegisterCB_t dsAudioFormatUpdateRegisterCBFun = 0;
    if (dsAudioFormatUpdateRegisterCBFun == 0) {
        printf("%s: %d - dlerror: %s\n", __FUNCTION__, __LINE__, dlerror());
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            dsAudioFormatUpdateRegisterCBFun = (dsAudioFormatUpdateRegisterCB_t) dlsym(dllib, "dsAudioFormatUpdateRegisterCB");
            if(dsAudioFormatUpdateRegisterCBFun == 0) {
                printf("%s: dsAudioFormatUpdateRegisterCB is not defined %s\r\n", __FUNCTION__, dlerror());
                eRet = dsERR_GENERAL;
            }
            else {
                printf("%s: dsAudioFormatUpdateRegisterCB is loaded\r\n", __FUNCTION__);
            }
            dlclose(dllib);
        }
        else {
            printf("%s: Opening RDK_DSHAL_NAME [%s] failed %s\r\n",
                   __FUNCTION__, RDK_DSHAL_NAME, dlerror());
            eRet = dsERR_GENERAL;
        }
    }
    if (0 != dsAudioFormatUpdateRegisterCBFun) {
        eRet = dsAudioFormatUpdateRegisterCBFun (cbFun);
        printf("%s: dsAudioFormatUpdateRegisterCBFun registered\r\n", __FUNCTION__);
    }
    else {
        printf("%s: dsAudioFormatUpdateRegisterCBFun NULL\r\n", __FUNCTION__);
    }

    return eRet;
}

IARM_Result_t _dsResetBassEnhancer(void *arg)
{
   _DEBUG_ENTER();
   IARM_BUS_Lock(lock);

   IARM_Result_t result = IARM_RESULT_INVALID_STATE;
   int *handle = (int*)arg;
   result = _resetBassEnhancer(*handle);
   IARM_BUS_Unlock(lock);
   return result;

}

static IARM_Result_t _resetBassEnhancer(int handle)
{
    typedef dsError_t (*dsSetBassEnhancer_t)(int handle, int boost);
    static dsSetBassEnhancer_t func = 0;
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetBassEnhancer_t) dlsym(dllib, "dsSetBassEnhancer");
            if (func) {
                printf("dsSetBassEnhancer_t(int, int) is defined and loaded\r\n");
            }
            else {
                printf("dsSetBassEnhancer_t(int, int) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
    if (func != 0) {
       std::string _Property = _dsGetCurrentProfileProperty("BassBoost");
       std::string _BassBoost("0");
       int m_bassBoost = 0;
       try {
            _BassBoost = device::HostPersistence::getInstance().getDefaultProperty(_Property);
       }
       catch(...) {
            _BassBoost = "0";
       }
       m_bassBoost = atoi(_BassBoost.c_str());

       if (func(handle,m_bassBoost) == dsERR_NONE) {
           printf("%s:%s Initialized Bass Boost : %d\n",__func__, _Property.c_str(), m_bassBoost);
           device::HostPersistence::getInstance().persistHostProperty("audio.BassBoost" ,_BassBoost);
           result = IARM_RESULT_SUCCESS;
       }
   }
#endif
   return result; 
}

IARM_Result_t _dsResetVolumeLeveller(void *arg)
{
    _DEBUG_ENTER();
    IARM_BUS_Lock(lock);
    int *handle = (int*)arg;
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;

    result = _resetVolumeLeveller(*handle);
    IARM_BUS_Unlock(lock);
    return result;
}

static IARM_Result_t _resetVolumeLeveller(int handle)
{
    typedef dsError_t (*dsSetVolumeLeveller_t)(int handle, dsVolumeLeveller_t volLeveller);
    static dsSetVolumeLeveller_t func = 0;
    IARM_Result_t result = IARM_RESULT_INVALID_STATE;

    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetVolumeLeveller_t) dlsym(dllib, "dsSetVolumeLeveller");
            if (func) {
                printf("dsSetVolumeLeveller_t(int, dsVolumeLeveller_t) is defined and loaded\r\n");
            }
            else {
                printf("dsSetVolumeLeveller_t(int, dsVolumeLeveller_t) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
    if (func != 0) {
       std::string _PropertyMode = _dsGetCurrentProfileProperty("VolumeLeveller.mode");
       std::string _Propertylevel = _dsGetCurrentProfileProperty("VolumeLeveller.level");
       std::string _volLevellerMode("0");
       std::string _volLevellerLevel("0");
       dsVolumeLeveller_t m_volumeLeveller;
       try {
           _volLevellerMode = device::HostPersistence::getInstance().getDefaultProperty(_PropertyMode);
           _volLevellerLevel = device::HostPersistence::getInstance().getDefaultProperty(_Propertylevel);
       }
       catch(...) {
           _volLevellerMode = "0";
           _volLevellerLevel = "0";
       }
       m_volumeLeveller.mode = atoi(_volLevellerMode.c_str());
       m_volumeLeveller.level = atoi(_volLevellerLevel.c_str());
       if (func(handle, m_volumeLeveller) == dsERR_NONE) {
          printf("%s %s %s Default Volume Leveller : Mode: %d, Level: %d\n",__func__,_PropertyMode.c_str(),_Propertylevel.c_str(), m_volumeLeveller.mode, m_volumeLeveller.level);
          device::HostPersistence::getInstance().persistHostProperty(_PropertyMode ,_volLevellerMode);
          device::HostPersistence::getInstance().persistHostProperty(_Propertylevel ,_volLevellerLevel);
          result = IARM_RESULT_SUCCESS;
       }
   }
#endif
   IARM_BUS_Unlock(lock);
   return result;
}

IARM_Result_t _dsResetSurroundVirtualizer(void *arg)
{
   _DEBUG_ENTER();
   IARM_BUS_Lock(lock);
   int *handle = (int*)arg;
   IARM_Result_t result = IARM_RESULT_INVALID_STATE;

   result = _resetSurroundVirtualizer(*handle);
   IARM_BUS_Unlock(lock);
   return result;
}

static IARM_Result_t _resetSurroundVirtualizer(int handle)
{
   typedef dsError_t (*dsSetSurroundVirtualizer_t)(int handle, dsSurroundVirtualizer_t virtualizer);
   static dsSetSurroundVirtualizer_t func = 0;
   IARM_Result_t result = IARM_RESULT_INVALID_STATE;
   if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsSetSurroundVirtualizer_t) dlsym(dllib, "dsSetSurroundVirtualizer");
            if (func) {
                printf("dsSetSurroundVirtualizer_t(int, dsSurroundVirtualizer_t) is defined and loaded\r\n");
            }
            else {
                printf("dsSetSurroundVirtualizer_t(int, dsSurroundVirtualizer_t) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
  }
#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
  if (func != 0) {
       std::string _PropertyMode = _dsGetCurrentProfileProperty("SurroundVirtualizer.mode");
       std::string _Propertylevel = _dsGetCurrentProfileProperty("SurroundVirtualizer.boost");
       std::string _SVMode("0");
       std::string _SVBoost("0");
       dsSurroundVirtualizer_t m_virtualizer;
       try {
          _SVMode = device::HostPersistence::getInstance().getDefaultProperty(_PropertyMode);
          _SVBoost = device::HostPersistence::getInstance().getDefaultProperty(_Propertylevel);
       }
       catch(...) {
          _SVMode = "0";
          _SVBoost = "0";
       }
       m_virtualizer.mode = atoi(_SVMode.c_str());
       m_virtualizer.boost = atoi(_SVBoost.c_str());
       if (func(handle, m_virtualizer) == dsERR_NONE) {
           printf("%s %s %s Default Surround Virtualizer : Mode: %d, Boost : %d\n",__func__,_PropertyMode.c_str(),_Propertylevel.c_str(), m_virtualizer.mode, m_virtualizer.boost);
           device::HostPersistence::getInstance().persistHostProperty(_PropertyMode ,_SVMode);
           device::HostPersistence::getInstance().persistHostProperty(_Propertylevel ,_SVBoost);
           result = IARM_RESULT_SUCCESS;
       }
  }
#endif
  return result;
}

IARM_Result_t _dsResetDialogEnhancement(void *arg)
{
  _DEBUG_ENTER();
  IARM_BUS_Lock(lock);
  IARM_Result_t result = IARM_RESULT_INVALID_STATE;
  int *handle = (int*)arg;

  result = _resetDialogEnhancerLevel(*handle);
  IARM_BUS_Unlock(lock);
  return result;
}

static IARM_Result_t  _resetDialogEnhancerLevel(int handle)
{
  typedef dsError_t (*dsSetDialogEnhancement_t)(int handle, int enhancerLevel);
  static dsSetDialogEnhancement_t func = 0;
  IARM_Result_t result = IARM_RESULT_INVALID_STATE;
  if (func == 0) {
     void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
     if (dllib) {
        func = (dsSetDialogEnhancement_t) dlsym(dllib, "dsSetDialogEnhancement");
        if (func) {
           printf("dsSetDialogEnhancement_t(int, int) is defined and loaded\r\n");
        }
        else {
           printf("dsSetDialogEnhancement_t(int, int ) is not defined\r\n");
        }
        dlclose(dllib);
     }
     else {
        printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
     }
  }

#ifdef DS_AUDIO_SETTINGS_PERSISTENCE
  std::string _Property = _dsGetCurrentProfileProperty("EnhancerLevel");
  if (func) {
     std::string _EnhancerLevel("0");
     int m_enhancerLevel = 0;
     try {
         _EnhancerLevel = device::HostPersistence::getInstance().getDefaultProperty(_Property);
     }
     catch(...) {
         _EnhancerLevel = "0";
     }
     m_enhancerLevel = atoi(_EnhancerLevel.c_str());
     if (func(handle, m_enhancerLevel) == dsERR_NONE) {
         printf("%s %s Default dialog enhancement level : %d\n",__func__,_Property.c_str(), m_enhancerLevel);
         device::HostPersistence::getInstance().persistHostProperty(_Property ,_EnhancerLevel);
         result = IARM_RESULT_SUCCESS;
     }
  }
#endif
 return result;
}

std::string _dsGetCurrentProfileProperty(std::string property)
{
   std::string _AProfile("Off");
   std::string _AProfileSupport("FALSE");
   try {
        _AProfileSupport = device::HostPersistence::getInstance().getDefaultProperty("audio.MS12Profile.supported");
   }
   catch(...) {
        _AProfileSupport = "FALSE";
        printf("audio.MS12Profile.supported setting not found in hostDataDeafult \r\n");
   }
   printf(" audio.MS12Profile.supported = %s ..... \r\n",_AProfileSupport.c_str());

   if(_AProfileSupport == "TRUE") {
      try {
          _AProfile = device::HostPersistence::getInstance().getProperty("audio.MS12Profile");
      }
      catch(...) {
         try {
             printf("audio.MS12Profile not found in persistence store. Try system default\n");
            _AProfile = device::HostPersistence::getInstance().getDefaultProperty("audio.MS12Profile");
        }
        catch(...) {
            _AProfile = "Off";
        }
      }
   }
   std::string profileProperty("audio.");
   if(_AProfileSupport == "TRUE") {
      profileProperty.append (_AProfile);
      profileProperty.append (".");
   }
   profileProperty.append (property);
   return profileProperty;
}

static std::string _dsGenerateProfileProperty(std::string profile,std::string property)
{
   std::string profileProperty("audio.");
   profileProperty.append (profile);
   profileProperty.append (".");
   profileProperty.append (property);
   return profileProperty;
}

void _dsMS12ProfileSettingOverride(int handle)
{

    typedef dsError_t (*dsSetDialogEnhancement_t)(int handle, int enhancerLevel);
    static dsSetDialogEnhancement_t dsSetDialogEnhancementfunc = 0;
    if (dsSetDialogEnhancementfunc == 0) {
       void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
       if (dllib) {
          dsSetDialogEnhancementfunc = (dsSetDialogEnhancement_t) dlsym(dllib, "dsSetDialogEnhancement");
          if (dsSetDialogEnhancementfunc) {
             printf("dsSetDialogEnhancement_t(int, int) is defined and loaded\r\n");
          }
          else {
             printf("dsSetDialogEnhancement_t(int, int ) is not defined\r\n");
          }
          dlclose(dllib);
       }
       else {
          printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
       }
    }

    if (dsSetDialogEnhancementfunc) {
       std::string _EnhancerLevel("0");
       int m_enhancerLevel = 0;
       std::string _Property = _dsGetCurrentProfileProperty("EnhancerLevel");
       try {
          _EnhancerLevel = device::HostPersistence::getInstance().getProperty(_Property);
       }
       catch(...) {
            try {
                printf("audio.EnhancerLevel not found in persistence store. Try system default\n");
                _EnhancerLevel = device::HostPersistence::getInstance().getDefaultProperty(_Property);
            }
            catch(...) {
                _EnhancerLevel = "0";
            }
       }
       m_enhancerLevel = atoi(_EnhancerLevel.c_str());
       if (dsSetDialogEnhancementfunc(handle, m_enhancerLevel) == dsERR_NONE) {
           device::HostPersistence::getInstance().persistHostProperty(_Property ,_EnhancerLevel);
           printf("%s: persist enhancer level: %d\n",__func__,m_enhancerLevel );
       }
    }
 
    typedef dsError_t (*dsSetBassEnhancer_t)(int handle, int boost);
    static dsSetBassEnhancer_t dsSetBassEnhancerFunc = 0;
    if (dsSetBassEnhancerFunc == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            dsSetBassEnhancerFunc = (dsSetBassEnhancer_t) dlsym(dllib, "dsSetBassEnhancer");
            if (dsSetBassEnhancerFunc) {
                printf("dsSetBassEnhancer_t(int, int) is defined and loaded\r\n");
            }
            else {
                printf("dsSetBassEnhancer_t(int, int) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
           printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }
    if (dsSetBassEnhancerFunc != 0)
    {
        std::string _BassBoost("0");
        int m_bassBoost = 0;
        std::string _Property = _dsGetCurrentProfileProperty("BassBoost");
        try {
            _BassBoost = device::HostPersistence::getInstance().getProperty("audio.BassBoost");
        }
        catch(...) {
            try {
                printf("audio.EnhancerLevel not found in persistence store. Try system default\n");
                _BassBoost = device::HostPersistence::getInstance().getDefaultProperty(_Property);
            }
            catch(...) {
                _BassBoost = "0";
            }
        }
        m_bassBoost = atoi(_BassBoost.c_str());
        if (dsSetBassEnhancerFunc(handle, m_bassBoost) == dsERR_NONE)
        {
            printf("%s: persist boost value: %d\n",__func__, m_bassBoost);
            device::HostPersistence::getInstance().persistHostProperty("audio.BassBoost" ,_BassBoost);
        }
    }

    typedef dsError_t (*dsSetVolumeLeveller_t)(int handle, dsVolumeLeveller_t volLeveller);
    static dsSetVolumeLeveller_t dsSetVolumeLevellerfunc = 0;
    if (dsSetVolumeLevellerfunc == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            dsSetVolumeLevellerfunc = (dsSetVolumeLeveller_t) dlsym(dllib, "dsSetVolumeLeveller");
            if (dsSetVolumeLevellerfunc) {
                printf("dsSetVolumeLeveller_t(int, dsVolumeLeveller_t) is defined and loaded\r\n");
            }
            else {
                printf("dsSetVolumeLeveller_t(int, dsVolumeLeveller_t) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
              printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    if (dsSetVolumeLevellerfunc != 0 )
    {
        std::string _volLevellerMode("0");
        std::string _volLevellerLevel("0");
        dsVolumeLeveller_t m_volumeLeveller;
        std::string _PropertyMode = _dsGetCurrentProfileProperty("VolumeLeveller.mode");
        std::string _PropertyLevel = _dsGetCurrentProfileProperty("VolumeLeveller.level");
        try {
            _volLevellerMode = device::HostPersistence::getInstance().getProperty(_PropertyMode);
            _volLevellerLevel = device::HostPersistence::getInstance().getProperty(_PropertyLevel);
        }
        catch(...) {
            try {
                _volLevellerMode = device::HostPersistence::getInstance().getDefaultProperty(_PropertyMode);
                _volLevellerLevel = device::HostPersistence::getInstance().getDefaultProperty(_PropertyLevel);
            }
            catch(...) {
                _volLevellerMode = "0";
                _volLevellerLevel = "0";
            }
        }
        m_volumeLeveller.mode = atoi(_volLevellerMode.c_str());
        m_volumeLeveller.level = atoi(_volLevellerLevel.c_str());
        if (dsSetVolumeLevellerfunc(handle, m_volumeLeveller) == dsERR_NONE)
        {
            printf("%s: persist volume leveller mode: %d\n",__func__, m_volumeLeveller.mode);
            device::HostPersistence::getInstance().persistHostProperty(_PropertyMode,_volLevellerMode);
            printf("%s: persist volume leveller value: %d\n",__func__, m_volumeLeveller.level);
            device::HostPersistence::getInstance().persistHostProperty(_PropertyLevel,_volLevellerLevel);
        }
    }   
    
    typedef dsError_t (*dsSetSurroundVirtualizer_t)(int handle, dsSurroundVirtualizer_t virtualizer);
    static dsSetSurroundVirtualizer_t dsSetSurroundVirtualizerfunc = 0;
    if (dsSetSurroundVirtualizerfunc == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
        dsSetSurroundVirtualizerfunc = (dsSetSurroundVirtualizer_t) dlsym(dllib, "dsSetSurroundVirtualizer");
            if (dsSetSurroundVirtualizerfunc) {
                printf("dsSetSurroundVirtualizer_t(int, dsSurroundVirtualizer_t) is defined and loaded\r\n");
            }
            else {
                printf("dsSetSurroundVirtualizer_t(int, dsSurroundVirtualizer_t) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
              printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    if (dsSetSurroundVirtualizerfunc != 0 )
    {
        std::string _SVMode("0");
        std::string _SVBoost("0");
        dsSurroundVirtualizer_t m_virtualizer;
        std::string _PropertyMode = _dsGetCurrentProfileProperty("SurroundVirtualizer.mode");
        std::string _PropertyBoost = _dsGetCurrentProfileProperty("SurroundVirtualizer.boost");
        try {
            _SVMode = device::HostPersistence::getInstance().getProperty(_PropertyMode);
            _SVBoost = device::HostPersistence::getInstance().getProperty(_PropertyBoost);
        }
        catch(...) {
            try {
                _SVMode = device::HostPersistence::getInstance().getDefaultProperty(_PropertyMode);
                _SVBoost = device::HostPersistence::getInstance().getDefaultProperty(_PropertyBoost);
            }
            catch(...) {
                _SVMode = "0";
                _SVBoost = "0";
            }
        }
        m_virtualizer.mode = atoi(_SVMode.c_str());
        m_virtualizer.boost = atoi(_SVBoost.c_str());

        if (dsSetSurroundVirtualizerfunc(handle,m_virtualizer) == dsERR_NONE)
        {
            printf("%s: persist surround virtualizer mode: %d\n",__func__, m_virtualizer.mode);
            device::HostPersistence::getInstance().persistHostProperty(_PropertyMode,_SVMode);
            printf("%s: persist surround virtualizer boost value: %d\n",__func__, m_virtualizer.boost);
            device::HostPersistence::getInstance().persistHostProperty(_PropertyBoost,_SVBoost);
        }
    }
}

bool _dsMs12ProfileSupported(int handle,std::string profile)
{
    typedef dsError_t (*dsGetMS12AudioProfileList_t)(int handle, dsMS12AudioProfileList_t* profiles);
    static dsGetMS12AudioProfileList_t func = 0;
    if (func == 0) {
        void *dllib = dlopen(RDK_DSHAL_NAME, RTLD_LAZY);
        if (dllib) {
            func = (dsGetMS12AudioProfileList_t) dlsym(dllib, "dsGetMS12AudioProfileList");
            if (func) {
                printf("dsGetMS12AudioProfileList_t(int, dsMS12AudioProfileList_t*) is defined and loaded\r\n");
            }
            else {
                printf("dsGetMS12AudioProfileList_t(int, dsMS12AudioProfileList_t*) is not defined\r\n");
            }
            dlclose(dllib);
        }
        else {
            printf("Opening RDK_DSHAL_NAME [%s] failed\r\n", RDK_DSHAL_NAME);
        }
    }

    dsMS12AudioProfileList_t pProfilesStr;
    bool result = 0;
    dsError_t ret = dsERR_NONE;
    if (func != 0 )
    {
        ret = func(handle, &pProfilesStr);
        if (ret == dsERR_NONE)
        {
            if(strstr(pProfilesStr.audioProfileList,profile.c_str()))
               result = 1;
            else
               result = 0;
        }
    }
    return result;
}

IARM_Result_t _dsGetHDMIARCPortId(void *arg)
{
#ifndef RDK_DSHAL_NAME
#warning   "RDK_DSHAL_NAME is not defined"
#define RDK_DSHAL_NAME "RDK_DSHAL_NAME is not defined"
#endif
    _DEBUG_ENTER();

    IARM_BUS_Lock(lock);
    std::string _HDMIARCPortId("0");
    dsGetHDMIARCPortIdParam_t *param = ( dsGetHDMIARCPortIdParam_t *)arg;
    try {
            _HDMIARCPortId = device::HostPersistence::getInstance().getDefaultProperty("HDMIARC.port.Id");
    }
    catch(...) {
            _HDMIARCPortId = "-1";
    }
    param->portId = atoi(_HDMIARCPortId.c_str());
    printf("The HDMI ARC Port Id is %d \r\n",param->portId);
    IARM_BUS_Unlock(lock);
    return IARM_RESULT_SUCCESS;
}

/** @} */
/** @} */
