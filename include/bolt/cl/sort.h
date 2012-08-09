#pragma once

#include <bolt/cl/bolt.h>
#include <bolt/cl/functional.h>
#include <mutex>
#include <string>
#include <iostream>

namespace bolt {
	namespace cl {
        

        /*! \addtogroup algorithms
         */

		/*! \addtogroup sorting
        *   \ingroup algorithms
        *   The sorting Algorithm for sorting the given InputIterator.
        */ 

		/*! \addtogroup sort
        *   \ingroup sorting
        *   \{
        */



		/*! \p sort returns the sorted result of all the elements in the inputIterator between the the first and last elements using the specified binary_op.  
		* You can do an ascending order sorting, where the binary_op is the minimum operator.  By default, 
		* the binary operator is "minimum<>()".  The version takes a bolt::cl::control structure as a first argument.
		*
		* \p reduce requires that the binary reduction op ("binary_op") is cummutative.  The order in which \p reduce applies the binary_op
		* is not deterministic.
		*
		* The \p reduce operation is similar the std::accumulate function.  See http://www.sgi.com/tech/stl/accumulate.html.
		*
		* \param ctl Control structure to control command-queue, debug, tuning, etc.  See control.
		* \param first The first position in the sequence to be reduced.
		* \param last  The last position in the sequence to be reduced.
		* \param init  The initial value for the accumulator.
		* \param binary_op  The binary operation used to combine two values.   By default, the binary operation is plus<>().
		* \param cl_code Optional OpenCL(TM) code to be passed to the OpenCL compiler. The cl_code is inserted first in the generated code, before the cl_code trait.
		* \return The result of the reduction.
		*
		* The following code example shows the use of \p reduce to find the max of 10 numbers, 
		* specifying a specific command-queue and enabling debug messages.
		* \code
		* #include <bolt/cl/reduce.h>
		*
		* int a[10] = {2, 9, 3, 7, 5, 6, 3, 8, 3, 4};
		*
		* cl::CommandQueue myCommandQueue = ...
		*
		* bolt::cl::control ctl(myCommandQueue); // specify an OpenCL(TM) command queue to use for executing the reduce.
		* ctl.debug(bolt::cl::control::debug::SaveCompilerTemps); // save IL and ISA files for generated kernel
		*
		* int max = bolt::cl::reduce(ctl, a, a+10, -1, bolt::cl:maximum<int>());
		* // max = 9
		*  \endcode
		*/

		template<typename RandomAccessIterator, typename Compare> 
		void sort(const bolt::cl::control &ctl,
			RandomAccessIterator first, 
			RandomAccessIterator last,  
            Compare comp=bolt::cl::minimum<typename std::iterator_traits<RandomAccessIterator>::value_type>(), 
			const std::string cl_code="");



		/*! \p reduce returns the result of combining all the elements in the specified range using the specified binary_op.  
		* The classic example is a summation, where the binary_op is the plus operator.  By default, the initial value is "0" 
		* and the binary operator is "plus<>()".
		*
		* \p reduce requires that the binary reduction op ("binary_op") is cummutative.  The order in which \p reduce applies the binary_op
		* is not deterministic.
		*
		* The \p reduce operation is similar the std::accumulate function.  See http://www.sgi.com/tech/stl/accumulate.html.
		*
		* \param ctl Control structure to control command-queue, debug, tuning. See FIXME.
		* \param first The first position in the sequence to be reduced.
		* \param last  The last position in the sequence to be reduced.
		* \param cl_code Optional OpenCL(TM) code to be passed to the OpenCL compiler. The cl_code is inserted first in the generated code, before the cl_code trait.
		* \return The result of the reduction.
		*
		* The following code example shows the use of \p reduce to sum 10 numbers, using the default plus operator.
		* \code
		* #include <bolt/cl/reduce.h>
		*
		* int a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
		*
		* cl::CommandQueue myCommandQueue = ...
		*
		* bolt::cl::control ctl(myCommandQueue); // specify an OpenCL(TM) command queue to use for executing the reduce.
		* ctl.debug(bolt::cl::control::debug::SaveCompilerTemps); // save IL and ISA files for generated kernel
		*
		* int sum = bolt::cl::reduce(ctl, a, a+10);
		* // sum = 55
		*  \endcode
		*/
		template<typename RandomAccessIterator> 
		void sort(const bolt::cl::control &ctl,
			RandomAccessIterator first, 
			RandomAccessIterator last, 
			const std::string cl_code="");




