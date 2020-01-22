/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *                                        
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/


#define _OSDEP_SERVICE_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <recv_osdep.h>
#include <linux/vmalloc.h>


#define RT_TAG	'1178'

#ifdef MEMORY_LEAK_DEBUG
#ifdef PLATFORM_LINUX
#include <asm/atomic.h>
atomic_t _malloc_cnt = ATOMIC_INIT(0);
atomic_t _malloc_size = ATOMIC_INIT(0);
#endif
#endif /* MEMORY_LEAK_DEBUG */


inline u8* _rtw_vmalloc(u32 sz)
{
	u8 	*pbuf;
#ifdef PLATFORM_LINUX	
	pbuf = vmalloc(sz);	
#endif	
	
#ifdef PLATFORM_WINDOWS
	NdisAllocateMemoryWithTag(&pbuf,sz, RT_TAG);	
#endif

	return pbuf;	
}

inline u8* _rtw_zvmalloc(u32 sz)
{
	u8 	*pbuf;
#ifdef PLATFORM_LINUX
	pbuf = _rtw_vmalloc(sz);
	if (pbuf != NULL)
		memset(pbuf, 0, sz);
#endif	
	
#ifdef PLATFORM_WINDOWS
	NdisAllocateMemoryWithTag(&pbuf,sz, RT_TAG);
	if (pbuf != NULL)
		NdisFillMemory(pbuf, sz, 0);
#endif

	return pbuf;	
	}

inline void _rtw_vmfree(u8 *pbuf, u32 sz)
{
#ifdef	PLATFORM_LINUX
	vfree(pbuf);
#endif	
	
#ifdef PLATFORM_WINDOWS
	NdisFreeMemory(pbuf,sz, 0);
#endif
}

u8* _rtw_malloc(u32 sz)
{

	u8 	*pbuf=NULL;

#ifdef PLATFORM_LINUX
#ifdef RTK_DMP_PLATFORM
	if(sz > 0x4000)
		pbuf = (u8 *)dvr_malloc(sz);
	else
#endif
		pbuf = kmalloc(sz, /*GFP_KERNEL*/GFP_ATOMIC);

#endif	
	
#ifdef PLATFORM_WINDOWS

	NdisAllocateMemoryWithTag(&pbuf,sz, RT_TAG);

#endif

#ifdef MEMORY_LEAK_DEBUG
#ifdef PLATFORM_LINUX
	if ( pbuf != NULL) {
		atomic_inc(&_malloc_cnt);
		atomic_add(sz, &_malloc_size);
	}
#endif
#endif /* MEMORY_LEAK_DEBUG */

	return pbuf;	
	
}


u8* _rtw_zmalloc(u32 sz)
{
	u8 	*pbuf = _rtw_malloc(sz);

	if (pbuf != NULL) {

#ifdef PLATFORM_LINUX
		memset(pbuf, 0, sz);
#endif	
	
#ifdef PLATFORM_WINDOWS
		NdisFillMemory(pbuf, sz, 0);
#endif

	}

	return pbuf;	
	
}

void	_rtw_mfree(u8 *pbuf, u32 sz)
{

#ifdef	PLATFORM_LINUX
#ifdef RTK_DMP_PLATFORM
	if(sz > 0x4000)
		dvr_free(pbuf);
	else
#endif
		kfree(pbuf);

#endif	
	
#ifdef PLATFORM_WINDOWS

	NdisFreeMemory(pbuf,sz, 0);

#endif
	
#ifdef MEMORY_LEAK_DEBUG
#ifdef PLATFORM_LINUX
	atomic_dec(&_malloc_cnt);
	atomic_sub(sz, &_malloc_size);
#endif
#endif /* MEMORY_LEAK_DEBUG */
	
}


#ifdef DBG_MEM_ALLOC
inline u8* dbg_rtw_vmalloc(u32 sz, const char *func, int line)
{
	DBG_871X("DBG_MEM_ALLOC %s:%d %s(%d)\n", func,  line, __FUNCTION__, (sz));
	return _rtw_vmalloc((sz));
}

