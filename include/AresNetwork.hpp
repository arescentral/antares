// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#ifndef ANTARES_ARES_NETWORK_HPP_
#define ANTARES_ARES_NETWORK_HPP_

// Ares Network.h

#include <appletalk.h>
#include <Base.h>

#include "AnyChar.hpp"
#include "SpaceObject.hpp"

#define kAresNetwork

#define kMaxNetPlayer           16
#define kMaxAvailableHost       16
#define kMaxAvailableClient     16

#define kNetNameLength          31

#define kAresNetObjectName      "\pAres 0.0.1 Object"
#define kAresNetTypeName        "\pAres 0.0.1 Type"

enum netStatusType { noNetwork, hostWorking, clientWorking, noNetGame };

enum datagramType {
    kClientAddress = 1,
    kAssignClientNumber = 2,
    kRequestPlayerShip = 3,
    kAssignPlayerShip = 4,
    kUpdateSpaceObject = 5
};

struct datagramHeaderType {
    datagramType    type;
    long            clientNumber;
    union
    {
        AddrBlock       address;
        long            playerShipNum;
        long            clientNumber;
        struct
        {
            spaceObjectType spaceObject;
            long            spaceObjectNum;
        } spaceObjectData;
    } header;
};

enum netMessageType {
    kYouAreInvited = 1,                 // host to client invitation
    kWhatIsYourName = 2,                // host requests client name
    kWeAreStarting = 3,                 // host tells client to prepare for beginning
    kWeAreCanceling = 4,                // host tells client that game is canceled

    kIReceivedInvite = 5,               // client tells host that invite received OK
    kIAcceptInvite = 6,                 // client tells host that user OKs invite
    kIDeclineInvite = 7,                // client tells host that user declines invite
    kMyNameIs = 8,                      // client response to host name request

    kYourNumberIs = 9,                  // host assigns client a client number
    kPlayerNumberData = 10,             // host gives client another client's # and address

    kNewSpaceObject = 11,               // anybody tells client to add a new object
    kRefreshThisSpaceObject = 12,       // annybody tells client to update object's info

    kImReady = 13,                      // generic simple I'm Ready message
    kStartTiming = 14                   // simple message to start global timer

};


//
// legal netMessageTypes for a netSetupDataType message:
//  kYouAreInvited
//  kWhatIsYourName
//  kWeAreStarting
//  kWeAreCanceling
//  kIReceivedInvite
//  kIAcceptInvite
//  kIDeclineInvite
//  kMyNameIs
//  kYourNumberIs
//  kPlayerNumberData
//

struct netSetupDataType {
    netMessageType          type;
    AddrBlock               address;
    short                   number;
    anyCharType             name[kNetNameLength];
};

//
// legal netMessageTypes for a netSpaceObjectType message:
//  kNewSpaceObject
//  kRefreshThisSpaceObject
//

struct netSpaceObjectType {
    netMessageType          type;
    short                   playerNum;
    spaceObjectType         spaceObject;
    long                    spaceObjectNum;
};

//
// legal netMessageTypes for a netSimpleMessageType message:
//  kWeAreStarting
//  kImReady
//  kStartTiming
//

struct netSimpleMessageType {
    netMessageType          type;
    short                   playerNum;
};

struct netAresEntity {
    AddrBlock               address;
    anyCharType             name[kNetNameLength];
};

enum netSetupStatusType {
    kAvailable = 1,
    kNotInterested = 2,
    kInGame = 3,
    kResponding = 4,
    kInvitedNoResponse = 5,
    kHasNoName = 6,
    kDoesNotExist = 7
};

struct netClientEntity {
    netAresEntity           entity;
    Boolean                 stillThere;
    Boolean                 selected;
    netSetupStatusType      status;
};

void AresNetworkInit ( void);
void AresNetworkClose ( void);
datagramHeaderType *RetreiveAndTossDatagram( datagramType);
netClientEntity *GetClientAddressMatch( AddrBlock   *anAddress);
long GetNetPlayerShip( short);
void ProcessUsedDatagram( void);
void SendSpaceObjectUpdate( long which);
void AresNetworkHostBegin( void);
void AresNetworkClientBegin( void);
void HostSynchronizeTime( void);
void ClientSynchronizeTime( void);

#endif // ANTARES_ARES_NETWORK_HPP_
