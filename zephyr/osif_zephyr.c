/*
 * Copyright (c) 2026 YI JIN <jinyi_2001@foxmail.com>
 * SPDX-License-Identifier: Apache-2.0
 *
 * Zephyr OSIF shim for YTMicro vendor SDK.
 *
 * The vendor osif.h defines semaphore_t as volatile uint8_t (bare-metal ABI),
 * so a struct k_sem cannot be stored directly in the vendor state structs.
 * We maintain a fixed-size pool that maps each semaphore_t* address to its
 * corresponding struct k_sem.
 *
 * Pool size: vendor drivers allocate at most 1-2 semaphores per driver
 * instance.  FlexCAN allocates one per active mailbox (up to 32 each).
 * 64 entries covers two CAN instances fully loaded plus all other peripherals.
 *
 * Thread-safety: pool modifications (create/destroy) are protected by a
 * spinlock. osif_sem_find() is called both from thread context (SemaWait)
 * and from ISR context (SemaPost). On Cortex-M the spinlock disables IRQs,
 * so there is no race between ISR searches and thread-context pool mutations.
 */

#include <zephyr/kernel.h>
#include "osif.h"
#include "status.h"

#define OSIF_SEM_POOL_SIZE 64

struct osif_sem_entry {
	const semaphore_t *key;
	struct k_sem sem;
};

static struct osif_sem_entry sem_pool[OSIF_SEM_POOL_SIZE];
static struct k_spinlock pool_lock;

static struct osif_sem_entry *find_entry(const semaphore_t *pSem)
{
	for (int i = 0; i < OSIF_SEM_POOL_SIZE; i++) {
		if (sem_pool[i].key == pSem) {
			return &sem_pool[i];
		}
	}
	return NULL;
}

/* ---- time ---------------------------------------------------------------- */

void OSIF_TimeDelay(const uint32_t delay)
{
	k_msleep((int32_t)delay);
}

uint32_t OSIF_GetMilliseconds(void)
{
	return (uint32_t)k_uptime_get();
}

/* ---- mutex (vendor driver layer does not use mutex APIs) ----------------- */

status_t OSIF_MutexCreate(mutex_t * const pMutex)
{
	(void)pMutex;
	return STATUS_SUCCESS;
}

status_t OSIF_MutexDestroy(const mutex_t * const pMutex)
{
	(void)pMutex;
	return STATUS_SUCCESS;
}

status_t OSIF_MutexLock(const mutex_t * const pMutex, const uint32_t timeout)
{
	(void)pMutex;
	(void)timeout;
	return STATUS_SUCCESS;
}

status_t OSIF_MutexUnlock(const mutex_t * const pMutex)
{
	(void)pMutex;
	return STATUS_SUCCESS;
}

/* ---- semaphore ------------------------------------------------------------ */

status_t OSIF_SemaCreate(semaphore_t * const pSem, const uint8_t initValue)
{
	k_spinlock_key_t key = k_spin_lock(&pool_lock);

	/* Re-initialise if already registered (driver re-init path). */
	struct osif_sem_entry *e = find_entry(pSem);

	if (e == NULL) {
		for (int i = 0; i < OSIF_SEM_POOL_SIZE; i++) {
			if (sem_pool[i].key == NULL) {
				sem_pool[i].key = pSem;
				e = &sem_pool[i];
				break;
			}
		}
	}

	k_spin_unlock(&pool_lock, key);

	if (e == NULL) {
		/* Pool exhausted — increase OSIF_SEM_POOL_SIZE */
		__ASSERT(false, "OSIF sem pool exhausted");
		return STATUS_ERROR;
	}

	k_sem_init(&e->sem, initValue, 255U);
	return STATUS_SUCCESS;
}

status_t OSIF_SemaDestroy(const semaphore_t * const pSem)
{
	k_spinlock_key_t key = k_spin_lock(&pool_lock);

	for (int i = 0; i < OSIF_SEM_POOL_SIZE; i++) {
		if (sem_pool[i].key == pSem) {
			sem_pool[i].key = NULL;
			break;
		}
	}

	k_spin_unlock(&pool_lock, key);
	return STATUS_SUCCESS;
}

status_t OSIF_SemaPost(semaphore_t * const pSem)
{
	struct osif_sem_entry *e = find_entry(pSem);

	if (e == NULL) {
		return STATUS_ERROR;
	}
	k_sem_give(&e->sem);
	return STATUS_SUCCESS;
}

status_t OSIF_SemaWait(semaphore_t * const pSem, const uint32_t timeout)
{
	struct osif_sem_entry *e = find_entry(pSem);

	if (e == NULL) {
		return STATUS_ERROR;
	}

	k_timeout_t t;

	if (timeout == 0U) {
		t = K_NO_WAIT;
	} else if (timeout == OSIF_WAIT_FOREVER) {
		t = K_FOREVER;
	} else {
		t = K_MSEC(timeout);
	}

	return (k_sem_take(&e->sem, t) == 0) ? STATUS_SUCCESS : STATUS_TIMEOUT;
}
