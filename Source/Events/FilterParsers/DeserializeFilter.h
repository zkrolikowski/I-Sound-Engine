//
// Created by zack on 10/22/21.
//

#ifndef I_SOUND_ENGINE_DESERIALIZEFILTER_H
#define I_SOUND_ENGINE_DESERIALIZEFILTER_H

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include "Filters/Filter.h"
#include "SoundData.h"
#include <unordered_map>
#include "ErrorList.h"
#include "AudioPackage/PackageManager.h"

class DeserializedFilter
{
public:
    //DeserializedFilter(rapidjson::Value& object)
    /*!
     * If the fitler can be created, it is
     * @param filter Fills in with the created filter
     * @param table The table of sounds info
     * @return Wether filter was made or not
     */
    virtual ErrorNum BuildFilter(Filter<float>** filter,  PackageManager& manager) = 0;
    virtual ErrorNum BuildFilter(Filter<float>** filter,  PackageManager& manager, GameObject& obj) = 0;
    virtual ~DeserializedFilter() {};
};

#endif //I_SOUND_ENGINE_DESERIALIZEFILTER_H
