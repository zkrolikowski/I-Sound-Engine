//
// Created by zack on 10/27/21.
//

#ifndef I_SOUND_ENGINE_CONVOLUTIONFREQ_H
#define I_SOUND_ENGINE_CONVOLUTIONFREQ_H

#include "Filter.h"
#include "HRIRCalculator.h"
#include "pffft.hpp"
#include <deque>

constexpr int BlockSize = 512;

class ConvolutionFreq : public Filter<float>
{
public:
    ConvolutionFreq(int size, HRIRCalculator<float>& HRIR) : fft(BlockSize * 2),
    HRIR(HRIR),
    Overlap(static_cast<int>(std::any_cast<float>(GameObject::GetParam("Overlap")))),
    leftOverlap((BlockSize * 2) - Overlap),
    rightOverlap((BlockSize * 2) - Overlap)
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
    }

    virtual int GetNextSamples(int numSamples, float* left, float* right, const GameObject& obj)
    {
        float crossFade = std::any_cast<float>(obj.GetParam("CrossFade"));
        if(crossFade != 0.0f)
        {
            memcpy(leftOld, left, sizeof(float) * numSamples);
            memcpy(rightOld, right, sizeof(float) * numSamples);
            GetNextSamplesFromBuffer(numSamples, leftOld, rightOld, obj, leftOverlap, rightOverlap);
            HRIR.GetNextSamples(BlockSize * 2, leftIR, rightIR, obj);
            GetNextSamplesFromBuffer(numSamples, left, right, obj, leftOverlap, rightOverlap);

            for(int i = 0; i < numSamples; i++)
            {
                if(left[i] != leftOld[i])
                {
                    //std::cout << i << " " << left[i] << " " << leftOld[i] << " pain" << std::endl;
                }
                left[i] = lerp(leftOld[i], left[i], static_cast<float>(i)/numSamples);
                right[i] = lerp(rightOld[i], right[i], static_cast<float>(i)/numSamples);
            }
        }
        else
        {
            HRIR.GetNextSamples(BlockSize * 2, leftIR, rightIR, obj);
            GetNextSamplesFromBuffer(numSamples, left, right, obj, leftOverlap, rightOverlap);
        }
        return 0;
    }

private:

    int GetNextSamplesFromBuffer(int numSamples, float* left, float* right, const GameObject& obj, std::deque<float>& leftOverlapT, std::deque<float>& rightOverlapT)
    {
        for(int offset = 0; offset < BlockSize; offset += Overlap)
        {
            CalculateBlock(Overlap, left + offset, right + offset, obj, leftOverlapT, rightOverlapT);
        }
        return 0;
    }

    void CalculateBlock(int numSamples, float* left, float* right, const GameObject& obj, std::deque<float>& leftOverlapT, std::deque<float>& rightOverlapT)
    {
        //-----------------------------------------
        // Left ear
        //-----------------------------------------
        memset(leftS, 0, 2 * BlockSize * sizeof(float));
        memcpy(leftS, left, numSamples * sizeof(float));


        fft.forward(leftS, leftComplex);
        fft.forward(leftIR, rightComplex);

        for(int i = 0; i < BlockSize * 2; ++i)
        {
            currentComplex[i] = leftComplex[i] * rightComplex[i];
        }

        fft.inverse(currentComplex, leftS);

        //-----------------------------------------
        // Right ear
        //-----------------------------------------

        memset(rightS, 0, 2 * BlockSize * sizeof(float));
        memcpy(rightS, right, numSamples * sizeof(float));


        fft.forward(rightS, leftComplex);
        fft.forward(rightIR, rightComplex);

        for(int i = 0; i < BlockSize * 2; ++i)
        {
            currentComplex[i] = leftComplex[i] * rightComplex[i];
        }

        fft.inverse(currentComplex, rightS);

        //-----------------------------------------
        // Normalize
        //-----------------------------------------

        for(int i = 0; i < numSamples; ++i)
        {
            left[i] = (leftS[i] + leftOverlapT.front()) / (numSamples * 2);
            right[i] = (rightS[i]  + rightOverlapT.front()) / (numSamples * 2);
            left[i] *= (static_cast<float>(Overlap) / BlockSize) * 0.5f;
            right[i] *= (static_cast<float>(Overlap) / BlockSize) * 0.5f;

            leftOverlapT.pop_front();
            rightOverlapT.pop_front();
        }

        for(int i = Overlap, j = 0; j < leftOverlapT.size(); ++i, ++j)
        {
            leftOverlapT[j] += leftS[i];
            rightOverlapT[j] += rightS[i];
        }

        for(int i = (BlockSize * 2) - Overlap; i < BlockSize * 2; ++i)
        {
            leftOverlapT.push_back(leftS[i]);
            rightOverlapT.push_back(rightS[i]);
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


    std::deque<float> leftOverlap;
    std::deque<float> rightOverlap;

    pffft::Fft<float> fft;
    HRIRCalculator<float>& HRIR;
};

#endif //I_SOUND_ENGINE_CONVOLUTIONFREQ_H