inline u8* dbg_rtw_zvmalloc(u32 sz, const char *func, int line)
{
	DBG_871X("DBG_MEM_ALLOC %s:%d %s(%d)\n", func, line, __FUNCTION__, (sz)); 
	return _rtw_zvmalloc((sz)); 
}

inline void dbg_rtw_vmfree(u8 *pbuf, u32 sz, const char *func, int line)
{
	DBG_871X("DBG_MEM_ALLOC %s:%d %s(%p,%d)\n",  func, line, __FUNCTION__, (pbuf), (sz));
	_rtw_vmfree((pbuf), (sz)); 
}

inline u8* dbg_rtw_malloc(u32 sz, const char *func, int line) 
{
	if((sz)>4096) 
		DBG_871X("DBG_MEM_ALLOC !!!!!!!!!!!!!! %s:%d %s(%d)\n", func, line, __FUNCTION__, (sz)); 
	return _rtw_malloc((sz)); 
}

inline u8* dbg_rtw_zmalloc(u32 sz, const char *func, int line)
{
	if((sz)>4096)
		DBG_871X("DBG_MEM_ALLOC !!!!!!!!!!!!!! %s:%d %s(%d)\n", func, line, __FUNCTION__, (sz));
	return _rtw_zmalloc((sz));
}

inline void dbg_rtw_mfree(u8 *pbuf, u32 sz, const char *func, int line)
{
	if((sz)>4096)
		DBG_871X("DBG_MEM_ALLOC !!!!!!!!!!!!!! %s:%d %s(%p,%d)\n", func, line, __FUNCTION__, (pbuf), (sz));
	_rtw_mfree((pbuf), (sz));
}
#endif


void _memcpy(void* dst, void* src, u32 sz)
{

#ifdef PLATFORM_LINUX

	memcpy(dst, src, sz);

#endif	
	
#ifdef PLATFORM_WINDOWS

	NdisMoveMemory(dst, src, sz);

#endif

}

int	_memcmp(void *dst, void *src, u32 sz)
{

#ifdef PLATFORM_LINUX
//under Linux/GNU/GLibc, the return value of memcmp for two same mem. chunk is 0

	if (!(memcmp(dst, src, sz)))
		return _TRUE;
	else
		return _FALSE;
#endif


#ifdef PLATFORM_WINDOWS
//under Windows, the return value of NdisEqualMemory for two same mem. chunk is 1
	
	if (NdisEqualMemory (dst, src, sz))
		return _TRUE;
	else
		return _FALSE;

#endif	
	
	
	
}

void _memset(void *pbuf, int c, u32 sz)
{

#ifdef PLATFORM_LINUX

        memset(pbuf, c, sz);

#endif

#ifdef PLATFORM_WINDOWS
#if 0
	NdisZeroMemory(pbuf, sz);
	if (c != 0) memset(pbuf, c, sz);
#else
	NdisFillMemory(pbuf, sz, c);
#endif
#endif

}

void _init_listhead(_list *list)
{

#ifdef PLATFORM_LINUX

        INIT_LIST_HEAD(list);

#endif

#ifdef PLATFORM_WINDOWS

        NdisInitializeListHead(list);

#endif

}


/*
For the following list_xxx operations, 
caller must guarantee the atomic context.
Otherwise, there will be racing condition.
*/
u32	is_list_empty(_list *phead)
{

#ifdef PLATFORM_LINUX

	if (list_empty(phead))
		return _TRUE;
	else
		return _FALSE;

#endif
	

#ifdef PLATFORM_WINDOWS

	if (IsListEmpty(phead))
		return _TRUE;
	else
		return _FALSE;

#endif

	
}


void list_insert_tail(_list *plist, _list *phead)
{

#ifdef PLATFORM_LINUX	
	
	list_add_tail(plist, phead);
	
#endif
	
#ifdef PLATFORM_WINDOWS

  InsertTailList(phead, plist);

#endif		
	
}


/*

Caller must check if the list is empty before calling list_delete

*/


void _init_sema(_sema	*sema, int init_val)
{

#ifdef PLATFORM_LINUX

	sema_init(sema, init_val);

#endif

#ifdef PLATFORM_OS_XP

	KeInitializeSemaphore(sema, init_val,  SEMA_UPBND); // count=0;

#endif
	
#ifdef PLATFORM_OS_CE
	if(*sema == NULL)
		*sema = CreateSemaphore(NULL, init_val, SEMA_UPBND, NULL);
#endif

}

