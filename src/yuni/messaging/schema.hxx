/*
** This file is part of libyuni, a cross-platform C++ framework (http://libyuni.org).
**
** This Source Code Form is subject to the terms of the Mozilla Public License
** v.2.0. If a copy of the MPL was not distributed with this file, You can
** obtain one at http://mozilla.org/MPL/2.0/.
**
** gitlab: https://gitlab.com/libyuni/libyuni/
** github: https://github.com/libyuni/libyuni/ {mirror}
*/
#ifndef __YUNI_MESSAGING_API_SCHEMA_HXX__
# define __YUNI_MESSAGING_API_SCHEMA_HXX__


namespace Yuni
{
namespace Messaging
{

	inline void Schema::shrinkMemory()
	{
		defaults.shrinkMemory();
		methods.shrinkMemory();
	}


	inline void Schema::Defaults::shrinkMemory()
	{
		pTmp.clear();
		pTmp.shrink();
	}


	inline const API::Method::Parameter::Hash& Schema::Defaults::params() const
	{
		return pParams;
	}





} // namespace Messaging
} // namespace Yuni

#endif // __YUNI_MESSAGING_API_SCHEMA_HXX__
