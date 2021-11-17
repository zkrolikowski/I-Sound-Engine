//
// Created by zack on 10/19/21.
//

#ifndef I_SOUND_ENGINE_FILTER_H
#define I_SOUND_ENGINE_FILTER_H

#include "AudioFrame.h"
#include "RealTimeParameters/GameObject.h"

template<typename sampleType>
class Filter
{
public:
    virtual ~Filter() {}
    /*!
     * Fills a buffer with audio samples, if no audio data is available zeros are filled
     * @param numSamples Number of samples to fill buffer with
     * @param buffer Buffer to fill
     * @return Number of samples filled
     */
    virtual int GetNextSamples(int numSamples, float* left, float* right, const GameObject& obj) = 0;
};

#endif //I_SOUND_ENGINE_FILTER_H
