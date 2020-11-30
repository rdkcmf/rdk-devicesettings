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
 * @file compositeIn.cpp
 * @brief Configuration of COMPOSITE Input
 */

/**
* @defgroup devicesettings
* @{
* @defgroup ds
* @{
**/


#include <iostream>
#include <sstream>
#include <string>
#include <string.h>
#include "compositeIn.hpp"
#include "illegalArgumentException.hpp"
#include "host.hpp"

#include "dslogger.h"
#include "dsError.h"
#include "dsTypes.h"
#include "dsCompositeIn.h"
#include "dsUtl.h"

// Enable for debug tracing
// #define COMPOSITE_IN_DEBUG
#ifdef COMPOSITE_IN_DEBUG
   #define COMPOSITE_IN_TRACE( m ) printf m
#else
   #define COMPOSITE_IN_TRACE( m )
#endif

namespace device
{


/**
 * @fn  CompositeInput::CompositeInput()
 * @brief default constructor
 *
 * @param None
 *
 * @return None
 * @callergraph
 */
CompositeInput::CompositeInput()
{
    COMPOSITE_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    COMPOSITE_IN_TRACE(("%s - invoking dsCompositeInInit()\n", __PRETTY_FUNCTION__));
    dsCompositeInInit();

    COMPOSITE_IN_TRACE(("%s <--- \n", __PRETTY_FUNCTION__));
}

/**
 * @fn  CompositeInput::~CompositeInput()
 * @brief destructor
 *
 * @param None
 *
 * @return None
 * @callergraph
 */
CompositeInput::~CompositeInput()
{
    COMPOSITE_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    COMPOSITE_IN_TRACE(("%s - invoking dsCompositeInTerm()\n", __PRETTY_FUNCTION__));
    dsCompositeInTerm();

    COMPOSITE_IN_TRACE(("%s <--- \n", __PRETTY_FUNCTION__));
}

/**
 * @fn  CompositeInput::getInstance()
 * @brief This API is used to get the instance of the COMPOSITE Input
 *
 * @param None
 *
 * @return Reference to the instance of COMPOSITE Input class instance
 * @callergraph
 */
CompositeInput & CompositeInput::getInstance()
{
    static CompositeInput _singleton;

    COMPOSITE_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));
    COMPOSITE_IN_TRACE(("%s <--- \n", __PRETTY_FUNCTION__));

	return _singleton;
}

/**
 * @fn  CompositeInput::getNumberOfInputs()
 * @brief This API is used to get the number of COMPOSITE Input ports on the set-top
 *
 * @param[in] None
 *
 * @return number of COMPOSITE Inputs
 * @callergraph
 */
uint8_t CompositeInput::getNumberOfInputs() const
{
    COMPOSITE_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    COMPOSITE_IN_TRACE(("%s - invoking dsCompositeInGetNumberOfInputs()\n", __PRETTY_FUNCTION__));

    uint8_t numCompositeInputs;
    dsError_t eError = dsCompositeInGetNumberOfInputs (&numCompositeInputs);

	// Throw an exception if there was an error
	if (dsERR_NONE != eError)
	{
        COMPOSITE_IN_TRACE(("%s <--- exception error=%d \n", __PRETTY_FUNCTION__, eError));
		throw Exception(eError);
	}

    COMPOSITE_IN_TRACE(("%s <--- Number of inputs=%d\n", __PRETTY_FUNCTION__, numCompositeInputs));
    return (numCompositeInputs);
}

/**
 * @fn  CompositeInput::isPresented()
 * @brief This API is used to specify if COMPOSITE Input is being
 *        presented via COMPOSITE Out
 *
 * @param[in] None
 *
 * @return true if COMPOSITE Input is being presetned.
 * @callergraph
 */
bool CompositeInput::isPresented() const
{
    COMPOSITE_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    COMPOSITE_IN_TRACE(("%s - invoking dsCompositeInGetStatus()\n", __PRETTY_FUNCTION__));
    dsCompositeInStatus_t Status;
    dsError_t eError = dsCompositeInGetStatus (&Status);

	// Throw an exception if there was an error
	if (dsERR_NONE != eError)
	{
        COMPOSITE_IN_TRACE(("%s <--- exception error=%d \n", __PRETTY_FUNCTION__, eError));
		throw Exception(eError);
	}
    COMPOSITE_IN_TRACE(("%s <--- \n", __PRETTY_FUNCTION__));
	return (Status.isPresented);
}

/**
 * @fn  CompositeInput::isActivePort()
 * @brief This API is used to specify if the provided COMPOSITE Input port is
 *        active (i.e. communicating with the set-top)
 *
 * @param[in] COMPOSITE Input port
 *
 * @return true if the provided COMPOSITE Input port is active.
 * @callergraph
 */