		/*! \p reduce returns the result of combining all the elements in the specified range using the specified binary_op.  
		* The classic example is a summation, where the binary_op is the plus operator.  By default, 
		* the binary operator is "plus<>()".
		*
		* \p reduce requires that the binary reduction op ("binary_op") is cummutative.  The order in which \p reduce applies the binary_op
		* is not deterministic.
		*
		* The \p reduce operation is similar the std::accumulate function.  See http://www.sgi.com/tech/stl/accumulate.html.
		*
		* \param first The first position in the sequence to be reduced.
		* \param last  The last position in the sequence to be reduced.
		* \param init  The initial value for the accumulator.
		* \param binary_op  The binary operation used to combine two values.   By default, the binary operation is plus<>().
		* \param cl_code Optional OpenCL(TM) code to be passed to the OpenCL compiler. The cl_code is inserted first in the generated code, before the cl_code trait.
		* \return The result of the reduction.
		*
		* The following code example shows the use of \p reduce to sum 10 numbers plus 100, using the default plus operator.
		* \code
		* #include <bolt/cl/reduce.h>
		*
		* int a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
		*
		* int sum = bolt::cl::reduce(a, a+10, 100);
		* // sum = 155
		*  \endcode
		*
		* The following code example shows the use of \p reduce to find the max of 10 numbers:
		* \code
		* #include <bolt/cl/reduce.h>
		*
		* int a[10] = {2, 9, 3, 7, 5, 6, 3, 8, 3, 4};
		*
		* int max = bolt::cl::reduce(a, a+10, -1, bolt::cl:maximum<int>());
		* // max = 9
		*  \endcode
		*/
		template<typename RandomAccessIterator, typename Compare> 
		void sort(RandomAccessIterator first, 
			RandomAccessIterator last,  
            Compare comp=bolt::cl::minimum<typename std::iterator_traits<RandomAccessIterator>::value_type>(), 
			const std::string cl_code="")  ;


		/*! \p reduce returns the result of combining all the elements in the specified range using the specified binary_op.  
		* The classic example is a summation, where the binary_op is the plus operator.  By default, the initial value is "0" 
		* and the binary operator is "plus<>()".
		*
		* \p reduce requires that the binary reduction op ("binary_op") is cummutative.  The order in which \p reduce applies the binary_op
		* is not deterministic.
		*
		* The \p reduce operation is similar the std::accumulate function.  See http://www.sgi.com/tech/stl/accumulate.html.
		*
		* \param first The first position in the sequence to be reduced.
		* \param last  The last position in the sequence to be reduced.
		* \param cl_code Optional OpenCL(TM) code to be passed to the OpenCL compiler. The cl_code is inserted first in the generated code, before the cl_code trait.
		* \return The result of the reduction.
		*
		* The following code example shows the use of \p reduce to sum 10 numbers, using the default plus operator.
		* \code
		* #include <bolt/cl/reduce.h>
		*
		* int a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
		*
		* int sum = bolt::cl::reduce(a, a+10);
		* // sum = 55
		*  \endcode
		*
		*/
		template<typename RandomAccessIterator> 
		void sort(RandomAccessIterator first, 
			RandomAccessIterator last, 
			const std::string cl_code="");

	}
}

#include <bolt/cl/detail/sort.inl>