void _free_sema(_sema	*sema)
{

#ifdef PLATFORM_OS_CE
	CloseHandle(*sema);
#endif

}

void _up_sema(_sema	*sema)
{

#ifdef PLATFORM_LINUX

	up(sema);

#endif	

#ifdef PLATFORM_OS_XP

	KeReleaseSemaphore(sema, IO_NETWORK_INCREMENT, 1,  FALSE );

#endif

#ifdef PLATFORM_OS_CE
	ReleaseSemaphore(*sema,  1,  NULL );
#endif
}

u32 _down_sema(_sema *sema)
{

#ifdef PLATFORM_LINUX
	
	if (down_interruptible(sema))
		return _FAIL;
	else
		return _SUCCESS;

#endif    	

#ifdef PLATFORM_OS_XP

	if(STATUS_SUCCESS == KeWaitForSingleObject(sema, Executive, KernelMode, TRUE, NULL))
		return  _SUCCESS;
	else
		return _FAIL;
#endif

#ifdef PLATFORM_OS_CE
	if(WAIT_OBJECT_0 == WaitForSingleObject(*sema, INFINITE ))
		return _SUCCESS; 
	else
		return _FAIL;
#endif
}



void	_rtw_mutex_init(_mutex *pmutex)
{
#ifdef PLATFORM_LINUX

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
	mutex_init(pmutex);
#else
	init_MUTEX(pmutex);
#endif

#endif

#ifdef PLATFORM_OS_XP

	KeInitializeMutex(pmutex, 0);

#endif

#ifdef PLATFORM_OS_CE
	*pmutex =  CreateMutex( NULL, _FALSE, NULL);
#endif
}


void	_spinlock_init(_lock *plock)
{

#ifdef PLATFORM_LINUX

	spin_lock_init(plock);

#endif	
	
#ifdef PLATFORM_WINDOWS

	NdisAllocateSpinLock(plock);

#endif
	
}

void	_spinlock_free(_lock *plock)
{

	
#ifdef PLATFORM_WINDOWS

	NdisFreeSpinLock(plock);

#endif
	
}


void	_spinlock(_lock	*plock)
{

#ifdef PLATFORM_LINUX

	spin_lock(plock);

#endif
	
#ifdef PLATFORM_WINDOWS

	NdisAcquireSpinLock(plock);

#endif
	
}

void	_spinunlock(_lock *plock)
{

#ifdef PLATFORM_LINUX

	spin_unlock(plock);

#endif
	
#ifdef PLATFORM_WINDOWS

	NdisReleaseSpinLock(plock);

#endif
}


void	_spinlock_ex(_lock	*plock)
{

#ifdef PLATFORM_LINUX

	spin_lock(plock);

#endif
	
#ifdef PLATFORM_WINDOWS

	NdisDprAcquireSpinLock(plock);

#endif
	
}

void	_spinunlock_ex(_lock *plock)
{

#ifdef PLATFORM_LINUX

	spin_unlock(plock);

#endif
	
#ifdef PLATFORM_WINDOWS

	NdisDprReleaseSpinLock(plock);

#endif
}



void	_init_queue(_queue	*pqueue)
{

	_init_listhead(&(pqueue->queue));

	_spinlock_init(&(pqueue->lock));

}

u32	  _queue_empty(_queue	*pqueue)
{
	return (is_list_empty(&(pqueue->queue)));
}


u32 end_of_queue_search(_list *head, _list *plist)
{
	if (head == plist)
		return _TRUE;
	else
		return _FALSE;
}


u32	get_current_time(void)
{
	
#ifdef PLATFORM_LINUX
	return jiffies;
#endif	

#ifdef PLATFORM_WINDOWS
	LARGE_INTEGER	SystemTime;
	NdisGetCurrentSystemTime(&SystemTime);
	return (u32)(SystemTime.LowPart);// count of 100-nanosecond intervals 
#endif
}

