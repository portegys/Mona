// Pointer list functions

#include "pointerlist.hpp"

// Constructor.
PointerList::PointerList()
{
	list = NULL;
	size = 0;
	allocation = 0;
}

// Parameter initialization
int PointerList::growIncrement = 100;

// Clear
void
PointerList::clear()
{
	register int i;

	if (list != NULL)
	{
		for (i = 0; i < size; i++)
		{
			if (list[i] != NULL) delete list[i];
		}
		delete list;
		list = NULL;
	}
	size = 0;
	allocation = 0;
}

// Zero entries
void
PointerList::zero()
{
	register int i;

	if (list != NULL)
	{
		for (i = 0; i < size; i++)
		{
			list[i] = NULL;
		}
	}
}

// Insert pointer to end of list and return its index
int
PointerList::insert(char *addPointer)
{
	if (size == allocation)
	{
		realloc(allocation + growIncrement);
	}
	list[size] = addPointer;
	size++;
	return(size-1);
}

// Insert pointer at given index
void
PointerList::insert(int index, char *addPointer)
{
	if (index >= allocation)
	{
		realloc(index + 1);
	}
	list[index] = addPointer;
	if (index >= size) size = index + 1;
}

// Remove pointer and condense list
bool
PointerList::remove(char *removePointer)
{
	register int i,j;

	for (i = 0; i < size; i++)
	{
		if (list[i] == removePointer)
		{
			list[i] = NULL;
			break;
		}
	}
	if (i == size) return(false);
	for (j = i + 1; j < size; i++, j++)
	{
		list[i] = list[j];
		list[j] = NULL;
	}
	size--;
	return(true);
}

// Remove index and condense list
bool
PointerList::remove(int index)
{
	register int i,j;

	if ((i = index) >= size) return(false);
	list[i] = NULL;
	for (j = i + 1; j < size; i++, j++)
	{
		list[i] = list[j];
		list[j] = NULL;
	}
	size--;
	return(true);
}

// Get number of pointers
int
PointerList::getsize()
{
	return(size);
}
	
// Get pointer at given index
char *
PointerList::index(int i)
{
	if (i < size)
	{
		return(list[i]);
	} else {
		return(NULL);
	}
}

// Reallocate to given amount.
void
PointerList::realloc(int allocation)
{
	register char **p;

	assert(allocation >= size);
	this->allocation = allocation;
	p = new char*[allocation];
	assert(p != NULL);
	if (list != NULL) memcpy(p, list, sizeof(char *)*size);
	memset(&p[size], 0, sizeof(char *)*(allocation-size));
	if (list != NULL) delete list;
	list = p;
}
