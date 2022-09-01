
#include <inc/lib.h>

// malloc()
//	This function use NEXT FIT strategy to allocate space in heap
//  with the given size and return void pointer to the start of the allocated space

//	To do this, we need to switch to the kernel, allocate the required space
//	in Page File then switch back to the user again.
//
//	We can use sys_allocateMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls allocateMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the allocateMem function is empty, make sure to implement it.


//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

struct UheapStrInformation{
	uint32 UheapStrInfo_StartAddress;
	uint32 UheapStrInfo_Size;
}
UheapStrInformation_Arr[(USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE];

uint32 FirstFreeAddressInUheap = USER_HEAP_START;
void* GetNextFitFrame(uint32);
void* GetBestFitFrame(uint32);

void* malloc(uint32 size)
{
	//TODO: [PROJECT 2022 - [9] User Heap malloc()] [User Side]
	// Write your code here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");

	// Steps:
	//	1) Implement NEXT FIT strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)
	//	2) if no suitable space found, return NULL
	//	 Else,
	//	3) Call sys_allocateMem to invoke the Kernel for allocation
	// 	4) Return pointer containing the virtual address of allocated space,
	//

	//This function should find the space of the required range
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyNEXTFIT() and
	//sys_isUHeapPlacementStrategyBESTFIT() for the bonus
	//to check the current strategy

	uint32 UH_NewUsedStartFrame, UH_UsedSizeFrameTemp;
	size = ROUNDUP(size, PAGE_SIZE);

	if (sys_isUHeapPlacementStrategyNEXTFIT()) {
		UH_NewUsedStartFrame = (uint32)GetNextFitFrame(size);
	}
	if (sys_isUHeapPlacementStrategyBESTFIT()) {
		UH_NewUsedStartFrame = (uint32)GetBestFitFrame(size);
	}
	if (!UH_NewUsedStartFrame) {
		return 0;
	}

	sys_allocateMem(UH_NewUsedStartFrame, size);
	uint32 UH_UsedStartFrameTemp = UH_NewUsedStartFrame;

	for (UH_UsedSizeFrameTemp = size; UH_UsedSizeFrameTemp  > 0; UH_UsedStartFrameTemp += PAGE_SIZE){
		UheapStrInformation_Arr[(UH_UsedStartFrameTemp - USER_HEAP_START) / PAGE_SIZE].UheapStrInfo_StartAddress = UH_NewUsedStartFrame;
		UheapStrInformation_Arr[(UH_UsedStartFrameTemp - USER_HEAP_START) / PAGE_SIZE].UheapStrInfo_Size = size;
		UH_UsedSizeFrameTemp  = UH_UsedSizeFrameTemp - PAGE_SIZE;
	}

	FirstFreeAddressInUheap = UH_UsedStartFrameTemp;
	return (void*)UH_NewUsedStartFrame;
}

void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	panic("smalloc() is not required ..!!");
	return NULL;
}

void* sget(int32 ownerEnvID, char *sharedVarName)
{
	panic("sget() is not required ..!!");
	return 0;
}

// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from page file and main memory then switch back to the user again.
//
//	We can use sys_freeMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls freeMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the freeMem function is empty, make sure to implement it.

void free(void* virtual_address)
{
	//TODO: [PROJECT 2022 - [11] User Heap free()] [User Side]
	// Write your code here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");

	//you shold get the size of the given allocation using its address
	//you need to call sys_freeMem()
	//refer to the project presentation and documentation for details

	uint32 UH_NewUsedVirtualAddress  = (uint32)virtual_address;
	uint32 UH_NewUsedSize = UheapStrInformation_Arr[((uint32)virtual_address - USER_HEAP_START) / PAGE_SIZE].UheapStrInfo_Size;

	sys_freeMem(UH_NewUsedVirtualAddress , UH_NewUsedSize);

	while (UH_NewUsedSize){
		UheapStrInformation_Arr[(UH_NewUsedVirtualAddress  - USER_HEAP_START) / PAGE_SIZE].UheapStrInfo_StartAddress = UH_NewUsedVirtualAddress ;
		UheapStrInformation_Arr[(UH_NewUsedVirtualAddress  - USER_HEAP_START) / PAGE_SIZE].UheapStrInfo_Size = 0;
		UH_NewUsedVirtualAddress  = UH_NewUsedVirtualAddress + PAGE_SIZE;
		UH_NewUsedSize = UH_NewUsedSize - PAGE_SIZE;
	}
}


void sfree(void* virtual_address)
{
	panic("sfree() is not requried ..!!");
}


//===============
// [2] realloc():
//===============

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_moveMem(uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		which switches to the kernel mode, calls moveMem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		in "memory_manager.c", then switch back to the user mode here
//	the moveMem function is empty, make sure to implement it.

void *realloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2022 - BONUS3] User Heap Realloc [User Side]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");

	return NULL;
}

