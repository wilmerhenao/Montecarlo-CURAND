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

#ifndef GENERICOPTION_H
#define GENERICOPTION_H

template <typename Real>
class genericOption{
    public:
    enum CallPut {Call, Put};

    // Parameters
    Real spot;
    Real strike;
    Real r;
    Real sigma;
    Real tenor;
    Real dt;
    Real barrier;

    // Value
    Real golden;
    Real valueAsian;
    Real valuePlainVanilla;
    Real valuePlainVanillaCPU;
    Real valueKnockout;
    Real valueKnockin;
    Real valueLookback;
    Real valueALK;

    // Option type
    CallPut type;
};

#endif
