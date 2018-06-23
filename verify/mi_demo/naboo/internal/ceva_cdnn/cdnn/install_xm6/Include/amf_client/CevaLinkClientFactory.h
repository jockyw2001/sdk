/*
 * CevaLinkClientFactory.h
 *
 *  Created on: Jan 08, 2014
 *      Author: yurys
 */

#ifndef _CEVALINKCLIENTFACTORY_H_
#define _CEVALINKCLIENTFACTORY_H_

/*!
* \addtogroup CevaLinkClient
*  @{
*/


#include "ICevaLinkClient.h"

namespace CEVA_AMF {

//! Factory class for the concrete CEVA Link client instances
class CevaLinkClientFactory {
public:
	//! Enumeration type for the CEVA Link classes
    enum ClientType { CLIENT_TYPE_BASE };

    /// \brief Creates an instance of the given client type. The instance must be freed by the user.
    /// \param[in] type The enumeration value of the client type
    /// \return A pointer to the new instance
    static ICevaLinkClient* create( ClientType type );
};

}

/** @}*/
#endif /* _CEVALINKCLIENTFACTORY_H_ */