void *GetNextFitFrame(uint32 size){
	uint32 UH_NewUsedStartAddress = FirstFreeAddressInUheap, 	 UH_NewUsedEndAddress = FirstFreeAddressInUheap;;
	uint32 UH_NewUsedFreeSize  = 0;

	while (UH_NewUsedStartAddress  < USER_HEAP_MAX) {
		if (!UheapStrInformation_Arr[(UH_NewUsedStartAddress  - USER_HEAP_START) / PAGE_SIZE].UheapStrInfo_Size) {
			UH_NewUsedFreeSize  = UH_NewUsedFreeSize + PAGE_SIZE;
		}

		else {
			UH_NewUsedFreeSize  = 0;
			if (UH_NewUsedEndAddress == FirstFreeAddressInUheap) {
				UH_NewUsedEndAddress = UH_NewUsedStartAddress ;
			}
		}
		UH_NewUsedStartAddress  = UH_NewUsedStartAddress + PAGE_SIZE;
		if (size == UH_NewUsedFreeSize){
			UH_NewUsedStartAddress  -= size;
			break;
		}
	}
	if (size != UH_NewUsedFreeSize){
		UH_NewUsedStartAddress  = USER_HEAP_START;
		UH_NewUsedFreeSize  = 0;
		while (UH_NewUsedStartAddress  < UH_NewUsedEndAddress) {
			if (!UheapStrInformation_Arr[(UH_NewUsedStartAddress  - USER_HEAP_START) / PAGE_SIZE].UheapStrInfo_Size) {
				UH_NewUsedFreeSize  = UH_NewUsedFreeSize + PAGE_SIZE;
			}
			else UH_NewUsedFreeSize  = 0;
			UH_NewUsedStartAddress  = UH_NewUsedStartAddress + PAGE_SIZE;
			if (UH_NewUsedFreeSize  == size) {
				UH_NewUsedStartAddress = UH_NewUsedStartAddress - size;
				break;
			}
		}
	}
	if (size != UH_NewUsedFreeSize) {
		return 0;
	}
	return (void*)UH_NewUsedStartAddress ;
}
void *GetBestFitFrame(uint32 size){
	uint32 UH_NewUsedStartAddress = USER_HEAP_START, UH_NewMinimumSizeOfAddress  = USER_HEAP_MAX - USER_HEAP_START + 1;
	uint32 UH_NewUsedFreeSize  = 0;

	for (; UH_NewUsedStartAddress < USER_HEAP_MAX; UH_NewUsedStartAddress += PAGE_SIZE) {
		if (!UheapStrInformation_Arr[(UH_NewUsedStartAddress - USER_HEAP_START) / PAGE_SIZE].UheapStrInfo_Size) {
			UH_NewUsedFreeSize  = UH_NewUsedFreeSize + PAGE_SIZE;
			if (UH_NewUsedStartAddress + PAGE_SIZE == USER_HEAP_MAX && UH_NewUsedFreeSize  >= size && UH_NewUsedFreeSize  < UH_NewMinimumSizeOfAddress) {
				UH_NewMinimumSizeOfAddress  = UH_NewUsedFreeSize;
			}
		}
		else {
			if (UH_NewUsedFreeSize  >= size && UH_NewUsedFreeSize  < UH_NewMinimumSizeOfAddress) {
				UH_NewMinimumSizeOfAddress  = UH_NewUsedFreeSize ;
			}
			UH_NewUsedFreeSize  = 0;
		}
	}

	if (UH_NewMinimumSizeOfAddress  == USER_HEAP_MAX - USER_HEAP_START + 1) {
		return 0;
	}

	UH_NewUsedFreeSize  = 0;
	UH_NewUsedStartAddress = USER_HEAP_START;
	for (; UH_NewUsedStartAddress < USER_HEAP_MAX; UH_NewUsedStartAddress += PAGE_SIZE) {
		if (!UheapStrInformation_Arr[(UH_NewUsedStartAddress - USER_HEAP_START) / PAGE_SIZE].UheapStrInfo_Size) {
			UH_NewUsedFreeSize  += PAGE_SIZE;
			if (UH_NewUsedStartAddress + PAGE_SIZE == USER_HEAP_MAX && UH_NewUsedFreeSize  == UH_NewMinimumSizeOfAddress ) {
				UH_NewUsedStartAddress -= UH_NewMinimumSizeOfAddress  - PAGE_SIZE;
				break;
			}
		}
		else {
			if (UH_NewMinimumSizeOfAddress == UH_NewUsedFreeSize) {
				UH_NewUsedStartAddress = UH_NewUsedStartAddress - UH_NewMinimumSizeOfAddress;
				break;
			}
			UH_NewUsedFreeSize  = 0;
		}
	}
	return (void*)UH_NewUsedStartAddress;
}

