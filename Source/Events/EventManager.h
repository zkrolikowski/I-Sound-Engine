//
// Created by zack on 10/19/21.
//

#ifndef I_SOUND_ENGINE_EVENTMANAGER_H
#define I_SOUND_ENGINE_EVENTMANAGER_H

#include "Event.h"
#include "Filters/Filter.h"
#include <unordered_map>
#include "EventParser.h"
#include "RealTimeParameters/GameObjectManager.h"
#include "Actions/Action.h"
#include "Actions/PostEvent.h"
#include <mutex>

constexpr int buffSize =  2048;

class EventManager
{
public:
    EventManager(PackageManager& soundData, GameObjectManager& objectManager);
    ~EventManager();
    /*!
     * TEMPERARY untill a json or such system is set up to read events from
     * @tparam T all filters to add types
     * @param filters Filters to add
     * @return event ID
     */
    template<typename... T>
    int AddEvent(Filter<float>* filter, T... filters)
    {
        Event* newEvent = new Event(0);
        return AddEvent(newEvent, filter, filters...);
    }

    template<typename... T>
    int AddEvent(uint64_t gameObjectId, Filter<float>* filter, T... filters)
    {
        Event* newEvent = new Event(gameObjectId);
        return AddEvent(newEvent, filter, filters...);
    }

    int AddEvent(uint64_t id);
    int AddEvent(const std::string& name);

    int AddEvent(uint64_t id, uint64_t gameObjectId);
    int AddEvent(const std::string& name, uint64_t gameObjectId);

    void StopEvent(uint64_t eventID);

    void Update();

    void ParseEvents(const std::string& path);

    /*!
     * Fills buffer with data from  all data
     * @param numSamples
     * @param buffer
     * @return
     */
    int GetSamplesFromAllEvents(int numSamples, Frame<float>* buffer);

private:

    template<typename... T>
    int AddEvent(Event* event, Filter<float>* filter, T... filters)
    {
        event->AddFilter(filter);
        return AddEvent(event, filters...);
    }

    int AddEvent(Event* event)
    {
        std::lock_guard lock(m);
        PostEventAction* newEvent = new PostEventAction(nextActionID, event, eventID);
        actionList.push_back(newEvent);

        ++eventID;
        ++nextActionID;

        return eventID;
//        return eventID;
    }

    int nextActionID;
    int lastActedID;

    std::vector<Action*> actionList;

    int eventID;
    std::unordered_map<int, Event*> events; //!< TODO  MAKE THREAD SAFE
    //std::unordered_map<uint64_t, SoundData>& soundData;
    PackageManager& soundData;
    std::mutex m;

    float* leftLocalBuffer;
    float* rightLocalBuffer;

    GameObjectManager& objectManager;
    GameObject globalGameObject;
    //Frame<float> localBuffer[buffSize];
    EventParser eventParser;
};


#endif //I_SOUND_ENGINE_EVENTMANAGER_H