inline u32 systime_to_ms(u32 systime)
{
#ifdef PLATFORM_LINUX
	return systime*1000/HZ;
#endif	
	
#ifdef PLATFORM_WINDOWS
	return systime /10000 ; 
#endif
}

// the input parameter start use the same unit as returned by get_current_time
inline s32 get_passing_time_ms(u32 start)
{
#ifdef PLATFORM_LINUX
	return systime_to_ms(jiffies-start);
#endif

#ifdef PLATFORM_WINDOWS
	LARGE_INTEGER	SystemTime;
	NdisGetCurrentSystemTime(&SystemTime);
	return systime_to_ms((u32)(SystemTime.LowPart) - start) ;
#endif
}

inline s32 get_time_interval_ms(u32 start, u32 end)
{
#ifdef PLATFORM_LINUX
	return systime_to_ms(end-start);
#endif
	
#ifdef PLATFORM_WINDOWS
	return systime_to_ms(end-start);
#endif
}
	

void sleep_schedulable(int ms)	
{

#ifdef PLATFORM_LINUX

    u32 delta;
    
    delta = (ms * HZ)/1000;//(ms)
    if (delta == 0) {
        delta = 1;// 1 ms
    }
    set_current_state(TASK_INTERRUPTIBLE);
    if (schedule_timeout(delta) != 0) {
        return ;
    }
    return;

#endif	
	
#ifdef PLATFORM_WINDOWS

	NdisMSleep(ms*1000); //(us)*1000=(ms)

#endif

}


void msleep_os(int ms)
{

#ifdef PLATFORM_LINUX

  	msleep((unsigned int)ms);

#endif	
	
#ifdef PLATFORM_WINDOWS

	NdisMSleep(ms*1000); //(us)*1000=(ms)

#endif


}
void usleep_os(int us)
{

#ifdef PLATFORM_LINUX
  	
      // msleep((unsigned int)us);
      if ( 1 < (us/1000) )
                msleep(1);
      else
		msleep( (us/1000) + 1);

#endif	
	
#ifdef PLATFORM_WINDOWS

	NdisMSleep(us); //(us)

#endif


}

void mdelay_os(int ms)
{

#ifdef PLATFORM_LINUX

   	mdelay((unsigned long)ms); 

#endif	
	
#ifdef PLATFORM_WINDOWS

	NdisStallExecution(ms*1000); //(us)*1000=(ms)

#endif


}
void udelay_os(int us)
{

#ifdef PLATFORM_LINUX

      udelay((unsigned long)us); 

#endif	
	
#ifdef PLATFORM_WINDOWS

	NdisStallExecution(us); //(us)

#endif

}

#define RTW_SUSPEND_LOCK_NAME "rtw_wifi"

#ifdef CONFIG_WAKELOCK
static struct wake_lock rtw_suspend_lock;
#elif defined(CONFIG_ANDROID_POWER)
static android_suspend_lock_t rtw_suspend_lock ={
	.name = RTW_SUSPEND_LOCK_NAME
};
#endif

inline void rtw_suspend_lock_init()
{
	#if  defined(CONFIG_WAKELOCK) || defined(CONFIG_ANDROID_POWER)
	DBG_871X("##########%s ###########\n", __FUNCTION__);
	#endif

	#ifdef CONFIG_WAKELOCK
	wake_lock_init(&rtw_suspend_lock, WAKE_LOCK_SUSPEND, RTW_SUSPEND_LOCK_NAME);
	#elif defined(CONFIG_ANDROID_POWER)
	android_init_suspend_lock(&rtw_suspend_lock);
	#endif
	
}

inline void rtw_suspend_lock_uninit()
{

	#if  defined(CONFIG_WAKELOCK) || defined(CONFIG_ANDROID_POWER)
	DBG_871X("##########%s###########\n", __FUNCTION__);
	if(rtw_suspend_lock.link.next == LIST_POISON1 || rtw_suspend_lock.link.prev == LIST_POISON2) {
		DBG_871X("##########%s########### list poison!!\n", __FUNCTION__);
		return;	
	}
	#endif
	
	#ifdef CONFIG_WAKELOCK
	wake_lock_destroy(&rtw_suspend_lock);
	#elif defined(CONFIG_ANDROID_POWER)
	android_uninit_suspend_lock(&rtw_suspend_lock);
	#endif
}