bool CompositeInput::isActivePort(int8_t Port) const
{
    COMPOSITE_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    COMPOSITE_IN_TRACE(("%s - invoking dsCompositeInGetStatus()\n", __PRETTY_FUNCTION__));
    dsCompositeInStatus_t Status;
    dsError_t eError = dsCompositeInGetStatus (&Status);

	// Throw an exception if there was an error
	if (dsERR_NONE != eError)
	{
        COMPOSITE_IN_TRACE(("%s <--- exception error=%d \n", __PRETTY_FUNCTION__, eError));
		throw Exception(eError);
	}
    COMPOSITE_IN_TRACE(("%s <--- \n", __PRETTY_FUNCTION__));
	return (Status.activePort == Port);
}

/**
 * @fn  CompositeInput::getActivePort()
 * @brief This API is used to specify the active (i.e. communicating with
 *        the set-top) COMPOSITE Input port
 *
 * @param[in] None
 *
 * @return the COMPOSITE Input port which is currently active.
 * @callergraph
 */
int8_t CompositeInput::getActivePort() const
{
    COMPOSITE_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    COMPOSITE_IN_TRACE(("%s - invoking dsCompositeInGetStatus()\n", __PRETTY_FUNCTION__));
    dsCompositeInStatus_t Status;
    dsError_t eError = dsCompositeInGetStatus (&Status);

	// Throw an exception if there was an error
	if (dsERR_NONE != eError)
	{
        COMPOSITE_IN_TRACE(("%s <--- exception error=%d \n", __PRETTY_FUNCTION__, eError));
		throw Exception(eError);
	}
    COMPOSITE_IN_TRACE(("%s <--- \n", __PRETTY_FUNCTION__));
	return (Status.activePort);
}

/**
 * @fn  CompositeInput::isPortConnected()
 * @brief This API is used to specify if the prvided COMPOSITE Input port is
 *        connected (i.e. COMPOSITE Input devie is plugged into the set-top).
 *
 * @param[in] COMPOSITE Input port
 *
 * @return true if the COMPOSITE Input port is connected
 * @callergraph
 */
bool CompositeInput::isPortConnected(int8_t Port) const
{
    COMPOSITE_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    COMPOSITE_IN_TRACE(("%s - invoking dsCompositeInGetStatus()\n", __PRETTY_FUNCTION__));
    dsCompositeInStatus_t Status;
    dsError_t eError =  dsCompositeInGetStatus (&Status);

	// Throw an exception if there was an error
	if (dsERR_NONE != eError)
	{
        COMPOSITE_IN_TRACE(("%s <--- exception error=%d \n", __PRETTY_FUNCTION__, eError));
		throw Exception(eError);
	}
    COMPOSITE_IN_TRACE(("%s <--- \n", __PRETTY_FUNCTION__));
	return (Status.isPortConnected[Port]);
}

/**
 * @fn  CompositeInput::selectPort()
 * @brief This API is used to select the COMPOSITE In port to be presented
 *
 * @param[in] int8_t Port : -1 for No COMPOSITE Input port to be presented
 *                           0..n for COMPOSITE Input port (n) to be presented 
 *
 * @return None
 * @callergraph
 */
void CompositeInput::selectPort (int8_t Port) const
{
    COMPOSITE_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    COMPOSITE_IN_TRACE(("%s - invoking dsCompositeInSelectPort()\n", __PRETTY_FUNCTION__));
    dsError_t eError = dsCompositeInSelectPort ((dsCompositeInPort_t)Port);

	// Throw an exception if there was an error
	if (dsERR_NONE != eError)
	{
        COMPOSITE_IN_TRACE(("%s <--- exception error=%d \n", __PRETTY_FUNCTION__, eError));
		throw Exception(eError);
	}
    COMPOSITE_IN_TRACE(("%s <--- \n", __PRETTY_FUNCTION__));
}

/**
 * @fn  CompositeInput::scaleVideo()
 * @brief This API is used to scale the COMPOSITE In video
 *
 * @param[in] int32_t x      : x coordinate for the video
 * @param[in] int32_t y      : y coordinate for the video
 * @param[in] int32_t width  : width of the video
 * @param[in] int32_t height : height of the video
 *
 * @return None
 * @callergraph
 */
void CompositeInput::scaleVideo (int32_t x, int32_t y, int32_t width, int32_t height) const
{
    COMPOSITE_IN_TRACE(("%s ---> \n", __PRETTY_FUNCTION__));

    COMPOSITE_IN_TRACE(("%s - invoking dsCompositeInScaleVideo(%d, %d, %d, %d)\n", __PRETTY_FUNCTION__, x, y, width, height));
    dsError_t eError = dsCompositeInScaleVideo (x, y, width, height);

	// Throw an exception if there was an error
	if (dsERR_NONE != eError)
	{
        COMPOSITE_IN_TRACE(("%s <--- exception error=%d \n", __PRETTY_FUNCTION__, eError));
		throw Exception(eError);
	}
    COMPOSITE_IN_TRACE(("%s <--- \n", __PRETTY_FUNCTION__));
}

}


/** @} */
/** @} */