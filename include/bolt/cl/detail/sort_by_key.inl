/***************************************************************************
*   Copyright 2012 Advanced Micro Devices, Inc.
*
*   Licensed under the Apache License, Version 2.0 (the "License");
*   you may not use this file except in compliance with the License.
*   You may obtain a copy of the License at
*
*       http://www.apache.org/licenses/LICENSE-2.0
*
*   Unless required by applicable law or agreed to in writing, software
*   distributed under the License is distributed on an "AS IS" BASIS,
*   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*   See the License for the specific language governing permissions and
*   limitations under the License.

***************************************************************************/

#if !defined( SORT_INL )
#define SORT_INL
#pragma once

#include <algorithm>
#include <type_traits>

#include <boost/bind.hpp>
#include <boost/thread/once.hpp>

#include "bolt/cl/bolt.h"
#include "bolt/cl/scan.h"
#include "bolt/cl/functional.h"
#include "bolt/cl/device_vector.h"


#define WGSIZE 64
#define DEBUG 1
namespace bolt {
    namespace cl {
        template<typename RandomAccessIterator1 , typename RandomAccessIterator2>
        void sort_by_key(RandomAccessIterator1 keys_first,
                         RandomAccessIterator1 keys_last,
                         RandomAccessIterator2 values_first,
                         const std::string& cl_code)
        {
            typedef std::iterator_traits< RandomAccessIterator1 >::value_type keys_T;

            detail::sort_by_key_detect_random_access( control::getDefault( ),
                                       keys_first, keys_last,
                                       values_first,
                                       less< keys_T >( ),
                                       cl_code,
                                       std::iterator_traits< RandomAccessIterator1 >::iterator_category( ) );
            return;
        }

        template<typename RandomAccessIterator1 , typename RandomAccessIterator2, typename StrictWeakOrdering>
        void sort_by_key(RandomAccessIterator1 keys_first,
                         RandomAccessIterator1 keys_last,
                         RandomAccessIterator2 values_first,
                         StrictWeakOrdering comp,
                         const std::string& cl_code)
        {
            typedef std::iterator_traits< RandomAccessIterator1 >::value_type keys_T;

            detail::sort_by_key_detect_random_access( control::getDefault( ),
                                       keys_first, keys_last,
                                       values_first,
                                       comp,
                                       cl_code,
                                       std::iterator_traits< RandomAccessIterator1 >::iterator_category( ) );
            return;
        }

        template<typename RandomAccessIterator1 , typename RandomAccessIterator2>
        void sort_by_key(const control &ctl,
                         RandomAccessIterator1 keys_first,
                         RandomAccessIterator1 keys_last,
                         RandomAccessIterator2 values_first,
                         const std::string& cl_code)
        {
            typedef std::iterator_traits< RandomAccessIterator1 >::value_type keys_T;

            detail::sort_by_key_detect_random_access( ctl,
                                       keys_first, keys_last,
                                       values_first,
                                       less< keys_T >( ),
                                       cl_code,
                                       std::iterator_traits< RandomAccessIterator1 >::iterator_category( ) );
            return;
        }

        template<typename RandomAccessIterator1 , typename RandomAccessIterator2, typename StrictWeakOrdering>
        void sort_by_key(const control &ctl,
                         RandomAccessIterator1 keys_first,
                         RandomAccessIterator1 keys_last,
                         RandomAccessIterator2 values_first,
                         StrictWeakOrdering comp,
                         const std::string& cl_code)
        {
            typedef std::iterator_traits< RandomAccessIterator1 >::value_type keys_T;

            detail::sort_by_key_detect_random_access( ctl,
                                       keys_first, keys_last,
                                       values_first,
                                       comp,
                                       cl_code,
                                       std::iterator_traits< RandomAccessIterator1 >::iterator_category( ) );
            return;
        }

    }
};


namespace bolt {
    namespace cl {
        namespace detail {

