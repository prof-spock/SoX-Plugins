\mainpage SoXPlugins - SoX Effects for DAWs
<DIV>
    <B>author:</B> Dr. Thomas Tensi<BR>
    <B>date:</B> November 2022
</DIV>

<H1>Overview</H1>

The SoXPlugins software package provides plugins for being used in
digital audio workstations (DAW); they implement some of the more
prominent audio processing effects from SoX as DAW audio
plugins. <U><A HREF="https://sox.sourceforge.net">SoX</A></U> is a
command line audio processing tool for Unix, Windows and Mac OS that
transforms source audio files in several formats into other audio
files.  It provides several standard audio effects (like e.g. filters
or reverb) in good quality and with a transparent, open-source
implementation.

This doxygen documentation gives a detailed description of the C++
sources for the SoX plugins.  Note that the implementation is
completely free, open-source, platform-neutral and based on the <U><A
HREF="https://juce.com">JUCE audio framework</A></U>.  Currently only
plugin versions as VST3 under Windows 10, VST3 and AU under MacOSX
(x86_64) and VST3 under Linux (x86_64) are provided, but porting to
other targets should be straightforward, since building is supported
by a platform-neutral CMAKE build file.

The effects provided here are a complete rewrite in C++ of the
original SoX algorithms producing <EM>(bit-exact) identical</EM>
renderings in the DAW.  This can easily be checked by rendering some
audio externally with SoX and internally with the plugins and
subtracting the results.  Apart from roundoff errors (SoX often uses
32bit integer processing, while the SoXPlugins always uses double
floating point processing) the results cancel out with typically a
residual noise of -140dBFS.

<H1>Available Effects From SoX</H1>

The following effects are available in this package:
<UL>

  <LI><B>allpass</B> (in <A
      HREF="namespace_so_x_plugins_1_1_effects_1_1_so_x_filter.html">SoXFilter</A>):
      a biquad allpass filter two-poled with filter frequency and the
      filter bandwith (in several units),</LI>

  <LI><B>band</B> (in <A
      HREF="namespace_so_x_plugins_1_1_effects_1_1_so_x_filter.html">SoXFilter</A>):
      a biquad bandpass filter with center filter frequency and the
      filter bandwith (in several units) and an option for unpitched
      audio,</LI>

  <LI><B>bandpass</B> (in <A
      HREF="namespace_so_x_plugins_1_1_effects_1_1_so_x_filter.html">SoXFilter</A>):
      a biquad filter for bandpass with center filter frequency and
      the filter bandwith (in several units),</LI>

  <LI><B>bandreject</B> (in <A
      HREF="namespace_so_x_plugins_1_1_effects_1_1_so_x_filter.html">SoXFilter</A>):
      a biquad filter for bandreject with center filter frequency and
      the filter bandwith (in several units),</LI>

  <LI><B>bass</B> (in <A
      HREF="namespace_so_x_plugins_1_1_effects_1_1_so_x_filter.html">SoXFilter</A>):
      a biquad filter for boosting or cutting bass with a shelving
      characteristics with settings for filter frequency and the
      filter bandwith (in several units),</LI>

  <LI><B>biquad</B> (in <A
      HREF="namespace_so_x_plugins_1_1_effects_1_1_so_x_filter.html">SoXFilter</A>):
      a generic biquad (iir) filter with 6 coefficients b0, b1, b2,
      a0, a1 and a2,</LI>

  <LI><B>compand</B> (in <A
      HREF="namespace_so_x_plugins_1_1_effects_1_1_so_x_compander.html">SoXCompander</A>):
      a compander with attack, release, input gain shift, threshold
      and compression and soft knee; this is a reduced version of SoX
      compand with only a simple transfer function,</LI>

  <LI><B>equalizer</B> (in <A
      HREF="namespace_so_x_plugins_1_1_effects_1_1_so_x_filter.html">SoXFilter</A>):
      a biquad filter for equalizing with settings for the pole count,
      the filter frequency and the filter bandwith (in several
      units),</LI>

  <LI><B>gain</B> (in <A
      HREF="namespace_so_x_plugins_1_1_effects_1_1_so_x_gain.html">SoXGain</A>):
      a volume changer by _exact_ decibels...,</LI>

  <LI><B>highpass</B> (in <A
      HREF="namespace_so_x_plugins_1_1_effects_1_1_so_x_filter.html">SoXFilter</A>):
      a biquad filter for highpass with settings for the pole count,
      the filter frequency and the filter bandwith (in several
      units),</LI>

  <LI><B>lowpass</B> (in <A
      HREF="namespace_so_x_plugins_1_1_effects_1_1_so_x_filter.html">SoXFilter</A>):
      a biquad filter for lowpass with settings for the pole count,
      the filter frequency and the filter bandwith (in several
      units),</LI>

  <LI><B>mcompand</B> (in <A
      HREF="namespace_so_x_plugins_1_1_effects_1_1_so_x_compander.html">SoXCompander</A>):
      a multiband compander with a Linkwitz-Riley crossover filter and
      for each band a compander with attack, release, input gain
      shift, threshold and compression and soft knee; again the
      companders only allow a simple transfer function,</LI>

  <LI><B>overdrive</B> (in <A
      HREF="namespace_so_x_plugins_1_1_effects_1_1_so_x_overdrive.html">SoXOverdrive</A>):
      a simple tanh distortion with gain and colour
      specification,</LI>

  <LI><B>phaser</B> (in <A
      HREF="namespace_so_x_plugins_1_1_effects_1_1_so_x_phaser_and_tremolo.html">SoXPhaserAndTremolo</A>):
      a phaser effect with sine or triangle modulation,</LI>

  <LI><B>reverb</B> (in <A
      HREF="namespace_so_x_plugins_1_1_effects_1_1_so_x_reverb.html">SoXReverb</A>):
      a reverb effect (based on Freeverb) with several parameters for
      the room (like size and HF damping) as well as a possible
      predelay,</LI>

  <LI><B>treble</B> (in <A
      HREF="namespace_so_x_plugins_1_1_effects_1_1_so_x_filter.html">SoXFilter</A>):
      a biquad filter for boosting or cutting treble with a shelving
      characteristics with settings for filter frequency and the
      filter bandwith (in several units), and</LI>

  <LI><B>tremolo</B> (in <A
      HREF="namespace_so_x_plugins_1_1_effects_1_1_so_x_phaser_and_tremolo.html">SoXPhaserAndTremolo</A>):
      a tremolo effect with sine modulation using a double-sideband
      suppressed carrier modulation</LI>

</UL>