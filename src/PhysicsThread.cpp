/*
 * This file is part of the HSL distribution (https://github.com/kristian80/HSL).
 * Copyright (c) 2019 Kristian80.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "PhysicsThread.h"
#include "HSL_PlugIn.h"
#include <thread>
#ifdef LIN
	#include <pthread.h>
#endif

PhysicsThread::PhysicsThread(HSL_PlugIn& HSLNew) :
	HSL(HSLNew)
{
}

void PhysicsThread::RunPhysicsThread(int index)
{
	using namespace std::chrono_literals;

	std::unique_lock<std::recursive_mutex> physics_shm_lock(cargoDataSharedMutex, std::defer_lock);

	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

	while (myRunFlag)
	{
		if (physics_shm_lock.try_lock()) // Non-Blocking Lock
		{
			if (HIGH_PERFORMANCE != HSL.myCargoDataShared.myHighPerformace)
			{
#ifdef IBM
				if (HSL.myCargoDataShared.myHighPerformace == true)
					SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
				else
					SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
#else
				// taken from https://stackoverflow.com/questions/10876342/equivalent-of-setthreadpriority-on-linux-pthreads
				// I don't think we should implement this in Linux.
				// In any case when I debuged the code, the values were 0 and 0 for policy and max_allowed, so it seems a moot point to implement this

//				pthread_t thId = pthread_self();
//				pthread_attr_t thAttr;
//				int policy = 0;
//				int max_prio_for_policy = 0;
//				if (HSL.myCargoDataShared.myHighPerformace == true)
//				{
//					pthread_attr_init(&thAttr);
//					pthread_attr_getschedpolicy(&thAttr, &policy);
//					max_prio_for_policy = sched_get_priority_max(policy);
//
//
//					pthread_setschedprio(thId, max_prio_for_policy);
//					pthread_attr_destroy(&thAttr);
//				}

#endif
			}

			HIGH_PERFORMANCE = HSL.myCargoDataShared.myHighPerformace;
			myRunFlag = HSL.myCargoDataShared.myThreadRunFlag;

			if (HSL.myCargoDataShared.myComputationRunFlag == true)
			{

				if (HSL.myCargoDataShared.myNewFrame == true)
				{
					HSL.myCargo.myUpdateHelicopterPosition = true;
					HSL.myHook.myUpdateHelicopterPosition = true;
					HSL.myCargoDataShared.myNewFrame = false;
				}

				HSL.myCargo.CalculatePhysics();
				HSL.myHook.CalculatePhysics();
			}

			physics_shm_lock.unlock(); // Unlock
		}
		if (HIGH_PERFORMANCE == true)
		{
			volatile int counter = 0;
			for (counter = 0; counter < 10000; counter++);
		}
		else
		{
			std::this_thread::sleep_for(1us);
		}

	}
}
