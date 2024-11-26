/**
 * @file
 * The <C>JucePluginDefines</C> sets up effect parameters for the JUCE
 * framework.
 *
 * @author Dr. Thomas Tensi
 * @date   2021-01
 */

#pragma once

/* Plugin Settings */

/** the fully qualified name of the plugin */
#define _PluginPathName                  eu.tensi.SoXPlugins.Gain

/**  the simple name of the plugin */
#define JucePlugin_Name                  "SoX Gain"

/** the description of the plugin */
#define JucePlugin_Desc                  "gain effect from SoX"

/** the four byte code for the plugin 'gain' */
#define JucePlugin_PluginCode            0x6761696e

/* Settings for the Different Plugin Kinds */

/** the name of the plugin for AAX */
#define JucePlugin_AAXIdentifier         _PluginPathName

/** the four byte code for the plugin for AAX */
#define JucePlugin_AAXProductId          JucePlugin_PluginCode

/** the prefix for the plugin in the description file */
#define JucePlugin_AUExportPrefix        SoXGainAU

/** the prefix for AU plugins */
#define JucePlugin_AUExportPrefixQuoted  "SoXGainAU"

/** the four byte code for the plugin for AU */
#define JucePlugin_AUSubType             JucePlugin_PluginCode

/** the bundle path for MacOSX */
#define JucePlugin_CFBundleIdentifier    _PluginPathName

/** the name for IAA plugins */
#define JucePlugin_IAAName               "SoX: Gain"

/** the subtype for IAA plugins (four byte code) */
#define JucePlugin_IAASubType            JucePlugin_PluginCode

/** the product ID for RTAS plugins (four byte code) */
#define JucePlugin_RTASProductId         JucePlugin_PluginCode

/** the four byte unique ID for VST plugins */
#define JucePlugin_VSTUniqueID           JucePlugin_PluginCode