            struct CallCompiler_SortByKey {

                static void constructAndCompile(::cl::Kernel *masterKernel,  std::string cl_code_dataType, std::string keysTypeName,  std::string valuesTypeName, std::string compareTypeName, const control *ctl) {

                    const std::string instantiationString =
                        "// Host generates this instantiation string with user-specified value type and functor\n"
                        "template __attribute__((mangled_name(sort_by_keyInstantiated)))\n"
                        "kernel void sortByKeyTemplate(\n"
                        "global " + keysTypeName + "* keys,\n"
                        "global " + valuesTypeName + "* values,\n"
                        "const uint stage,\n"
                        "const uint passOfStage,\n"
                        "global " + compareTypeName + " * userComp\n"
                        ");\n\n";

                    bolt::cl::constructAndCompileString(masterKernel, "sort_by_key", sort_by_key_kernels, instantiationString, cl_code_dataType, keysTypeName, compareTypeName, *ctl);
                }
#if 0
                static void constructAndCompileSelectionSort(std::vector< ::cl::Kernel >* sortKernels,  std::string cl_code_dataType, std::string keysTypeName,  std::string valuesTypeName, std::string compareTypeName, const control *ctl) {

                    std::vector< const std::string > kernelNames;
                    kernelNames.push_back( "ssByKeyLocal" );
                    kernelNames.push_back( "ssByKeyFinal" );


                    const std::string instantiationString =
                        "\n// Host generates this instantiation string with user-specified value type and functor\n"
                        "template __attribute__((mangled_name(" + kernelNames[0] + "Instantiated)))\n"
                        "kernel void ssByKeyLocalTemplate(\n"
                        "global const " + keysTypeName + " * keys,\n"
                        "global const " + valuesTypeName + " * values,\n"
                        "global " + keysTypeName + " * outKeys,\n"
                        "global " + valuesTypeName + " * outValues,\n"
                        "global " + compareTypeName + " * userComp,\n"
                        "local  " + keysTypeName + " * scratch,\n"
                        "const int buffSize\n"
                        ");\n\n"

                        "\n// Host generates this instantiation string with user-specified value type and functor\n"
                        "template __attribute__((mangled_name(" + kernelNames[1] + "Instantiated)))\n"
                        "kernel void ssByKeyFinalTemplate(\n"
                        "global const " + keysTypeName + " * keys,\n"
                        "global const " + valuesTypeName + " * values,\n"
                        "global " + keysTypeName + " * outKeys,\n"
                        "global " + valuesTypeName + " * outValues,\n"
                        "global " + compareTypeName + " * userComp,\n"
                        "local  " + keysTypeName + " * scratch,\n"
                        "const int buffSize\n"
                        ");\n\n";

                    bolt::cl::compileKernelsString( *sortKernels, kernelNames, sort_by_key_kernels, instantiationString, cl_code_dataType, keysTypeName, compareTypeName, *ctl );
                }
#endif
            }; //End of struct CallCompiler_Sort

            // Wrapper that uses default control class, iterator interface
            template<typename RandomAccessIterator1, typename RandomAccessIterator2, typename StrictWeakOrdering>
            void sort_by_key_detect_random_access( const control &ctl, RandomAccessIterator1 keys_first, RandomAccessIterator1 keys_last,
                                                   RandomAccessIterator2 values_first, StrictWeakOrdering comp,
                                                   const std::string& cl_code, std::input_iterator_tag )
            {
                //  TODO:  It should be possible to support non-random_access_iterator_tag iterators, if we copied the data
                //  to a temporary buffer.  Should we?
                static_assert( false, "Bolt only supports random access iterator types" );
            };

            template<typename RandomAccessIterator1, typename RandomAccessIterator2, typename StrictWeakOrdering>
            void sort_by_key_detect_random_access( const control &ctl, RandomAccessIterator1 keys_first, RandomAccessIterator1 keys_last,
                								   RandomAccessIterator2 values_first, StrictWeakOrdering comp,
                                                   const std::string& cl_code, std::random_access_iterator_tag )
            {
                return sort_by_key_pick_iterator(ctl, keys_first, keys_last, values_first, comp, cl_code);
            };

