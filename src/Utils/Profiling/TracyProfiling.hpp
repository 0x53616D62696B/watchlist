#pragma once

#ifdef ENABLE_PROFILING
  #include <tracy/Tracy.hpp>

  #define PROFILE_FRAME FrameMark
  #define PROFILE_FUNCTION ZoneScoped
  #define PROFILE_SCOPE(name) ZoneNamedN(name, #name, true)
  #define PROFILE_SCOPE_TEXT(name) ZoneScopedN(name)
  #define PROFILE_MESSAGE(text) TracyMessageL(text)
  #define PROFILE_THREAD(name) tracy::SetThreadName(name)
#else
  #define PROFILE_FRAME
  #define PROFILE_FUNCTION
  #define PROFILE_SCOPE(name)
  #define PROFILE_SCOPE_TEXT(name)
  #define PROFILE_MESSAGE(text)
  #define PROFILE_THREAD(name)
#endif
