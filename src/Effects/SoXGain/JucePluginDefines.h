/**
 * @file
 * The <C>JucePluginDefines</C> sets up effect parameters for the JUCE
 * framework.
 *
 * @author Dr. Thomas Tensi
 * @date   2021-01
 */

#pragma once

// Plugin Settings
#define _PluginPathName                  eu.tensi.SoXPlugins.Gain

#define JucePlugin_Name                  "SoX Gain"
#define JucePlugin_Desc                  "gain effect from SoX"
#define JucePlugin_PluginCode            4711

// settings for the different plugin kinds
#define JucePlugin_AAXIdentifier         _PluginPathName
#define JucePlugin_AAXProductId          JucePlugin_PluginCode
#define JucePlugin_AUExportPrefix        SoXGainAU
#define JucePlugin_AUExportPrefixQuoted  "SoXGainAU"
#define JucePlugin_AUSubType             JucePlugin_PluginCode
#define JucePlugin_CFBundleIdentifier    _PluginPathName
#define JucePlugin_IAAName               "SoX: Gain"
#define JucePlugin_IAASubType            JucePlugin_PluginCode
#define JucePlugin_RTASProductId         JucePlugin_PluginCode
#define JucePlugin_VSTUniqueID           JucePlugin_PluginCode
