cmake_minimum_required(VERSION 3.22)

# Version is needed by JUCE.
project(AudioPlugin VERSION 0.2.3)

# Adding a directory with the library/application name as a subfolder of the
# include folder is a good practice. It helps avoid name clashes later on.
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/E3Seq")


string(TIMESTAMP BUILD_DATE "%Y-%m-%d")
configure_file(
    ${INCLUDE_DIR}/Utility.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/Utility.h
)

# Adds a plugin target (that's basically what the Projucer does).
# doc: https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md
juce_add_plugin(${PROJECT_NAME}
    COMPANY_NAME KORGES # change this
    IS_SYNTH TRUE # may change this
    NEEDS_MIDI_INPUT TRUE # may change this
    NEEDS_MIDI_OUTPUT TRUE # may change this
    PLUGIN_MANUFACTURER_CODE KGES # change this
    PLUGIN_CODE E3SQ # change this
    FORMATS Standalone VST3 # may change this
    PRODUCT_NAME "E3Seq" # change this
    # VST3_CATEGORIES Generator
    # VST_NUM_MIDI_INS 1
    # VST_NUM_MIDI_OUTS 1
    # IS_MIDI_EFFECT TRUE # somehow enabling this makes Live unable to open the VST3
)

# Sets the source files of the plugin project.
target_sources(${PROJECT_NAME}
    PRIVATE
        source/PluginEditor.cpp
        source/PluginProcessor.cpp
        source/E3Sequencer.cpp
        source/Track.cpp
)

# Sets the include directories of the plugin project.
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE 
        ${CMAKE_CURRENT_BINARY_DIR}
)

# Links to all necessary dependencies. The present ones are recommended by JUCE.
# If you use one of the additional modules, like the DSP module, you need to specify it here.
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# These definitions are recommended by JUCE.
target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)



# Enables all warnings and treats warnings as errors.
# This needs to be set up only for your projects, not 3rd party
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# In Visual Studio this command provides a nice grouping of source files in "filters".
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/..)

juce_enable_copy_plugin_step(${PROJECT_NAME})