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
 
// TODO: Include your class to test here.


/**
* @defgroup devicesettings
* @{
* @defgroup sample
* @{
**/


#include "videoOutputPort.hpp"
#include "videoOutputPortType.hpp"
#include "host.hpp"
#include "audioOutputPort.hpp"
#include "dsUtl.h"
#include "dsError.h"
#include "stdlib.h"
#include "manager.hpp"



#include "libIBus.h"


#include <string>



int main(int argc, char *argv[]) 
{
   
    if (argc != 5) {
        printf("%s : <Port Type - HDMI, Component> <Port Number-0, 1, 2...> < 0 = STEREO, SURROUND,PASSTHRU> < 1 = PERSIST , 0 - DO NOT PERSIST>\r\n", argv[0]);
        return 0;
    }

    char *portType = argv[1];
    char *portId = argv[2];
    char *mode = argv[3];
    int isPersist = atoi((const char *)argv[4]);


    IARM_Bus_Init("dsTestAudioMode");
    IARM_Bus_Connect();

    device::Manager::Initialize();

    try {
		printf("Sample Application: set Audio Mode %s with persist value %d \r\n",mode,isPersist);
		device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(std::string(portType).append(portId));
		vPort.getAudioOutputPort().setStereoMode(mode,isPersist);
		printf("Sample Application: set Audio Mode is completed\r\n");
    }
    catch (...) {
    	printf("Exception Caught during [%s]\r\n", argv[0]);
    }

    device::Manager::DeInitialize();
	IARM_Bus_Disconnect();
	IARM_Bus_Term();

    return 0;
}


/** @} */
/** @} */
