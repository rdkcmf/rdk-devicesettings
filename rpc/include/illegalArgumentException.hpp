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


#ifndef _DS_MGR_ILLEGALARGUMENTEXCEPTION_HPP_
#define _DS_MGR_ILLEGALARGUMENTEXCEPTION_HPP_

#include "exception.hpp"
#include <iostream>

namespace device {

class IllegalArgumentException : public device::Exception {
public:
	IllegalArgumentException(const char *msg = "Illegal Argument") throw (): device::Exception(msg) {
	};

	virtual ~IllegalArgumentException() throw(){};
};

}

#endif /* ILLEGALARGUMENTEXCEPTION_HPP_ */


/** @} */
/** @} */
