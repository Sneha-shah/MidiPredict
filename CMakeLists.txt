cmake_minimum_required(VERSION 3.28)
#cmake_minimum_required(VERSION 3.16)

project(MinimalPluginTemplate VERSION 0.0.1)

#This is temporarily needed due to a bug in Xcode 15:
if (CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "15.0")
        add_compile_definitions(JUCE_SILENCE_XCODE_15_LINKER_WARNING=1)
        set(CMAKE_EXE_LINKER_FLAGS "-Wl,-ld_classic" CACHE INTERNAL "")
    endif ()
endif ()

set (BaseTargetName MidiPredict)

#----------------------------------------------------------------------------
# From top-level:

#First, we'll add the CMake folder, incase we'll need to find_package later:
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

#Compile commands, useful for some IDEs like VS-Code
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

#Minimum MacOS target, set globally
if (${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
    set(CMAKE_OSX_DEPLOYMENT_TARGET 14.0 CACHE STRING "Minimum OS X deployment version" FORCE)

    #code signing to run on an iOS device:
    #    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer" CACHE STRING "" FORCE)
    #    set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "XXXXXXXXXX" CACHE STRING "" FORCE)
else ()
    set(CMAKE_OSX_DEPLOYMENT_TARGET "14.4" CACHE STRING "Minimum OS X deployment version" FORCE)
endif ()

option(UniversalBinary "Build universal binary for mac" OFF)
if (UniversalBinary)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE INTERNAL "")
endif ()

#static linking in Windows
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

#We 'require' that we need juce. If JUCE isn't found, it will revert to what's in
#CMake/Findjuce.cmake, where you can see how JUCE is brought in/configured
# JOS:
find_package(juce REQUIRED)

# add foleys_gui_magic to juce. You need to list it in `target_link_libraries` too
# juce_add_module(${CMAKE_CURRENT_LIST_DIR}/JuceLibraryCode)
# JOS: modules/foleys_gui_magic) # ../../UX/PluginGuiMagic/modules/foleys_gui_magic/

# add moforte to juce. You need to list it in `target_link_libraries` too
# JOS: juce_add_module(${CMAKE_CURRENT_LIST_DIR}/modules/moforte) # ../../UX/PluginGuiMagic/modules/moforte/


#By default, we are building the provided plugin/app examples:
option(BUILD_JUCE_PROTOTYPE_EXAMPLES "Build JUCE prototype examples" OFF)

#adding any custom modules you might have:
# JOS: add_subdirectory(Modules)

if (${BUILD_JUCE_PROTOTYPE_EXAMPLES})
    #adding project folders:
    add_subdirectory(Apps)
    add_subdirectory(Plugins)
endif ()

#optionally, we're also adding the unit tests:
option(BUILD_UNIT_TESTS "Build JUCE prototype examples" OFF)
if (BUILD_UNIT_TESTS)
    enable_testing()
    add_subdirectory(Tests)
endif ()

#----------------------------------------------------------------------------

juce_add_plugin("${BaseTargetName}"
        # VERSION ...                               # Set this if the plugin version is different to the project version
        # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
        # ICON_SMALL ...
        COMPANY_NAME "MyCompany"
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE Juce
        PLUGIN_CODE MiPr
        FORMATS AU Standalone
        PRODUCT_NAME "MidiPredict")

target_sources(${BaseTargetName} PRIVATE
        Source/PluginProcessor.cpp
        Source/FoleysSynth.cpp
        Source/SineWaveSound.cpp
        Source/SineWaveVoice.cpp
        Source/SynthAudioSource.cpp)

target_compile_definitions(${BaseTargetName}
        PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0)

target_link_libraries(${BaseTargetName} PRIVATE
        juce_audio_utils
        juce_midi_ci
        juce_recommended_config_flags
        juce_recommended_lto_flags
        juce_recommended_warning_flags)
