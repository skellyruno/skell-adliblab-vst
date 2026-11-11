cmake_minimum_required(VERSION 3.22)
project(SkellAdlibLab VERSION 0.9.3 LANGUAGES C CXX)

include(FetchContent)
if(DEFINED JUCE_PATH AND EXISTS "${JUCE_PATH}/CMakeLists.txt")
  add_subdirectory("${JUCE_PATH}" JUCE)
else()
  FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.12 GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(JUCE)
endif()

# All shipped code, GuidedTour, SkellWorld and the development renderer use
# the SAME immutable SkellCore checkout. The older bundled SkellCore copy is
# intentionally not added to the build.
option(SKELL_ADLIBLAB_BUILD_UI_RENDERER "Build real JUCE editor screenshots" OFF)
set(SKELL_ADLIBLAB_SKELLCORE_REV "b0c79ff75cf7f1ca7e55ad231a9a9013f7c9e6c3"
    CACHE STRING "Tested immutable SkellCore revision")
FetchContent_Declare(SkellCoreDependency
  GIT_REPOSITORY https://github.com/skellyruno/skellcore-vst-template.git
  GIT_TAG ${SKELL_ADLIBLAB_SKELLCORE_REV}
  GIT_SHALLOW FALSE
  SOURCE_SUBDIR SkellCore)
FetchContent_MakeAvailable(SkellCoreDependency)
if(NOT TARGET SkellCore::DSP OR NOT TARGET SkellCore::UI OR NOT TARGET SkellCore::UIRenderer)
  message(FATAL_ERROR "Pinned SkellCore must expose DSP, UI and UIRenderer targets")
endif()
message(STATUS "Skell AdlibLab SkellCore pinned commit: ${SKELL_ADLIBLAB_SKELLCORE_REV}")

juce_add_plugin(SkellAdlibLab
  COMPANY_NAME "Skell Audio"
  BUNDLE_ID "uno.skellyr.adliblab"
  PLUGIN_MANUFACTURER_CODE SkAu
  PLUGIN_CODE AdL1
  FORMATS VST3
  PRODUCT_NAME "Skell AdlibLab"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE)

juce_add_binary_data(AdlibLabAssets SOURCES Assets/adliblab-symbol.svg)
juce_generate_juce_header(SkellAdlibLab)
file(GLOB_RECURSE ADLIB_SOURCES CONFIGURE_DEPENDS
    "Source/*.cpp" "Source/*.h")
target_sources(SkellAdlibLab PRIVATE ${ADLIB_SOURCES})
target_include_directories(SkellAdlibLab PRIVATE Source)
target_compile_features(SkellAdlibLab PRIVATE cxx_std_17)
target_compile_definitions(SkellAdlibLab PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(SkellAdlibLab PRIVATE
    AdlibLabAssets
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)
target_link_libraries(SkellAdlibLab PRIVATE SkellCore::DSP SkellCore::UI)

# This development executable links the REAL JUCE processor/editor and
# plugin assets. It is not a standalone release or an additional plugin format.
if(SKELL_ADLIBLAB_BUILD_UI_RENDERER)
  add_executable(SkellAdlibLabUIRenderer tests/ui_renderer.cpp)
  target_include_directories(SkellAdlibLabUIRenderer PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/SkellAdlibLab_artefacts/JuceLibraryCode")
  target_compile_features(SkellAdlibLabUIRenderer PRIVATE cxx_std_17)
  target_compile_definitions(SkellAdlibLabUIRenderer PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
  target_link_libraries(SkellAdlibLabUIRenderer PRIVATE
    SkellAdlibLab AdlibLabAssets SkellCore::UIRenderer
    SkellCore::UI SkellCore::DSP
    juce::juce_audio_utils juce::juce_dsp juce::juce_gui_extra)
endif()
