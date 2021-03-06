//
// Created by zack on 10/27/21.
//

#ifndef I_SOUND_ENGINE_CONVOLUTIONFREQ_H
#define I_SOUND_ENGINE_CONVOLUTIONFREQ_H

#include "Filter.h"
#include "HRIRCalculator.h"
#include "pffft.hpp"
#include <deque>
#include <chrono>
#include "RingDeque.h"

constexpr int BlockSize = 512;

class ConvolutionFreq : public Filter<float>
{
public:
    ConvolutionFreq(int size, HRIRCalculator<float>& HRIR) : fft(BlockSize * 2),
    HRIR(HRIR),
    Overlap(static_cast<int>((GameObject::GetParamStatic<float>("Overlap")))),
    leftOverlap( 1024,((BlockSize * 2) - Overlap) - 1),
    rightOverlap( 1024,((BlockSize * 2) - Overlap) - 1)
    {
        currentComplex = new std::complex<float>[size* 2]();

        leftIR = new float[size*2]();
        rightIR = new float[size*2]();
        rightS = new float[size*2]();
        leftS = new float[size*2]();
        leftComplex = new std::complex<float>[size*2]();
        rightComplex = new std::complex<float>[size*2]();
        leftOld = new float[size*2]();
        rightOld = new float[size*2]();
    }

    virtual ~ConvolutionFreq()
    {
        delete [] currentComplex;
        delete [] leftIR;
        delete [] rightIR;
        delete [] rightS;
        delete [] leftS;
        delete [] leftComplex;
        delete [] rightComplex;
        delete [] rightOld;
        delete [] leftOld;
    }

    virtual int GetNextSamples(int numSamples, float* left, float* right, const GameObject& obj)
    {
        auto start = std::chrono::steady_clock::now();

        // Source signal never changes
        memset(leftS + blockSize, 0,  BlockSize * sizeof(float));
        memcpy(leftS, left, numSamples * sizeof(float));
        fft.forwardToInternalLayout(leftS, reinterpret_cast<float *>(leftComplex));

        float crossFade = (obj.GetParam<float>("CrossFade"));
        if(crossFade != 0.0f)
        {
            memcpy(leftOld, left, sizeof(float) * numSamples);
            memcpy(rightOld, right, sizeof(float) * numSamples);

            GetNextSamplesFromBuffer(numSamples, leftOld, rightOld, obj, false);
            HRIR.GetNextSamples(BlockSize * 2, leftIR, rightIR, obj);
            GetNextSamplesFromBuffer(numSamples, left, right, obj, true);

            for(int i = 0; i < numSamples; i++)
            {
                // incorrect crossfade
                //left[i] = lerp(leftOld[i], left[i], static_cast<float>(i)/numSamples);
                //right[i] = lerp(rightOld[i], right[i], static_cast<float>(i)/numSamples);

                // linear crossfade
                left[i]  = ((1 - static_cast<float>(i) / numSamples) * leftOld[i]) +
                           ((static_cast<float>(i) / numSamples)     * left[i]);
                right[i] = ((1 - static_cast<float>(i) / numSamples) * rightOld[i]) +
                           ((static_cast<float>(i) / numSamples)     * right[i]);

                // Easing fade - sounds way worse than linear??
//                left[i] =  (this->EaseInQuart(1.0f - (static_cast<float>(i) / numSamples)) * leftOld[i]) +
//                           (this->EaseInQuart( static_cast<float>(i) / numSamples) * left[i]);
//                right[i] = (this->EaseInQuart(1.0f - (static_cast<float>(i) / numSamples)) * rightOld[i]) +
//                           (this->EaseInQuart( static_cast<float>(i) / numSamples) * right[i]);
            }
        }
        else
        {
            HRIR.GetNextSamples(BlockSize * 2, leftIR, rightIR, obj);
            GetNextSamplesFromBuffer(numSamples, left, right, obj,  true);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<float> time = end - start;
        GameObject::SetParamStatic("HRTFLoadTemp", GameObject::GetParamStatic<float>("HRTFLoadTemp") + time.count());
        return 0;
    }

private:

    int GetNextSamplesFromBuffer(int numSamples, float* left, float* right, const GameObject& obj, bool saveOverlap)
    {
        for(int offset = 0; offset < BlockSize; offset += Overlap)
        {
            CalculateBlock(Overlap, left + offset, right + offset, obj, saveOverlap);
        }
        return 0;
    }

    void CalculateBlock(int numSamples, float* left, float* right, const GameObject& obj, bool saveOverlap)
    {
        //-----------------------------------------
        // Left ear
        //-----------------------------------------

        fft.forwardToInternalLayout(leftIR, reinterpret_cast<float *>(rightComplex));

        fft.convolve(reinterpret_cast<const float *>(leftComplex), reinterpret_cast<const float *>(rightComplex),
                     reinterpret_cast<float *>(currentComplex), 0.8f / (numSamples * 2));

        fft.inverseFromInternalLayout(reinterpret_cast<const float *>(currentComplex), leftS);

        //-----------------------------------------
        // Right ear
        //-----------------------------------------

        fft.forwardToInternalLayout(rightIR, reinterpret_cast<float *>(rightComplex));

        fft.convolve(reinterpret_cast<const float *>(leftComplex), reinterpret_cast<const float *>(rightComplex),
                     reinterpret_cast<float *>(currentComplex), 0.8f / (numSamples * 2));

        fft.inverseFromInternalLayout(reinterpret_cast<const float *>(currentComplex), rightS);

        //-----------------------------------------
        // Normalize
        //-----------------------------------------

        for(int i = 0; i < numSamples; ++i)
        {
            left[i] = (leftS[i] + leftOverlap[i]);
            right[i] = (rightS[i]  + rightOverlap[i]);
        }

        if(saveOverlap)
        {
            for (int i = 0; i < numSamples; ++i)
            {
                leftOverlap.pop_front();
                rightOverlap.pop_front();
            }

            for (int i = Overlap, j = 0; j < leftOverlap.size(); ++i, ++j)
            {
                leftOverlap[j] += leftS[i];
                rightOverlap[j] += rightS[i];
            }

            for (int i = (BlockSize * 2) - Overlap; i < BlockSize * 2; ++i)
            {
                leftOverlap.push_back(leftS[i]);
                rightOverlap.push_back(rightS[i]);
            }
        }
    }

    const int Overlap;

    float* leftIR;
    float* rightIR;
    float* leftS;
    float* rightS;

    float* leftOld;
    float* rightOld;

    //float* leftOverlap;
    //float* rightOverlap;
    std::complex<float>* leftComplex;
    std::complex<float>* rightComplex;
    std::complex<float>* currentComplex;


    RingDeque<float> leftOverlap;
    RingDeque<float> rightOverlap;

    pffft::Fft<float> fft;
    HRIRCalculator<float>& HRIR;
};

#endif //I_SOUND_ENGINE_CONVOLUTIONFREQ_H
