/**
 * @file
 * The <C>JucePluginDefines</C> sets up effect parameters for the JUCE
 * framework.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-09
 */

#pragma once

// Plugin Settings
#define _PluginPathName                  eu.tensi.SoXPlugins.PhaserAndTremolo
#define JucePlugin_Name                  "SoX Phaser And Tremolo"
#define JucePlugin_PluginCode            4721

// settings for the different plugin kinds
#define JucePlugin_AAXIdentifier         _PluginPathName
#define JucePlugin_AAXProductId          JucePlugin_PluginCode
#define JucePlugin_AUExportPrefix        SoXPhaserAndTremoloAU
#define JucePlugin_AUExportPrefixQuoted  "SoXPhaserAndTremoloAU"
#define JucePlugin_AUSubType             JucePlugin_PluginCode
#define JucePlugin_CFBundleIdentifier    _PluginPathName
#define JucePlugin_IAAName               "SoX: PhaserAndTremolo"
#define JucePlugin_IAASubType            JucePlugin_PluginCode
#define JucePlugin_RTASProductId         JucePlugin_PluginCode
#define JucePlugin_VSTUniqueID           JucePlugin_PluginCode
