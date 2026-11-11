cmake_minimum_required(VERSION 3.22)
project(SkellAdlibLab VERSION 0.9.1 LANGUAGES C CXX)

include(FetchContent)
if(DEFINED JUCE_PATH AND EXISTS "${JUCE_PATH}/CMakeLists.txt")
  add_subdirectory("${JUCE_PATH}" JUCE)
else()
  FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.12 GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(JUCE)
endif()

# The VST3 release retains its bundled SkellCore. The optional screenshot
# executable uses a pinned upstream commit that exposes SkellCore::UIRenderer.
option(SKELL_ADLIBLAB_BUILD_UI_RENDERER "Build real JUCE editor screenshots" OFF)
set(SKELL_ADLIBLAB_RENDERER_SKELLCORE_REV "438bc74a86a40bf54cfa8089832a477822c8ccda"
    CACHE STRING "Immutable SkellCore renderer commit")
if(SKELL_ADLIBLAB_BUILD_UI_RENDERER)
  FetchContent_Declare(SkellCoreRendererDependency
    GIT_REPOSITORY https://github.com/skellyruno/skell-vst-template.git
    GIT_TAG ${SKELL_ADLIBLAB_RENDERER_SKELLCORE_REV}
    SOURCE_SUBDIR SkellCore)
  FetchContent_MakeAvailable(SkellCoreRendererDependency)
  if(NOT TARGET SkellCore::UIRenderer)
    message(FATAL_ERROR "SkellCore::UIRenderer unavailable in pinned revision")
  endif()
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SkellCore/CMakeLists.txt")
  add_subdirectory(SkellCore)
endif()

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
if(TARGET SkellCoreDSP)
  target_link_libraries(SkellAdlibLab PRIVATE SkellCore::DSP)
endif()
if(TARGET SkellCoreUI)
  target_link_libraries(SkellAdlibLab PRIVATE SkellCore::UI)
endif()

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
