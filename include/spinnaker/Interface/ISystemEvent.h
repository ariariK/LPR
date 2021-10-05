//=============================================================================
// Copyright (c) 2001-2019 FLIR Systems, Inc. All Rights Reserved.
//
// This software is the confidential and proprietary information of FLIR
// Integrated Imaging Solutions, Inc. ("Confidential Information"). You
// shall not disclose such Confidential Information and shall use it only in
// accordance with the terms of the license agreement you entered into
// with FLIR Integrated Imaging Solutions, Inc. (FLIR).
//
// FLIR MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
// SOFTWARE, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, OR NON-INFRINGEMENT. FLIR SHALL NOT BE LIABLE FOR ANY DAMAGES
// SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
// THIS SOFTWARE OR ITS DERIVATIVES.
//=============================================================================

#ifndef FLIR_SPINNAKER_ISYSTEM_EVENT_H
#define FLIR_SPINNAKER_ISYSTEM_EVENT_H

#include "Event.h"
#include "InterfaceArrivalEvent.h"
#include "InterfaceRemovalEvent.h"
#include "SpinnakerPlatform.h"

namespace Spinnaker
{
    class SPINNAKER_API ISystemEvent : public virtual IInterfaceArrivalEvent, public virtual IInterfaceRemovalEvent
    {
      public:
        virtual ~ISystemEvent(){};
        virtual void OnInterfaceArrival(std::string interfaceID) = 0;
        virtual void OnInterfaceRemoval(std::string interfaceID) = 0;

      protected:
        ISystemEvent(){};
        ISystemEvent(const ISystemEvent&){};
        ISystemEvent& operator=(const ISystemEvent&);
    };
} // namespace Spinnaker

#endif // FLIR_SPINNAKER_ISYSTEM_EVENT_H