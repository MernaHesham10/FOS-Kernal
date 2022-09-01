#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>

//2022: NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)

struct KheapStrInformation{
 	uint32 KheapStrInfo_StartAddress;
 	uint32 KheapStrInfo_Size;
}
KheapStrInformation_Arr[(KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE];

uint32 KH_FirstFreeAddressInKheap = KERNEL_HEAP_START;
uint32 KH_ConvertFromPhysicalAddToVirtualAdd[1024 * 1024];
void *GetNextFitFrame(uint32 size);
void *GetBestFitFrame(uint32 size);

void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT 2022 - [1] Kernel Heap] kmalloc()
	// Write your code here, remove the panic and write your code
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");

	//NOTE: Allocation using NEXTFIT strategy
	//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
	//refer to the project presentation and documentation for details

	//TODO: [PROJECT 2022 - BONUS1] Implement a Kernel allocation strategy
	// Instead of the Next allocation/deallocation, implement
	// BEST FIT strategy
	// use "isKHeapPlacementStrategyBESTFIT() ..."
	// and "isKHeapPlacementStrategyNEXTFIT() ..."
	//functions to check the current strategy
	//change this "return" according to your answer

	size = ROUNDUP(size, PAGE_SIZE);
	uint32 KH_NewUsedStartFrame;

	if(isKHeapPlacementStrategyBESTFIT()){
		KH_NewUsedStartFrame = (uint32)GetBestFitFrame(size);
	}

	if(isKHeapPlacementStrategyNEXTFIT()){
		KH_NewUsedStartFrame = (uint32)GetNextFitFrame(size);
	}

	if (!KH_NewUsedStartFrame){
		return 0;
	}

	uint32 KH_UsedStartFrameTemp = KH_NewUsedStartFrame;

	for (uint32 KHusedSizeFrameTemp = 0; KHusedSizeFrameTemp < size; KHusedSizeFrameTemp += PAGE_SIZE) {

		struct Frame_Info *KHpointerToFrameInfo = 0;
		allocate_frame(&KHpointerToFrameInfo);
		map_frame(ptr_page_directory, KHpointerToFrameInfo, (void*)KH_UsedStartFrameTemp, PERM_PRESENT | PERM_WRITEABLE);
		KheapStrInformation_Arr[(KH_UsedStartFrameTemp - KERNEL_HEAP_START) / PAGE_SIZE].KheapStrInfo_StartAddress = KH_NewUsedStartFrame;
		KheapStrInformation_Arr[(KH_UsedStartFrameTemp - KERNEL_HEAP_START) / PAGE_SIZE].KheapStrInfo_Size = size;
		KH_ConvertFromPhysicalAddToVirtualAdd[kheap_physical_address(KH_UsedStartFrameTemp) / PAGE_SIZE] = KH_UsedStartFrameTemp;
		KH_UsedStartFrameTemp = KH_UsedStartFrameTemp + PAGE_SIZE;
	}

	KH_FirstFreeAddressInKheap = KH_UsedStartFrameTemp;
	return (void*)KH_NewUsedStartFrame;
}


void kfree(void* virtual_address)
{
	//TODO: [PROJECT 2022 - [2] Kernel Heap] kfree()
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");

	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details

	uint32 KH_NewUsedStartFrame = (uint32)virtual_address;
	KH_NewUsedStartFrame -= KH_NewUsedStartFrame % PAGE_SIZE;
	virtual_address = (void*)KH_NewUsedStartFrame;

	/*if (KH_NewUsedStartFrame >= KERNEL_HEAP_MAX || KH_NewUsedStartFrame < KERNEL_HEAP_START) {
		return;
	}*/

	while (KH_NewUsedStartFrame < KERNEL_HEAP_MAX) {

		if (KheapStrInformation_Arr[(KH_NewUsedStartFrame - KERNEL_HEAP_START) / PAGE_SIZE].KheapStrInfo_Size){

			if (KheapStrInformation_Arr[(KH_NewUsedStartFrame - KERNEL_HEAP_START) / PAGE_SIZE].KheapStrInfo_StartAddress == (uint32)virtual_address){

				KH_ConvertFromPhysicalAddToVirtualAdd[kheap_physical_address(KH_NewUsedStartFrame) / PAGE_SIZE] = 0;
				unmap_frame(ptr_page_directory, (void*)KH_NewUsedStartFrame);
				KheapStrInformation_Arr[(KH_NewUsedStartFrame - KERNEL_HEAP_START) / PAGE_SIZE].KheapStrInfo_StartAddress = KH_NewUsedStartFrame;
				KheapStrInformation_Arr[(KH_NewUsedStartFrame - KERNEL_HEAP_START) / PAGE_SIZE].KheapStrInfo_Size = 0;
				KH_NewUsedStartFrame = KH_NewUsedStartFrame + PAGE_SIZE;
			}
			else {
				break;
			}
		}
		else {
			break;
		}
	}
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT 2022 - [3] Kernel Heap] kheap_virtual_address()
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");

	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details

	//change this "return" according to your answer

	return KH_ConvertFromPhysicalAddToVirtualAdd[physical_address / PAGE_SIZE];
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT 2022 - [4] Kernel Heap] kheap_physical_address()
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details

	//change this "return" according to your answer
	uint32 *KH_usedPointerToPageTable = NULL;

	get_page_table(ptr_page_directory, (void*)virtual_address, &KH_usedPointerToPageTable);

	if (KH_usedPointerToPageTable) {
		return (KH_usedPointerToPageTable [PTX(virtual_address)] >> 12) * PAGE_SIZE;
	}
	return 0;
}


