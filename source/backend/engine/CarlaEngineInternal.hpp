/*
 * Carla Plugin Host
 * Copyright (C) 2011-2014 Filipe Coelho <falktx@falktx.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * For a full copy of the GNU General Public License see the doc/GPL.txt file.
 */

#ifndef CARLA_ENGINE_INTERNAL_HPP_INCLUDED
#define CARLA_ENGINE_INTERNAL_HPP_INCLUDED

#include "CarlaEngine.hpp"
#include "CarlaEngineOsc.hpp"
#include "CarlaEngineThread.hpp"

#include "CarlaMutex.hpp"
#include "LinkedList.hpp"

// -----------------------------------------------------------------------
// Engine helper macro, sets lastError and returns false/NULL

#define CARLA_SAFE_ASSERT_RETURN_ERR(cond, err)  if (cond) pass(); else { carla_safe_assert(#cond, __FILE__, __LINE__); setLastError(err); return false; }
#define CARLA_SAFE_ASSERT_RETURN_ERRN(cond, err) if (cond) pass(); else { carla_safe_assert(#cond, __FILE__, __LINE__); setLastError(err); return nullptr; }

// -----------------------------------------------------------------------

CARLA_BACKEND_START_NAMESPACE

#if 0
} // Fix editor indentation
#endif

// -----------------------------------------------------------------------
// Maximum pre-allocated events for rack and bridge modes

const unsigned short kMaxEngineEventInternalCount = 512;

// -----------------------------------------------------------------------
// AbstractEngineBuffer

struct AbstractEngineBuffer {
    AbstractEngineBuffer(const uint32_t /*bufferSize*/) {}
    virtual ~AbstractEngineBuffer() noexcept {}

    virtual void clear() noexcept = 0;
    virtual void resize(const uint32_t bufferSize) = 0;

    virtual bool connect(CarlaEngine* const engine, const int groupA, const int portA, const int groupB, const int port) noexcept = 0;
    virtual bool disconnect(const uint connectionId) noexcept = 0;

    virtual const char* const* getConnections() const = 0;
};

// -----------------------------------------------------------------------
// InternalAudio

struct EngineInternalAudio {
    bool isPatchbay; // can only be patchbay or rack mode
    bool isReady;

    uint inCount;
    uint outCount;

    AbstractEngineBuffer* buffer;

    EngineInternalAudio() noexcept
        : isPatchbay(false),
          isReady(false),
          inCount(0),
          outCount(0),
          buffer(nullptr) {}

    ~EngineInternalAudio() noexcept
    {
        CARLA_SAFE_ASSERT(! isReady);
        CARLA_SAFE_ASSERT(buffer == nullptr);
    }

    void clear() noexcept
    {
        isReady  = false;
        inCount  = 0;
        outCount = 0;

        CARLA_SAFE_ASSERT_RETURN(buffer != nullptr,);
        delete buffer;
        buffer = nullptr;
    }

    void create(const uint32_t bufferSize);
    void initPatchbay() noexcept;

    CARLA_DECLARE_NON_COPY_STRUCT(EngineInternalAudio)
};

// -----------------------------------------------------------------------
// InternalEvents

struct EngineInternalEvents {
    EngineEvent* in;
    EngineEvent* out;

    EngineInternalEvents() noexcept
        : in(nullptr),
          out(nullptr) {}

    ~EngineInternalEvents() noexcept
    {
        CARLA_SAFE_ASSERT(in == nullptr);
        CARLA_SAFE_ASSERT(out == nullptr);
    }

    CARLA_DECLARE_NON_COPY_STRUCT(EngineInternalEvents)
};

// -----------------------------------------------------------------------
// InternalTime

struct EngineInternalTime {
    bool playing;
    uint64_t frame;

    EngineInternalTime() noexcept
        : playing(false),
          frame(0) {}

    CARLA_DECLARE_NON_COPY_STRUCT(EngineInternalTime)
};

// -----------------------------------------------------------------------
// NextAction