            //Device Vector specialization
            template<typename DVRandomAccessIterator1, typename DVRandomAccessIterator2, typename StrictWeakOrdering>
            typename std::enable_if< std::is_base_of<typename device_vector<typename std::iterator_traits<DVRandomAccessIterator1>::value_type>::iterator,DVRandomAccessIterator1>::value >::type
            sort_by_key_pick_iterator(const control &ctl, DVRandomAccessIterator1 keys_first, DVRandomAccessIterator1 keys_last, DVRandomAccessIterator2 values_first,
                StrictWeakOrdering comp, const std::string& cl_code)
            {
                // User defined Data types are not supported with device_vector. Hence we have a static assert here.
                // The code here should be in compliant with the routine following this routine.
                size_t szElements = (size_t)(keys_last - keys_first);
                if (szElements == 0 )
                        return;
                const bolt::cl::control::e_RunMode runMode = ctl.forceRunMode();  // could be dynamic choice some day.
                if (runMode == bolt::cl::control::SerialCpu) {
                    //  TODO:  Need access to the device_vector .data method to get a host pointer
                    throw ::cl::Error( CL_INVALID_DEVICE, "Sort of device_vector CPU device not implemented" );
                    return;
                } else if (runMode == bolt::cl::control::MultiCoreCpu) {
                    std::cout << "The MultiCoreCpu version of device_vector sort is not implemented yet." << std ::endl;
                    throw ::cl::Error( CL_INVALID_DEVICE, "The BOLT sort routine device_vector does not support non power of 2 buffer size." );
                    return;
                } else {
                    sort_by_key_enqueue(ctl, keys_first, keys_last, values_first, comp, cl_code);
                }
                return;
            }

            //Non Device Vector specialization.
            //This implementation creates a cl::Buffer and passes the cl buffer to the sort specialization whichtakes the cl buffer as a parameter.
            //In the future, Each input buffer should be mapped to the device_vector and the specialization specific to device_vector should be called.
            template<typename RandomAccessIterator1, typename RandomAccessIterator2, typename StrictWeakOrdering>
            typename std::enable_if< !std::is_base_of<typename device_vector<typename std::iterator_traits<RandomAccessIterator1>::value_type>::iterator,RandomAccessIterator1>::value >::type
            sort_by_key_pick_iterator(const control &ctl, RandomAccessIterator1 keys_first, RandomAccessIterator1 keys_last, RandomAccessIterator2 values_first,
                StrictWeakOrdering comp, const std::string& cl_code)
            {
                typedef typename std::iterator_traits<RandomAccessIterator1>::value_type T_keys;
                typedef typename std::iterator_traits<RandomAccessIterator2>::value_type T_values;
                size_t szElements = (size_t)(keys_last - keys_first);
                if (szElements == 0)
                    return;

                const bolt::cl::control::e_RunMode runMode = ctl.forceRunMode();  // could be dynamic choice some day.
                if ((runMode == bolt::cl::control::SerialCpu) || (szElements < WGSIZE)) {
                    std::cout << "The SerialCpu version of sort is not implemented yet." << std ::endl;
                    std::sort(keys_first, keys_last);
                    return;
                } else if (runMode == bolt::cl::control::MultiCoreCpu) {
                    std::cout << "The MultiCoreCpu version of sort is not implemented yet." << std ::endl;
                } else {
                    device_vector< T_values > dvInputValues( values_first, szElements, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, true, ctl );
                    device_vector< T_keys > dvInputKeys( keys_first, keys_last, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, ctl );
                    //Now call the actual cl algorithm
                    sort_by_key_enqueue(ctl,dvInputKeys.begin(),dvInputKeys.end(), dvInputValues.begin(), comp, cl_code);
                    //Map the buffer back to the host
                    dvInputValues.data( );
                    dvInputKeys.data( );
                    return;
                }
            }


