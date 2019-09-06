// Value set definitions

// Store and manage a set of floating point values.

#ifndef __VALUESET__
#define __VALUESET__

#include "common.h"

class ValueSet
{

    public:

        ValueSet();                               // Constructors
        ValueSet(int size);
        ~ValueSet();                              // Destructor
        int size();                               // Get size
        void alloc(int size);                     // Allocate
        void clear();                             // Clear
        void zero();                              // Zero values
        double get(int);                          // Get a value
        double sum();                             // Get sum of values
        void set(int, double);                    // Set a value
        void add(int, double);                    // Add value
        // Subtract value
        void subtract(int, double);
        // Multiply by value
        void multiply(int, double);
        void divide(int, double);                 // Divide by value
        void add(double);                         // Add value to all
        void subtract(double);                    // Subtract value from all
        void multiply(double);                    // Multiply all by value
        void divide(double);                      // Divide all by value
        void load(ValueSet&);                     // Load values from given set
        void add(ValueSet&);                      // Vector addition
        void subtract(ValueSet&);                 // Vector subtraction
        void print();                             // Print
        void load(FILE *fp);                      // Load.
        void save(FILE *fp);                      // Save.

        vector<double> values;                    // Value set
};

typedef class ValueSet VALUE_SET;
#endif
