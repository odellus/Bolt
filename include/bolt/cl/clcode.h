#pragma once

#include <string>

/*!
* \page OpenCLCodeStrings_Page OpenCL(TM) Code Strings
* \tableofcontents
* \section SplitSource_Section Introduction and the Split-Source Model
* OpenCL provides a split compilation model, meaning that the host and device code
* are split into separate files and compiled by different compilers.  In OpenCL(TM),
* the device code (the kernels) are typically provided as strings which are 
* passed through the OpenCL(TM) runtime APIs to compile for the target device.  Recall that
* a Bolt algorithm can also be executed on the CPU, and Bolt may use non-OpenCL(TM)
* paths for host execution such as a serial loop or a multi-core task-parallel runtime
* such as Intel Threading Building Blocks.  Additionally, advanced use cases of Bolt 
* use a functor which is initialized on the host CPU using the functor constructor, 
* and then executed on the device using the body operator.  
*
* \section Functors C++ Functors
* A functor ("function object") is a C++ construct that allows us to construct a
* class or struct which can be called like a regular function.  The surrounding 
* class can be used to capture additional values which can be used inside the function.
* Essentially the function has additional state beyond just its input arguments but without
* changing the calling signature of the function.  This is a critical point for the 
* construction of generic libraries (such as Bolt algorithms), which can then contain
* a call to the well-defined function interface.  For example, consider the classic 
* Saxpy code which uses a functor to pass the value "100" from the calling scope to the 
* \p transform algorithm:
* \reference saxpy_stl
* \code
// STL implementation of Saxpy showing using of functor:
#include <algorithm>
#include <vector>

struct SaxpyFunctor 
{
    float _a;
    SaxpyFunctor(float a) : _a(a) {};

    float operator() (const float &xx, const float &yy)
    {
        return _a * xx + yy;
    };
};

void main() { 
   SaxpyFunctor s(100);
   std::vector<float> x(1000000); // initialization not shown
   std::vector<float> y(1000000); // initialization not shown
   std::vector<float> z(1000000);
   
   std::transform(x.begin(), x.end(), y.begin(), z.begin(), s);
};
* \endcode
*
*
* \section OclSplitSource_Tools Bolt Tools for Split-Source
* The bottom line is that Bolt requires that functor classes be defined both as strings
* (for OpenCL(TM) compilation) and as regular host-side definitions.  Since we don't want
* to have to create and maintain two copies of our source code, Bolt provides several 
* mechanisms to construct the two representations from a single source definition. These are 
* described below.  
*
*
*
* \subsection boltfunctormacro BOLT_FUNCTOR macro
*
* The simplest technique is to use the \p BOLT_FUNCTOR macro.  Given a class name and
* a definition for that class, this macro will automatically:
*    \li Create a string version of the class definition, and associate the string with the class.  (Bolt finds the string using a C++ trait called \p ClCode. This is described in more detail below.)
*    \li Create a string version of the class name.  (Bolt finds the class name using the \p TypeName trait.  This is described in more detail below.)
*    \li The macro expansion contains the class definition - so the host compiler (used for CPU execution) also has a version of the class.
*
* The example below shows how to use the BOLT_FUNCTOR macro to implement the Saxpy function using Bolt:
* \code
// Show the use of BOLT_FUNCTOR macro
#include <bolt/cl/transform.h>
#include <vector>

BOLT_FUNCTOR(SaxpyFunctor,
struct SaxpyFunctor 
{
    float _a;
    SaxpyFunctor(float a) : _a(a) {};

    float operator() (const float &xx, const float &yy)
    {
        return _a * xx + yy;
    };
};
);


void main() { 
   SaxpyFunctor s(100);
   std::vector<float> x(1000000); // initialization not shown
   std::vector<float> y(1000000); // initialization not shown
   std::vector<float> z(1000000);
   
   bolt::cl::transform(x.begin(), x.end(), y.begin(), z.begin(), s);
};
* \endcode
*
*
* BOLT_FUNCTOR requires only a small syntax change as compared to the original Saxpy implementation #saxpy_stl - only the BOLT_FUNCTOR line
* before the class definition, passing the name of the class as a parameter, then the unmodified functor, and a trailing ");" at the end
* to close the macro.  This can be surprisingly useful, especially for the relatively common operation of creating simple functors 
* for use with algorthms.  However, BOLT_FUNCTOR is based on a standard C-style #define macro, and as a result does have several 
* important limitations:
*    \li BOLT_FUNCTOR only works with non-templated classes.  To use a templated class with a Bolt algorithm call, see the next
*       section \p BOLT_CODE_STRING.
*    \li Unprotected commas can break the macro's argument parsing.  If this happens, your compiler will 
*        generate an error such as <em>"too many actual parameters for macro 'BOLT_FUNCTOR'"</em>.  A common source of this
*        error is an initializer list which contains more than one field.
*        Another cause for this error is template definitions with more than one typename (ie template< typename T1, typename T2>).
*    \li BOLT_FUNCTOR uses macro stringification to create the OpenCL(TM) code string.  As a result, the class definition has all line feeds 
*       removed and is "smushed" into a single line before being passed to the compiler.  This can make debugging harder, and for that reason
*       you may choose to use one of the other techniqueus described below.
*
* \subsection boltcodestring TypeName and ClCode
*
* In order to create the OpenCL code string, the Bolt algorithm implementations need access to the following information:
*   \li the name of the functor (ie "SaxypFunctor" in the examples above) 
*   \li the class definition (ie the code that defines the SaxpyFunctor class).
*
* Bolt uses C++ traits to define both of these fields.  A trait is a C++ coding technique which uses template specialization
* to allow the name (or code) to be associated with the class.  As an example, we define a baseline TypeName trait which
* returns an error message.  Each class which is to be used by Bolt must provide a template specialization for the TypeName
* class which returns the string version of the class.   For example:
* \code
* template<> struct TypeName<SaxpyFunctor> { static std::string get() { return "SaxpyFunctor"; }};
*
* // Use convenience function - this has same result as above.
* BOLT_CREATE_TYPENAME(SaxpyFunctor);
* \endcode
*
* Because the template specialization syntax can be a bit verbose, Bolt provides the convenience macro BOLT_CREATE_TYPENAME
* as shown in the example above.  Note that the class name used for BOLT_CREATE_TYPENAME (or the more verbose template specialization
* equivalent) must be fully instantiated without any template parameters. So \p BOLT_CREATE_TYPENAME(myplus<T>) is illegal but
* \p BOLT_CREATE_TYPENAME(myplus<int>) is legal syntax.
*
* Bolt uses a similar technique to associate the string representation of the class with the class definition. In this
* case the C++ trait is called "ClCode", and the default value is the empty string.  Bolt defines a convenience function 
* \p BOLT_CREATE_CLCODE to assist in creating ClCode. An example:
* \code
* BOLT_CREATE_CLCODE(MyClass, "class MyClass { ... };");
* \endcode
*
*
*
* The BOLT_FUNCTOR macro described in the previous section implicitly calls BOLT_CREATE_TYPENAME and BOLT_CREATE_CLCODE,
* but you can also opt to explicitly call these.  This is more verbose but can be useful to work around the limitations
* of the BOLT_FUNCTOR macro, specifically:
*   \li the BOLT_CREATE_CLCODE does not rely on macro stringification, and thus does not have the limitations with unprotected commas, 
*   and does not "smush" the code into a single line. See the example in the next section on how we will use this to create the 
*   ClCode string from a functor definition which is stored in a separate file.
*
*   \li When working with a templated class, you can call BOLT_CREATE_TYPENAME to create typenames for the instantiated versions
*       of the class that you plan to use in the OpenCL(TM) code.  See the example below.
*
* Before showing the next example, let's introduce two more concepts that allow low-level control over the OpenCL code 
* which is compiled by the Bolt algorithm implementation.   
*
* \p BOLT_CODE_STRING is a macro which is designed to make it easier to work with templated classes.  
* BOLT_CODE_STRING will expand to create a host-side version 
* of the functor and will also return a string which can later be manually associated with the class. 
*
* Finally, the last argument to every Bolt algorithm API call is an optional parameter \p cl_code which contains any code that 
* we wish to pass to the OpenCL compiler.  This parameter is useful in the case of templated functors
*
* Let's take a look at an example usage:
*
*\code
// Example of defining a templated class that works with Bolt
std::string InRange_CodeString = 
BOLT_CODE_STRING(
template<typename T>
// Functor for range checking.
struct InRange {
	InRange (T low, T high) {
		_low=low;
		_high=high;
	};

	bool operator() (const T& value) { return (value >= _low) && (value <= _high) ; };

	T _low;
	T _high;
};
);
BOLT_CREATE_TYPENAME(InRange<float>);

void testCountIf(int aSize) 
{
	std::vector<float> A(aSize);
	for (int i=0; i < aSize; i++) {
		A[i] = static_cast<float> (i+1);
	};

	std::cout << "BOLT Count7..15=" << bolt::cl::count_if (A.begin(), A.end(), InRange<float>(7,15), InRange_CodeString) << std::endl;
}
\endcode

* TODO-explain
*
* \subsection codeFromFile Reading code from a file
* By now you may be thinking "enough with all the stringify macros already".  This section describes how to store the functor code in a separate file,
* and then #include it (to create the host version) and use the Bolt calls to create the ClCode and TypeName traits that the Bolt 
* algorithm implementations are looking for.
*
*
* \subsection different_code Different Code
* TODO
*
* \section Conclusion Conclusion
* TODO

* - define functor or function object.
* - describe macros that may work and the limitations of said macros.
* - describe how to place code in a file.
* - describe how to use different versions of code for CL and for hot.
* - Get the documentation in the right place - add to modules/misc for code definitions below, and provide link to the SplitSource_Page
*   - Avoid defining class traits more than once. (just like classes)
*  user_code parameter.
*  Document the functions below
*  Move this code file to another location.
*
* /{
*/

