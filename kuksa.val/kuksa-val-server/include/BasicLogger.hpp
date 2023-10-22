/**********************************************************************
 * Copyright (c) 2019 Robert Bosch GmbH.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Contributors:
 *      Robert Bosch GmbH - initial API and functionality
 **********************************************************************/

#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "ILogger.hpp"

#include <mutex>


/**
 * \class Logger
 * \brief Implementation class of logging utility
 *
 * Logger shall provide standardized way to put logging information
 * to standard output (stdout)
 *
 */
class BasicLogger : public ILogger {
  private:
    std::mutex    accessMutex;
    const uint8_t logLevels;

  public:
    BasicLogger(uint8_t logEventsEnabled);
    ~BasicLogger();

    void Log(LogLevel level, std::string logString);
};

#endif /* __LOGGER_H__ */
