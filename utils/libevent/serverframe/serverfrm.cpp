/*
 * @Author: huqing 
 * @Date: 2018-12-10 10:53:41 
 * @Last Modified by:   huqing 
 * @Last Modified time: 2018-12-10 10:53:41 
 */

#include "serverfrm.h"
#include "../event/include/event2/event.h"
#include "../event/include/event2/buffer.h"
#include "../event/include/event2/bufferevent.h"

serverfrm::serverfrm()
	:	m_pEvBase( NULL )
	,	m_pBufferEnv( NULL )
	,	m_strAddr( "" )
	,	m_iPort( 16068 )
{
}

serverfrm::~serverfrm()
{
	if (m_pBufferEnv)
	{
		bufferevent_free(m_pBufferEnv);
		m_pBufferEnv = NULL;
	}

	if (m_pEvBase)
	{
		event_base_free(m_pEvBase);
		m_pEvBase = NULL;
	}
}

bool	serverfrm::initialModel()
{
	m_pEvBase = event_base_new();
	if (!m_pEvBase)
	{ 
		Fail("func[%s], event_base_new failure, errMsg[%s]", __FUNCTION__, 
			evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
		return false;
	}
	return true;
}

bool	serverfrm::exitModel()
{
	return true;
}

bool	serverfrm::runloop()
{

	return true;
}