            template<typename DVRandomAccessIterator1, typename DVRandomAccessIterator2, typename StrictWeakOrdering> 
            void sort_by_key_enqueue(const control &ctl, const DVRandomAccessIterator1& keys_first, const DVRandomAccessIterator1& keys_last, const DVRandomAccessIterator2& values_first, 
                const StrictWeakOrdering& comp, const std::string& cl_code)  
            {
                    typedef typename std::iterator_traits< DVRandomAccessIterator1 >::value_type T_keys;
                    typedef typename std::iterator_traits< DVRandomAccessIterator2 >::value_type T_values;
                    size_t szElements = (size_t)(keys_last - keys_first);
                    if(((szElements-1) & (szElements)) != 0)
                    {
                        // sort_by_key_enqueue_non_powerOf2(ctl,keys_first,keys_last,values_first,comp,cl_code);
                        std::cout << "There is no use supporting selection sort for the sort_by_key routine.\n Hence only power of 2 buffer sizes work.\n non power of 2 buffer sizes will be supported once radix-sort is working\n";
                        throw ::cl::Error( CL_INVALID_BUFFER_SIZE, "Currently the sort_by_key routine supports only power of 2 buffer size" );
                        return;
                    }
                    static  boost::once_flag initOnlyOnce;
                    static  ::cl::Kernel masterKernel;

                    size_t temp;

                    // Set up shape of launch grid and buffers:
                    int computeUnits     = ctl.device().getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
                    int wgPerComputeUnit =  ctl.wgPerComputeUnit();
                    int resultCnt = computeUnits * wgPerComputeUnit;
                    cl_int l_Error = CL_SUCCESS;

                    //Power of 2 buffer size
                    //For user-defined types, the user must create a TypeName trait which returns the name of the class - note use of TypeName<>::get to retreive the name here.
                    boost::call_once( initOnlyOnce, boost::bind( CallCompiler_SortByKey::constructAndCompile, &masterKernel, cl_code + ClCode<T_keys>::get() + ClCode<T_values>::get(), TypeName<T_keys>::get(), TypeName<T_values>::get(), TypeName<StrictWeakOrdering>::get(), &ctl) );

                    size_t wgSize  = masterKernel.getWorkGroupInfo< CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE >( ctl.device( ), &l_Error );
                    V_OPENCL( l_Error, "Error querying kernel for CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE" );
                    if((szElements/2) < wgSize)
                    {
                        wgSize = (int)szElements/2;
                    }
                    unsigned int numStages,stage,passOfStage;

                    ::cl::Buffer Keys = keys_first->getBuffer( );
                    ::cl::Buffer Values = values_first->getBuffer( );
                    ::cl::Buffer userFunctor(ctl.context(), CL_MEM_USE_HOST_PTR, sizeof(comp), (void*)&comp );   // Create buffer wrapper so we can access host parameters.

                    ::cl::Kernel k = masterKernel;  // hopefully create a copy of the kernel. FIXME, doesn't work.
                    numStages = 0;
                    for(temp = szElements; temp > 1; temp >>= 1)
                        ++numStages;
                    V_OPENCL( k.setArg(0, Keys), "Error setting a kernel argument" );
                    V_OPENCL( k.setArg(1, Values), "Error setting a kernel argument" );
                    V_OPENCL( k.setArg(4, userFunctor), "Error setting a kernel argument" );
                    for(stage = 0; stage < numStages; ++stage)
                    {
                        // stage of the algorithm
                        V_OPENCL( k.setArg(2, stage), "Error setting a kernel argument" );
                        // Every stage has stage + 1 passes
                        for(passOfStage = 0; passOfStage < stage + 1; ++passOfStage) {
                            // pass of the current stage
                            V_OPENCL( k.setArg(3, passOfStage), "Error setting a kernel argument" );
                            /*
                             * Enqueue a kernel run call.
                             * Each thread writes a sorted pair.
                             * So, the number of  threads (global) should be half the length of the input buffer.
                             */
                            l_Error = ctl.commandQueue().enqueueNDRangeKernel(
                                    k,
                                    ::cl::NullRange,
                                    ::cl::NDRange(szElements/2),
                                    ::cl::NDRange(wgSize),
                                    NULL,
                                    NULL);
                            V_OPENCL( l_Error, "enqueueNDRangeKernel() failed for sort() kernel" );
                            V_OPENCL( ctl.commandQueue().finish(), "Error calling finish on the command queue" );
                        }//end of for passStage = 0:stage-1
                    }//end of for stage = 0:numStage-1
                    //Map the buffer back to the host
                    ctl.commandQueue().enqueueMapBuffer(Keys, true, CL_MAP_READ | CL_MAP_WRITE, 0/*offset*/, sizeof(T_keys) * szElements, NULL, NULL, &l_Error );
                    ctl.commandQueue().enqueueMapBuffer(Values, true, CL_MAP_READ | CL_MAP_WRITE, 0/*offset*/, sizeof(T_values) * szElements, NULL, NULL, &l_Error );
                    V_OPENCL( l_Error, "Error calling map on the result buffer" );
                    return;
            }// END of sort_enqueue

#if 0
            template<typename DVRandomAccessIterator1, typename DVRandomAccessIterator2, typename StrictWeakOrdering>
            void sort_by_key_enqueue_non_powerOf2(const control &ctl, DVRandomAccessIterator1 keys_first, DVRandomAccessIterator1 keys_last, DVRandomAccessIterator2 values_first,
                StrictWeakOrdering comp, const std::string& cl_code)
            {
                    //std::cout << "The BOLT sort routine does not support non power of 2 buffer size. Falling back to CPU std::sort" << std ::endl;
                    typedef typename std::iterator_traits< DVRandomAccessIterator1 >::value_type T_keys;
                    typedef typename std::iterator_traits< DVRandomAccessIterator2 >::value_type T_values;
                    static boost::once_flag initOnlyOnce;
                    size_t szElements = (size_t)(keys_last - keys_first);

                    // Set up shape of launch grid and buffers:
                    int computeUnits     = ctl.device().getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
                    int wgPerComputeUnit =  ctl.wgPerComputeUnit();
                    cl_int l_Error = CL_SUCCESS;

                    //Power of 2 buffer size
                    // For user-defined types, the user must create a TypeName trait which returns the name of the class - note use of TypeName<>::get to retreive the name here.
                    static std::vector< ::cl::Kernel > sortByKeyKernels;
                    boost::call_once( initOnlyOnce, boost::bind( CallCompiler_SortByKey::constructAndCompileSelectionSort, &sortByKeyKernels, cl_code + ClCode<T_keys>::get() + ClCode<T_values>::get(), TypeName<T_keys>::get(), TypeName<T_values>::get(), TypeName<StrictWeakOrdering>::get(), &ctl) );

                    size_t wgSize  = sortByKeyKernels[0].getWorkGroupInfo< CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE >( ctl.device( ), &l_Error );

                    size_t totalWorkGroups = (szElements + wgSize)/wgSize;
                    size_t globalSize = totalWorkGroups * wgSize;
                    V_OPENCL( l_Error, "Error querying kernel for CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE" );

                    ::cl::Buffer in = keys_first->getBuffer( );
                    ::cl::Buffer inValues = values_first->getBuffer( );
                    ::cl::Buffer out(ctl.context(), CL_MEM_READ_WRITE, sizeof(T_keys)*szElements);
                    ::cl::Buffer outValues(ctl.context(), CL_MEM_READ_WRITE, sizeof(T_values)*szElements);
                    ::cl::Buffer userFunctor(ctl.context(), CL_MEM_USE_HOST_PTR, sizeof(comp), &comp );   // Create buffer wrapper so we can access host parameters.
                    ::cl::LocalSpaceArg loc;
                    loc.size_ = wgSize*sizeof(T_keys);

                    V_OPENCL( sortByKeyKernels[0].setArg(0, in), "Error setting a kernel argument in" );
                    V_OPENCL( sortByKeyKernels[0].setArg(1, inValues), "Error setting a kernel argument in" );
                    V_OPENCL( sortByKeyKernels[0].setArg(2, out), "Error setting a kernel argument out" );
                    V_OPENCL( sortByKeyKernels[0].setArg(3, outValues), "Error setting a kernel argument out" );
                    V_OPENCL( sortByKeyKernels[0].setArg(4, userFunctor), "Error setting a kernel argument userFunctor" );
                    V_OPENCL( sortByKeyKernels[0].setArg(5, loc), "Error setting kernel argument loc" );
                    V_OPENCL( sortByKeyKernels[0].setArg(6, static_cast<cl_uint> (szElements)), "Error setting kernel argument szElements" );
                    {
                            l_Error = ctl.commandQueue().enqueueNDRangeKernel(
                                    sortByKeyKernels[0],
                                    ::cl::NullRange,
                                    ::cl::NDRange(globalSize),
                                    ::cl::NDRange(wgSize),
                                    NULL,
                                    NULL);
                            V_OPENCL( l_Error, "enqueueNDRangeKernel() failed for sort() kernel" );
                            V_OPENCL( ctl.commandQueue().finish(), "Error calling finish on the command queue" );
                    }

                    wgSize  = sortByKeyKernels[1].getWorkGroupInfo< CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE >( ctl.device( ), &l_Error );

                    V_OPENCL( sortByKeyKernels[1].setArg(0, out), "Error setting a kernel argument in" );
                    V_OPENCL( sortByKeyKernels[1].setArg(1, outValues), "Error setting a kernel argument in" );
                    V_OPENCL( sortByKeyKernels[1].setArg(2, in), "Error setting a kernel argument out" );
                    V_OPENCL( sortByKeyKernels[1].setArg(3, inValues), "Error setting a kernel argument out" );
                    V_OPENCL( sortByKeyKernels[1].setArg(4, userFunctor), "Error setting a kernel argument userFunctor" );
                    V_OPENCL( sortByKeyKernels[1].setArg(5, loc), "Error setting kernel argument loc" );
                    V_OPENCL( sortByKeyKernels[1].setArg(6, static_cast<cl_uint> (szElements)), "Error setting kernel argument szElements" );
                    {
                            l_Error = ctl.commandQueue().enqueueNDRangeKernel(
                                    sortByKeyKernels[1],
                                    ::cl::NullRange,
                                    ::cl::NDRange(globalSize),
                                    ::cl::NDRange(wgSize),
                                    NULL,
                                    NULL);
                            V_OPENCL( l_Error, "enqueueNDRangeKernel() failed for sort() kernel" );
                            V_OPENCL( ctl.commandQueue().finish(), "Error calling finish on the command queue" );
                    }
                    // Map the buffer back to the host
                    ctl.commandQueue().enqueueMapBuffer(in, true, CL_MAP_READ | CL_MAP_WRITE, 0/*offset*/, sizeof(T_keys) * szElements, NULL, NULL, &l_Error );
                    ctl.commandQueue().enqueueMapBuffer(inValues, true, CL_MAP_READ | CL_MAP_WRITE, 0/*offset*/, sizeof(T_values) * szElements, NULL, NULL, &l_Error );
                    V_OPENCL( l_Error, "Error calling map on the result buffer" );
                    return;
            }// END of sort_enqueue_non_powerOf2
#endif
        }//namespace bolt::cl::detail
    }//namespace bolt::cl
}//namespace bolt

#endif