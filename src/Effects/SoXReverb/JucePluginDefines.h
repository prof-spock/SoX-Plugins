/**
 * @file
 * The <C>JucePluginDefines</C> sets up effect parameters for the JUCE
 * framework.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-10
 */

#pragma once

// Plugin Settings
#define _PluginPathName                  eu.tensi.SoXPlugins.Reverb

#define JucePlugin_Name                  "SoX Reverb"
#define JucePlugin_Desc                  "reverb effect from SoX"
#define JucePlugin_PluginCode            0x72657662

// settings for the different plugin kinds
#define JucePlugin_AAXIdentifier         _PluginPathName
#define JucePlugin_AAXProductId          JucePlugin_PluginCode
#define JucePlugin_AUExportPrefix        SoXReverbAU
#define JucePlugin_AUExportPrefixQuoted  "SoXReverbAU"
#define JucePlugin_AUSubType             JucePlugin_PluginCode
#define JucePlugin_CFBundleIdentifier    _PluginPathName
#define JucePlugin_IAAName               "SoX: Reverb"
#define JucePlugin_IAASubType            JucePlugin_PluginCode
#define JucePlugin_RTASProductId         JucePlugin_PluginCode
#define JucePlugin_VSTUniqueID           JucePlugin_PluginCode
