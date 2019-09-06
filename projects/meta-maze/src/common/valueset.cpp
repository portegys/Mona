// Value set functions

#include "valueset.hpp"

// Constructors.
ValueSet::ValueSet() {}

ValueSet::ValueSet(int size)
{
    values.resize(size);
}


// Destructor.
ValueSet::~ValueSet()
{
    clear();
}


// Clear
void
ValueSet::clear()
{
    values.clear();
}


// Get size.
int
ValueSet::size()
{
    return values.size();
}


// Allocate
void
ValueSet::alloc(int size)
{
    clear();
    values.resize(size);
}


// Zero
void
ValueSet::zero()
{
    int i,j;
    for (i = 0, j = values.size(); i < j; i++) values[i] = 0.0;
}


// Get a specified value.
double
ValueSet::get(int index)
{
    assert(index >= 0 && index < values.size());
    return(values[index]);
}


// Get sum of values.
double
ValueSet::sum()
{
    int i,j;
    double d;

    d = 0.0;
    for (i = 0, j = values.size(); i < j; i++) d += values[i];
    return(d);
}


// Set a specified value.
void
ValueSet::set(int index, double value)
{
    assert(index >= 0 && index < values.size());
    values[index] = value;
}


// Add a specified value.
void
ValueSet::add(int index, double value)
{
    assert(index >= 0 && index < values.size());
    values[index] += value;
}


// Subtract a specified value.
void
ValueSet::subtract(int index, double value)
{
    assert(index >= 0 && index < values.size());
    values[index] -= value;
}


// Multiply by a specified value.
void
ValueSet::multiply(int index, double value)
{
    assert(index >= 0 && index < values.size());
    values[index] *= value;
}


// Divide by a specified value.
void
ValueSet::divide(int index, double value)
{
    assert(index >= 0 && index < values.size());
    assert(value != 0.0);
    values[index] /= value;
}


// Add a specified value to all.
void
ValueSet::add(double value)
{
    int i,j;

    for (i = 0, j = values.size(); i < j; i++)
    {
        values[i] += value;
    }
}


// Subtract a specified value from all.
void
ValueSet::subtract(double value)
{
    int i,j;

    for (i = 0, j = values.size(); i < j; i++)
    {
        values[i] -= value;
    }
}


// Multiply all by a specified value.
void
ValueSet::multiply(double value)
{
    int i,j;

    for (i = 0, j = values.size(); i < j; i++)
    {
        values[i] *= value;
    }
}


// Divide all by a specified value.
void
ValueSet::divide(double value)
{
    assert(value != 0.0);
    int i,j;

    for (i = 0, j = values.size(); i < j; i++)
    {
        values[i] /= value;
    }
}


// Load given set of values into this.
void
ValueSet::load(class ValueSet &loadSet)
{
    int i,j;

    if (loadSet.size() != values.size())
    {
        values.resize(loadSet.size());
    }

    for (i = 0, j = values.size(); i < j; i++)
    {
        values[i] = loadSet.values[i];
    }
}


// Vector addition.
void
ValueSet::add(class ValueSet &addSet)
{
    int i,j;

    assert(size() == addSet.size());

    for (i = 0, j = values.size(); i < j; i++)
    {
        values[i] += addSet.values[i];
    }
}


// Vector subtraction.
void
ValueSet::subtract(class ValueSet &subSet)
{
    int i,j;

    assert(size() == subSet.size());

    for (i = 0, j = values.size(); i < j; i++)
    {
        values[i] -= subSet.values[i];
    }
}


// Print.
void
ValueSet::print()
{
    int i,j;

    for (i = 0, j = values.size(); i < j; i++)
    {
        printf("%f ", values[i]);
    }
    printf("\n");
}


// Load.
void
ValueSet::load(FILE *fp)
{
    int i,j;

    values.clear();
    FREAD_INT(&i, fp);
    values.resize(i);
    for (i = 0, j = values.size(); i < j; i++)
    {
        FREAD_DOUBLE(&values[i], fp);
    }
}


// Save.
void
ValueSet::save(FILE *fp)
{
    int i,j;

    i = values.size();
    FWRITE_INT(&i, fp);
    for (i = 0, j = values.size(); i < j; i++)
    {
        FWRITE_DOUBLE(&values[i], fp);
    }
}
