/*
 * mutex.h
 *
 *  Created on: Oct 9, 2013
 *      Author: idor
 */

#ifndef MUTEX_H_
#define MUTEX_H_

#include <pthread.h>

/*!
 * This class implements a mutex based on pthread_mutex_t.
 */
class Mutex
{
public:
	/*!
	 * FAST - Normal mutex
	 * RECURSIVE - Recursive mutex
	 *
	 */
	typedef enum {
		FAST, RECURSIVE
	} Type;

public:
	/*!
	 * @brief The default constructor
	 *
	 * The default mutex type is FAST.
	 */
	Mutex();
	/*!
	 * @brief Constructor
	 * @param [in] mutex The mutex type (default is FAST)
	 */
	Mutex(Mutex::Type);
	/*!
	 * @brief Destructor
	 */
	virtual ~Mutex();

	/*!
	 * @brief Locks a mutex object
	 * @return Zero for success, nonzero for failure
	 */
	int lock();
	/*!
	 * @brief Unlocks a mutex object
	 * @return Zero for success, nonzero for failure
	 */
	int unlock();

	/*!
	 * @brief Gets a pthread_mutex_t member object
	 * @return The pthread_mutex_t object
	 */
	pthread_mutex_t &getMutex();

private:
	pthread_mutex_t _mutex;
};

#endif /* MUTEX_H_ */