void *GetNextFitFrame(uint32 size){

	uint32 KH_NewUsedStartAddress = KH_FirstFreeAddressInKheap, KH_NewUsedEndAddress = KH_FirstFreeAddressInKheap;
	uint32 KH_NewUsedFreeSize = 0;

	while (KH_NewUsedStartAddress < KERNEL_HEAP_MAX) {

		if (!KheapStrInformation_Arr[(KH_NewUsedStartAddress - KERNEL_HEAP_START) / PAGE_SIZE].KheapStrInfo_Size) {
			KH_NewUsedFreeSize = KH_NewUsedFreeSize + PAGE_SIZE;
		}
		else {
			KH_NewUsedFreeSize = 0;
			if (KH_FirstFreeAddressInKheap == KH_NewUsedEndAddress) {
				KH_NewUsedEndAddress = KH_NewUsedStartAddress;
			}
		}
		KH_NewUsedStartAddress += PAGE_SIZE;
		if (size == KH_NewUsedFreeSize) {
			KH_NewUsedStartAddress = KH_NewUsedStartAddress - size;
			break;
		}
	}

	if (size != KH_NewUsedFreeSize) {
		KH_NewUsedStartAddress = KERNEL_HEAP_START;
		KH_NewUsedFreeSize = 0;

		while (KH_NewUsedStartAddress < KH_NewUsedEndAddress){
			if (!KheapStrInformation_Arr[(KH_NewUsedStartAddress - KERNEL_HEAP_START) / PAGE_SIZE].KheapStrInfo_Size) {
				KH_NewUsedFreeSize = KH_NewUsedFreeSize + PAGE_SIZE;
			}
			else {
				KH_NewUsedFreeSize = 0;
			}
			KH_NewUsedStartAddress = KH_NewUsedStartAddress + PAGE_SIZE;
			if (size == KH_NewUsedFreeSize) {
				KH_NewUsedStartAddress = KH_NewUsedStartAddress - size;
				break;
			}
		}
	}
	if (KH_NewUsedFreeSize != size) {
		return 0;
	}

	return (void*)KH_NewUsedStartAddress;
}


void *GetBestFitFrame(uint32 size) {
	uint32 KH_NewMinimumSizeOfAddress = KERNEL_HEAP_MAX - KERNEL_HEAP_START + 1, KH_NewUsedFreeSize = 0;
	uint32 KH_NewUsedStartAddress = KERNEL_HEAP_START;

	for (; KH_NewUsedStartAddress < KERNEL_HEAP_MAX; KH_NewUsedStartAddress += PAGE_SIZE){
		if (!KheapStrInformation_Arr[(KH_NewUsedStartAddress - KERNEL_HEAP_START) / PAGE_SIZE].KheapStrInfo_Size) {
			KH_NewUsedFreeSize = KH_NewUsedFreeSize + PAGE_SIZE;

			if (KH_NewUsedStartAddress + PAGE_SIZE == KERNEL_HEAP_MAX && KH_NewUsedFreeSize >= size && KH_NewUsedFreeSize < KH_NewMinimumSizeOfAddress) {
				KH_NewMinimumSizeOfAddress = KH_NewUsedFreeSize;
			}
		}
		else {
			if (KH_NewUsedFreeSize >= size && KH_NewUsedFreeSize < KH_NewMinimumSizeOfAddress) {
				KH_NewMinimumSizeOfAddress = KH_NewUsedFreeSize;
			}
			KH_NewUsedFreeSize = 0;
		}

	}

	if (KH_NewMinimumSizeOfAddress == KERNEL_HEAP_MAX - KERNEL_HEAP_START + 1) {
		return 0;
	}

	KH_NewUsedFreeSize = 0;
	KH_NewUsedStartAddress = KERNEL_HEAP_START;
	for (;KH_NewUsedStartAddress < KERNEL_HEAP_MAX; KH_NewUsedStartAddress += PAGE_SIZE){

		if (!KheapStrInformation_Arr[(KH_NewUsedStartAddress - KERNEL_HEAP_START) / PAGE_SIZE].KheapStrInfo_Size){
			KH_NewUsedFreeSize = KH_NewUsedFreeSize + PAGE_SIZE;

			if (KH_NewUsedStartAddress + PAGE_SIZE == KERNEL_HEAP_MAX && KH_NewUsedFreeSize == KH_NewMinimumSizeOfAddress){
				KH_NewUsedStartAddress -= KH_NewMinimumSizeOfAddress - PAGE_SIZE;
				break;
			}
		}
		else {
			if (KH_NewMinimumSizeOfAddress == KH_NewUsedFreeSize){
				KH_NewUsedStartAddress = KH_NewUsedStartAddress - KH_NewMinimumSizeOfAddress;
				break;
			}
			KH_NewUsedFreeSize = 0;
		}
	}
	return (void*)KH_NewUsedStartAddress;
}