//---
// TypeName trait implementation - this is the base version that is typically overridden
template <typename T>
struct TypeName
{
	static std::string get()
	{
		return std::string("ERROR (bolt): Unknown typename; define missing TypeName<") + typeid(T).name() + ">";  
	}
};


//---
// ClCode trait implementation - this is the base version that is typically overridden to asscociate OpenCL code with this class.
template <typename T>
struct ClCode
{
	static std::string get()
	{
		return "";
	}
};


// Create a template trait to return a string version of the class name.  Usage:
// class MyClass {...};
// BOLT_CREATE_TYPENAME(MyClass);
#define BOLT_CREATE_TYPENAME(T) \
	template<> struct TypeName<T> { static std::string get() { return #T; }};


/*!
* Creates the ClCode trait which associated the specified type \p T with the string \p CODE_STRING
* \arg T : Class to use
* \arg CODE_STRING : Code string to associate with T's ClCode trait.
* Note: Only one ClCode can be created 
*/
#define BOLT_CREATE_CLCODE(T,CODE_STRING) template<> struct ClCode<T> { static std::string get() { return CODE_STRING; }};





// Creates a string and a regular version of the functor F, and automatically sets up the ClCode trait to associate the code string with the specified class T 
#define BOLT_FUNCTOR(T, F)  F ; BOLT_CREATE_TYPENAME(T); BOLT_CREATE_CLCODE(T,#F);


// Return a string with the specified function F, and also create code that is fed to the host compiler.
#define BOLT_CODE_STRING(F)  #F; F ; 
