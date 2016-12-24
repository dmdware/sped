#include "mempool.h"
#include "../platform.h"
#include "../utils.h"

MemPool::MemPool()
{
	m_pMemBlock = NULL;
	m_pAllocatedMemBlock = NULL;
	m_pFreeMemBlock = NULL;
}

/*==============================================================================
MemPool:
Constructor of this class. It allocate memory block from system and create
a static double linked std::list to manage all memory unit.

Parameters:
[in]ulUnitNum
The number of unit which is a part of memory block.

[in]ulUnitSize
The size of unit.
//=============================================================================
*/
void MemPool::allocsys(int nunits, int unitsz)
{
	freesysmem();

	m_ulBlockSize = nunits * (unitsz + sizeof(struct MemUnit));
	m_ulUnitSize = unitsz;

	m_pMemBlock = malloc(m_ulBlockSize);			//Allocate a memory block.

	if(!m_pMemBlock)
		OutOfMem(__FILE__, __LINE__);

	resetunits();
}

/*==============================================================================
~MemPool():
Destructor of this class. Its task is to free memory block.
//=============================================================================
*/
MemPool::~MemPool()
{
	freesysmem();
}


/*==============================================================================
Alloc:
To allocate a memory unit. If memory pool can`t provide proper memory unit,
will call system function.

Parameters:
[in]ulSize
Memory unit size.

[in]bUseMemPool
Whether use memory pool.

Return Values:
Return a pointer to a memory unit.
//=============================================================================
*/
void* MemPool::alloc()
{
	if( m_pMemBlock == NULL || m_pFreeMemBlock == NULL)
	{
#if 0
		return malloc(ulSize);
#else
		return NULL;
#endif
	}

	//Now FreeList isn`t empty
	struct MemUnit *pCurUnit = m_pFreeMemBlock;

	m_pFreeMemBlock = pCurUnit->pNext;			//Get a unit from free linkedlist.
	if(NULL != m_pFreeMemBlock)
	{
		m_pFreeMemBlock->pPrev = NULL;
	}

	pCurUnit->pNext = m_pAllocatedMemBlock;

	if(NULL != m_pAllocatedMemBlock)
	{
		m_pAllocatedMemBlock->pPrev = pCurUnit;
	}
	m_pAllocatedMemBlock = pCurUnit;

	return (void *)((char *)pCurUnit + sizeof(struct MemUnit) );
}


/*==============================================================================
Free:
To free a memory unit. If the pointer of parameter point to a memory unit,
then insert it to "Free linked std::list". Otherwise, call system function "free".

Parameters:
[in]p
It point to a memory unit and prepare to free it.

Return Values:
none
//=============================================================================
*/
void MemPool::freeunit( void* p )
{
#if 0
	if(m_pMemBlock<p && p<(void *)((char *)m_pMemBlock + m_ulBlockSize) )
	{
#endif
		struct MemUnit *pCurUnit = (struct MemUnit *)((char *)p - sizeof(struct MemUnit) );

		m_pAllocatedMemBlock = pCurUnit->pNext;
		if(NULL != m_pAllocatedMemBlock)
		{
			m_pAllocatedMemBlock->pPrev = NULL;
		}

		pCurUnit->pNext = m_pFreeMemBlock;
		if(NULL != m_pFreeMemBlock)
		{
			m_pFreeMemBlock->pPrev = pCurUnit;
		}

		m_pFreeMemBlock = pCurUnit;
#if 0
	}
	else
	{
		free(p);
	}
#endif
}

void MemPool::resetunits()
{
	if(!m_pMemBlock)
		return;

	for(int i=0; i<m_nUnits; i++)	//Link all mem unit .
	{
		struct MemUnit *pCurUnit = (struct MemUnit *)( (char *)m_pMemBlock + i*(m_ulUnitSize + sizeof(struct MemUnit)) );

		pCurUnit->pPrev = NULL;
		pCurUnit->pNext = m_pFreeMemBlock;		//Insert the new unit at head.

		if(NULL != m_pFreeMemBlock)
		{
			m_pFreeMemBlock->pPrev = pCurUnit;
		}
		m_pFreeMemBlock = pCurUnit;
	}
}

void MemPool::freesysmem()
{
	if(m_pMemBlock)
	{
		free(m_pMemBlock);
		m_pMemBlock = NULL;
	}

	m_pMemBlock = NULL;
	m_pAllocatedMemBlock = NULL;
	m_pFreeMemBlock = NULL;
}
