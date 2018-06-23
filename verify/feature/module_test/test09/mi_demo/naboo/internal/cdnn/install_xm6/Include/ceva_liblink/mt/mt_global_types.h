#ifndef MT_GLOBAL_TYPES_H_
#define MT_GLOBAL_TYPES_H_

#define MUTEX_LOCK_SUCCESS (0)
#define MUTEX_LOCK_MAX_RECURSIVE_LOCKS_REACHED (-12)
#define MUTEX_UNLOCK_SUCCESS (0)
#define MUTEX_UNLOCK_MUTEX_IS_NOT_LOCKED_BY_THIS_THREAD (-13)

#define CONDITION_WAIT_SUCCESS (0)
#define CONDITION_WAIT_MUTEX_LOCK_ERROR (-1)
#define CONDITION_WAIT_GETTIMEOFDAY_NOT_SUPPORTED_ON_SYSTEM (-2)
#define CONDITION_WAIT_TIME_OUT (1)
#define CONDITION_WAIT_ERROR (-4)
#define CONDITION_SIGNAL_SUCCESS (0)
#define CONDITION_BROADCAST_SUCCESS (0)

#endif
