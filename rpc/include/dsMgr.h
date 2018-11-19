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


#ifndef RPDSMGR_H_
#define RPDSMGR_H_
#include "libIARM.h"
#include "dsTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IARM_BUS_DSMGR_NAME		"DSMgr"

IARM_Result_t dsMgr_init();
IARM_Result_t dsMgr_term();


/*! Events published from DS Mananger */
typedef enum _DSMgr_EventId_t {
	IARM_BUS_DSMGR_EVENT_RES_PRECHANGE=0,              /*!< Resolution Pre Change Event  */
    IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE,               /*!< Resolution Post Change Event  */
	IARM_BUS_DSMGR_EVENT_ZOOM_SETTINGS,				   /*!< Zoom Settings Events */
	IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG,                 /*!< HDMI HPD DETECT Change Event  */
	IARM_BUS_DSMGR_EVENT_AUDIO_MODE,                   /*!< HDMI HPD DETECT Change Event  */
	IARM_BUS_DSMGR_EVENT_HDCP_STATUS,                  /*!< HDMI HDCP status */
	IARM_BUS_DSMGR_EVENT_RX_SENSE,                     /*!< HDMI Rx Sense status */
	IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG,              /*!< HDMI IN HPD change event */
    IARM_BUS_DSMGR_EVENT_TIME_FORMAT_CHANGE,           /*!< Clock Time Format Change Event*/
	IARM_BUS_DSMGR_EVENT_MAX,					       /*!< Max Event  */
} IARM_Bus_DSMgr_EventId_t;



/*! DS Manager  Event Data */
typedef struct _DSMgr_EventData_t {
    union {
        struct _RESOLUTION_DATA{
        	/* Declare Event Data structure for Video resolution Event */
            int width;              /*!< Resolution Width  */
            int height;              /*!< Key code */ 
        } resn; /*Reolution data*/
		struct _DFC_DATA{
        	/* Declare Event Data structure for Zoom settings Event */
            int zoomsettings;
        }dfc; /*zoom data*/
		
		struct _AUDIOMODE_DATA {
        	/* Declare Event Data structure for Video resolution Event */
            int type;              /*!< device type  */
            int mode;              /*!< device mode */ 
        }Audioport; /*Audio mode  data*/

		struct _HDMI_HPD_DATA{
        	/* Declare HDMI HPD Data */
            int event;
        }hdmi_hpd; /*HDMI Hot Plug detect*/
		
		struct _HDMI_HDCP_DATA{
        	/* Declare HDMI DCP Data */
            int hdcpStatus;
        }hdmi_hdcp; /*HDMI HDCP Hot Plug detect*/

        struct _HDMI_RXSENSE_DATA{
            /* Declare HDMI Rx Sense status */
            int status;
        }hdmi_rxsense; /*HDMI Rx Sense Data*/
        
        struct _HDMI_IN_CONNECT_DATA
        {
            dsHdmiInPort_t port;
            bool           isPortConnected;
        }hdmi_in_connect;
        
        struct _FPD_TIME_FORMAT
        {
            dsFPDTimeFormat_t  eTimeFormat;   
        }FPDTimeFormat;

        struct _HDCP_PROTOCOL_DATA
        {
        	dsHdcpProtocolVersion_t  protocolVersion;
        }HDCPProtocolVersion;

    } data;
}IARM_Bus_DSMgr_EventData_t;

#ifdef __cplusplus
}
#endif

#endif /* RPDSMGR_H_ */


/** @} */
/** @} */