enum EnginePostAction {
    kEnginePostActionNull,
    kEnginePostActionZeroCount,
    kEnginePostActionRemovePlugin,
    kEnginePostActionSwitchPlugins
};

struct EngineNextAction {
    EnginePostAction opcode;
    unsigned int pluginId;
    unsigned int value;
    CarlaMutex   mutex;

    EngineNextAction() noexcept
        : opcode(kEnginePostActionNull),
          pluginId(0),
          value(0) {}

    ~EngineNextAction() noexcept
    {
        CARLA_SAFE_ASSERT(opcode == kEnginePostActionNull);
    }

    void ready() const noexcept
    {
        mutex.lock();
        mutex.unlock();
    }

    CARLA_DECLARE_NON_COPY_STRUCT(EngineNextAction)
};

// -----------------------------------------------------------------------
// EnginePluginData

struct EnginePluginData {
    CarlaPlugin* plugin;
    float insPeak[2];
    float outsPeak[2];

    void clear() noexcept
    {
        plugin = nullptr;
        insPeak[0] = insPeak[1] = 0.0f;
        outsPeak[0] = outsPeak[1] = 0.0f;
    }
};

// -----------------------------------------------------------------------
// CarlaEngineProtectedData

struct CarlaEngineProtectedData {
    CarlaEngineOsc    osc;
    CarlaEngineThread thread;

    const CarlaOscData* oscData;

    EngineCallbackFunc callback;
    void*              callbackPtr;

    FileCallbackFunc fileCallback;
    void*            fileCallbackPtr;

    unsigned int hints;
    uint32_t     bufferSize;
    double       sampleRate;

    bool         aboutToClose;    // don't re-activate thread if true
    unsigned int curPluginCount;  // number of plugins loaded (0...max)
    unsigned int maxPluginNumber; // number of plugins allowed (0, 16, 99 or 255)
    unsigned int nextPluginId;    // invalid if == maxPluginNumber

    CarlaString    lastError;
    CarlaString    name;
    EngineOptions  options;
    EngineTimeInfo timeInfo;

    EnginePluginData* plugins;

#ifndef BUILD_BRIDGE
    EngineInternalAudio  bufAudio;
#endif
    EngineInternalEvents bufEvents;
    EngineInternalTime   time;
    EngineNextAction     nextAction;

    // -------------------------------------------------------------------

    CarlaEngineProtectedData(CarlaEngine* const engine);
    ~CarlaEngineProtectedData() noexcept;

    // -------------------------------------------------------------------

    void doPluginRemove() noexcept;
    void doPluginsSwitch() noexcept;
    void doNextPluginAction(const bool unlock) noexcept;

    // -------------------------------------------------------------------

#ifndef BUILD_BRIDGE
    // the base, where plugins run
    void processRack(float* inBufReal[2], float* outBuf[2], const uint32_t nframes, const bool isOffline);

    // extended, will call processRack() in the middle
    void processRackFull(float** const inBuf, const uint32_t inCount, float** const outBuf, const uint32_t outCount, const uint32_t nframes, const bool isOffline);
#endif

    // -------------------------------------------------------------------

    class ScopedActionLock
    {
    public:
        ScopedActionLock(CarlaEngineProtectedData* const data, const EnginePostAction action, const unsigned int pluginId, const unsigned int value, const bool lockWait) noexcept;
        ~ScopedActionLock() noexcept;

    private:
        CarlaEngineProtectedData* const fData;

        CARLA_PREVENT_HEAP_ALLOCATION
        CARLA_DECLARE_NON_COPY_CLASS(ScopedActionLock)
    };

    // -------------------------------------------------------------------

#ifdef CARLA_PROPER_CPP11_SUPPORT
    CarlaEngineProtectedData() = delete;
    CARLA_DECLARE_NON_COPY_STRUCT(CarlaEngineProtectedData)
#endif
};

// -----------------------------------------------------------------------

CARLA_BACKEND_END_NAMESPACE

#endif // CARLA_ENGINE_INTERNAL_HPP_INCLUDED
