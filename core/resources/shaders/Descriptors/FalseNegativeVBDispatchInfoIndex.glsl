#ifndef FALSENEGATIVEVBDISPATCHINFOINDEX
#define FALSENEGATIVEVBDISPATCHINFOINDEX

layout (std430, set = 1, binding = 2) buffer FalseNegativeVBDispatchInfoIndexDescriptor {
	volatile uint falseNegativeVBDispatchInfoIndexCount;
	uint falseNegativeVBDispatchInfoIndex[];
};

void FalseNegativeVBDispatchInfoIndex_Reset(uint index)
{
	falseNegativeVBDispatchInfoIndex[index] = 0;
}

uint FalseNegativeVBDispatchInfoIndex_GetIndex(uint index)
{
	return falseNegativeVBDispatchInfoIndex[index];
}

void FalseNegativeVBDispatchInfoIndex_SetData(uint index, uint data)
{
	falseNegativeVBDispatchInfoIndex[index] = data;
}

void FalseNegativeVBDispatchInfoIndex_ResetCount()
{
	falseNegativeVBDispatchInfoIndexCount = 0;
}

uint FalseNegativeVBDispatchInfoIndex_GetCount()
{
	return falseNegativeVBDispatchInfoIndexCount;
}

void FalseNegativeVBDispatchInfoIndex_SetCount(uint count)
{
	falseNegativeVBDispatchInfoIndexCount = count;
}

#endif