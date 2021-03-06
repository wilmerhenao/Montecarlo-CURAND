/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

#include "../inc/test.h"

#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <memory>
#include <iostream>
#include <cassert>
#include <typeinfo>
#include <shrUtils.h>
#include <cuda_runtime_api.h>

#include "../inc/genericoption.h"
#include "../inc/pricingengine.h"

template <typename Real>
bool Test<Real>::operator()()
{
    using std::stringstream;
    using std::endl;
    using std::setw;

    // Get device properties
    struct cudaDeviceProp deviceProperties;
    cudaError_t cudaResult = cudaGetDeviceProperties(&deviceProperties, device);
    if (cudaResult != cudaSuccess){
        std::string msg("Could not get device properties: ");
        msg += cudaGetErrorString(cudaResult);
        throw std::runtime_error(msg);
    }

    // This test prices a set of path-dependent options
    genericOption<Real> option;
    option.spot   = static_cast<Real>(40);
    option.strike = static_cast<Real>(35);
    option.r      = static_cast<Real>(0.03);
    option.sigma  = static_cast<Real>(0.20);
    option.tenor  = static_cast<Real>(1.0/3.0);
    option.dt     = static_cast<Real>(1.0/261);
    option.type   = genericOption<Real>::Call;
    option.valueAsian  = static_cast<Real>(0.0);
    option.golden = static_cast<Real>(5.162534);
    option.barrier = static_cast<Real>(45);
    
    // Evaluate on GPU
    shrLog("Pricing option on GPU (%s)\n\n", deviceProperties.name);
    // pricer is an instantiation of the PricingEngine class
    PricingEngine<Real> pricer(numSims, device, threadBlockSize, seed);
    //shrDeltaT(0);
    pricer(option);
    //elapsedTime = shrDeltaT(0);
    // Run PlainVanilla Option on the CPU
    shrDeltaT(1);
    pricer[option];
    elapsedTimeCPU = shrDeltaT(1);
    // Tolerance to compare result with expected
    // This is just to check that nothing has gone very wrong with the
    // test, the actual accuracy of the result depends on the number of
    // MonteCarlo trials
    const Real tolerance = static_cast<Real>(0.1);
    
    // Display results
    stringstream output;
    output << "Time Spent working on PlainVanilla on the CPU: " << elapsedTimeCPU << endl;
    //output << "Improvement CPU / GPU is: " << (elapsedTimeCPU / elapsedTime) << " times faster" << std::endl;
    output << "Precision:      " << ((typeid(Real) == typeid(double)) ? "double" : "single") << endl;
    output << "Number of simulations: " << numSims << endl;
    output << endl;
    output << "Spot|Strike|  r   |sigma|   tenor  |  Call/Put  | AsianValue |AsiaExpected|PlainVanilla|   PVCPU    | Knock-Out  |  Knock-In  | K-Out+K-In | Lookback |AsianLkBkK-O|" << endl;
    output << "----|------|------|-----|----------|------------|------------|------------|------------|------------|------------|------------|------------|----------|------------|" << std::endl;
    output << setw(3) << option.spot << " | ";
    output << setw(4) << option.strike << " | ";
    output << setw(4) << option.r << " | ";
    output << setw(3) << option.sigma << " | ";
    output << setw(3) << option.tenor << " | ";
    output << setw(10) << (option.type == genericOption<Real>::Call ? "Call" : "Put") << " | ";
    output << setw(10) << option.valueAsian << " | ";
    output << setw(10) << option.golden << " | ";
    output << setw(10) << option.valuePlainVanilla << " | ";
    output << setw(10) << option.valuePlainVanillaCPU << " | ";
    output << setw(10) << option.valueKnockout << " | ";
    output << setw(10) << option.valueKnockin << " | ";
    output << setw(10) << option.valueKnockin + option.valueKnockout << " | ";  
    output << setw(10) << option.valueLookback  << " | ";  
    output << setw(8) << option.valueALK << " | ";
    output << "\nTotal Time Spent on  GPU :" << elapsedTime;
    
    shrLog("%s\n\n", output.str().c_str());

    // Check result
    if (fabs(option.valueAsian - option.golden) > tolerance)
    {
        shrLogEx(LOGBOTH | ERRORMSG, 0, "computed result (%e) does not match expected result (%e).\n", option.valueAsian, option.golden);
        pass = false;
    }
    else
    {
        pass = true;
    }

    // Print results
#ifdef GPU_PROFILING
    shrLogEx(LOGBOTH | MASTER, 0, "MonteCarloSinglegenericOptionP, Performance = %.4f sims/s, Time = %.5f s, NumDevsUsed = %u, Blocksize = %u\n", 
            numSims / elapsedTime, elapsedTime, 1, threadBlockSize);
#endif
        
    return pass;
}

// Explicit template instantiation
template struct Test<float>;
template struct Test<double>;