inline void rtw_lock_suspend()
{

	#if  defined(CONFIG_WAKELOCK) || defined(CONFIG_ANDROID_POWER)
	//DBG_871X("##########%s###########\n", __FUNCTION__);
	if(rtw_suspend_lock.link.next == LIST_POISON1 || rtw_suspend_lock.link.prev == LIST_POISON2) {
		DBG_871X("##########%s########### list poison!!\n", __FUNCTION__);
		return;	
	}
	#endif
	
	#ifdef CONFIG_WAKELOCK
	wake_lock(&rtw_suspend_lock);
	#elif defined(CONFIG_ANDROID_POWER)
	android_lock_suspend(&rtw_suspend_lock);
	#endif
}

inline void rtw_unlock_suspend()
{
	#if  defined(CONFIG_WAKELOCK) || defined(CONFIG_ANDROID_POWER)
	//DBG_871X("##########%s###########\n", __FUNCTION__);
	if(rtw_suspend_lock.link.next == LIST_POISON1 || rtw_suspend_lock.link.prev == LIST_POISON2) {
		DBG_871X("##########%s########### list poison!!\n", __FUNCTION__);
		return;	
	}
	#endif
	
	#ifdef CONFIG_WAKELOCK
	wake_unlock(&rtw_suspend_lock);
	#elif defined(CONFIG_ANDROID_POWER)
	android_unlock_suspend(&rtw_suspend_lock);
	#endif
}


inline void ATOMIC_SET(ATOMIC_T *v, int i)
{
	#ifdef PLATFORM_LINUX
	atomic_set(v,i);
	#elif defined(PLATFORM_WINDOWS)
	*v=i;// other choice????
	#endif
}

inline int ATOMIC_READ(ATOMIC_T *v)
{
	#ifdef PLATFORM_LINUX
	return atomic_read(v);
	#elif defined(PLATFORM_WINDOWS)
	return *v; // other choice????
	#endif
}

inline void ATOMIC_ADD(ATOMIC_T *v, int i)
{
	#ifdef PLATFORM_LINUX
	atomic_add(i,v);
	#elif defined(PLATFORM_WINDOWS)
	InterlockedAdd(v,i);
	#endif
}
inline void ATOMIC_SUB(ATOMIC_T *v, int i)
{
	#ifdef PLATFORM_LINUX
	atomic_sub(i,v);
	#elif defined(PLATFORM_WINDOWS)
	InterlockedAdd(v,-i);
	#endif
}

inline void ATOMIC_INC(ATOMIC_T *v)
{
	#ifdef PLATFORM_LINUX
	atomic_inc(v);
	#elif defined(PLATFORM_WINDOWS)
	InterlockedIncrement(v);
	#endif
}

inline void ATOMIC_DEC(ATOMIC_T *v)
{
	#ifdef PLATFORM_LINUX
	atomic_dec(v);
	#elif defined(PLATFORM_WINDOWS)
	InterlockedDecrement(v);
	#endif
}

inline int ATOMIC_ADD_RETURN(ATOMIC_T *v, int i)
{
	#ifdef PLATFORM_LINUX
	return atomic_add_return(i,v);
	#elif defined(PLATFORM_WINDOWS)
	return InterlockedAdd(v,i);
	#endif
}

inline int ATOMIC_SUB_RETURN(ATOMIC_T *v, int i)
{
	#ifdef PLATFORM_LINUX
	return atomic_sub_return(i,v);
	#elif defined(PLATFORM_WINDOWS)
	return InterlockedAdd(v,-i);
	#endif
}

inline int ATOMIC_INC_RETURN(ATOMIC_T *v)
{
	#ifdef PLATFORM_LINUX
	return atomic_inc_return(v);
	#elif defined(PLATFORM_WINDOWS)
	return InterlockedIncrement(v);
	#endif
}

inline int ATOMIC_DEC_RETURN(ATOMIC_T *v)
{
	#ifdef PLATFORM_LINUX
	return atomic_dec_return(v);
	#elif defined(PLATFORM_WINDOWS)
	return InterlockedDecrement(v);
	#endif
}

