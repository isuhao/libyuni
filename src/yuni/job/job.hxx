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
#pragma once
#include "job.h"



namespace Yuni
{
namespace Job
{

	inline Job::State IJob::state() const
	{
		return static_cast<Job::State>((int) pState);
	}


	inline bool IJob::idle() const
	{
		return ((int) pState == static_cast<int>(State::idle));
	}


	inline bool IJob::waiting() const
	{
		return ((int) pState == static_cast<int>(State::waiting));
	}


	inline bool IJob::running() const
	{
		return ((int) pState == static_cast<int>(State::running));
	}


	inline void IJob::cancel()
	{
		pCanceling = true;
	}


	inline bool IJob::canceling() const
	{
		return (0 != pCanceling);
	}


	inline void IJob::progression(const int p)
	{
		pProgression = ((p < 0) ? 0 : (p > 100 ? 100 : p));
	}


	inline bool IJob::finished() const
	{
		// The state must be at the very end
		return  (pProgression >= 100
			and (int) pState == static_cast<int>(State::idle));
	}


	inline bool IJob::shouldAbort() const
	{
		assert(pThread != NULL and "Job: The pointer to the attached thread must not be NULL");
		return (pCanceling or pThread->shouldAbort());
	}


	template<class T>
	void IJob::fillInformation(T& info)
	{
		// The first important value is the state
		info.state = static_cast<Job::State>((int) pState);
		// Then, if the job is canceling its work
		info.canceling = (0 != pCanceling);

		info.progression = pProgression;
		info.name = caption();
	}


	inline String IJob::caption() const
	{
		return nullptr;
	}





} // namespace Job
} // namespace Yuni
