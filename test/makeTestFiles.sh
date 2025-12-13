#!/bin/bash
# generate SOX test files for cancellation test

# ============================================================

function _popd () {
    # popd directory stack without output
    popd >/dev/null
}

#--------------------

function _pushd () {
    # pushd directory stack without output
    pushd $1 >/dev/null
}

#--------------------
#--------------------

function performTest() {
    # Performs test on source file given by <sourceStem>
    # for effect kind <category> with <soxCommands>

    sourceStem=$1
    category=$2

    echo "--- ${category} (${sourceStem}) test --- ${soxCommands[@]}"
    sourceFile=${sourceStem}${soxFileType}
    targetFile=${sourceStem}-${category}test${soxFileType}
    ${sox} ${sourceFile} ${targetFile} "${soxCommands[@]}"
}

# --------------------

function makeBurstFile() {
    # Makes file from <targetFileStem> with two bursts on
    # <soxCommands> for <durationInSeconds>

    targetFileStem=$1
    isChannelB=$2

    noiseFile=part${soxFileType}
    ${sox} -n ${soxFileSettings[@]} ${noiseFile} "${soxCommands[@]}"

    if [ ${isChannelB} != "true" ]; then
        fileList="-v 0.2 ${noiseFile} -v 0.6 ${noiseFile}";
    else
        fileList="-v 0.6 ${noiseFile} -v 0.2 ${noiseFile}";
    fi

    fileList="${fileList} ${fileList}"
    echo "--- burst file ${targetFileStem} --- ${soxCommandsSuffix[@]}"
    ${sox} ${fileList} ${targetFileStem}${soxFileType} \
           "${soxCommandsSuffix[@]}"
}

# ============================================================

_pushd "$(dirname -- "$0")"
scriptDirectory="$PWD"
_popd

# --- sox command path ---
sox=sox

# --- internal configuration ---
soxFileType=".flac"
soxFileSettings=(-b 24 -r 44100)
durationInSeconds="16"
durationInSecondsByFour="4"
pulseLength="0.1"
pulseFrequency="10"
durationInSecondsByFourMinusPLMinus1="2.9"

# ==========================
# === prepare test files ===
# ==========================

_pushd ${scriptDirectory}

echo "=== preparing test files ==="
soxCommandsSuffix=(fade 0.1 -0 pad 1 1)

# -- a stereo file with noise on one channel and sine on the other
# -- (both with two bursts)
soxCommands=(synth ${durationInSecondsByFour} pinknoise)
makeBurstFile channelA false
soxCommands=(synth ${durationInSecondsByFour} sine 135)
makeBurstFile channelB true
${sox} -M channelA${soxFileType} channelB${soxFileType} noise${soxFileType}

# -- sine file with a sweep --
soxCommands=(synth ${durationInSeconds} sine 100-5000 gain -6 remix 1 1)
targetFile="sine-sweep${soxFileType}"
${sox} -n ${soxFileSettings[@]} ${targetFile} \
          ${soxCommands[@]} ${soxCommandsSuffix[@]}

# -- four square bursts
soxCommands=(synth ${pulseLength} square ${pulseFrequency} gain -6 remix 1 1 \
             pad 1 ${durationInSecondsByFourMinusPLMinus1} repeat 3)
targetFile="pulse${soxFileType}"
${sox} -n ${soxFileSettings[@]} ${targetFile} \
          ${soxCommands[@]} ${soxCommandsSuffix[@]}

# -- four notes
notes=(E2 A2 D3 G3)
command=(${sox} -n ${soxFileSettings[@]})
soxCommand=(synth ${durationInSecondsByFour} pluck)

for note in ${notes[@]}; do
    targetFile="note-${note}${soxFileType}"
    noteFileList="${noteFileList} ${targetFile}"
    ${command[@]} ${targetFile} ${soxCommand[@]} ${note}
done

targetFile="notes${soxFileType}"
${sox} ${soxFileSettings[@]} ${noteFileList[@]} \
       ${targetFile} ${soxCommandsSuffix[@]}

# =====================
# === perform tests ===
# =====================

echo "=== performing tests ==="

soxCommands=(allpass 1050 3q)
performTest noise allpass

soxCommands=(band 1222 2.3q)
performTest noise band

soxCommands=(bandpass -c 520 2o)
performTest noise bandpass

soxCommands=(bandreject 1531 1q)
performTest noise bandreject

soxCommands=(bass -1.23 536 2q)
performTest sine-sweep bass

soxCommands=(biquad 0.3 -0.5 0.3 2 -0.4 0.1)
performTest noise biquad

soxCommands=(chorus 1.0 0.5
                      50 0.75 1.25 1 -s
                      75 0.5  2.5  2 -t
                     100 0.25 5    4 -s)
performTest notes chorus
performTest pulse chorus

soxCommands=(compand 0.02,0.15 -4:-60,0,-20 +4.5)
performTest noise compander

soxCommands=(echo 0.9 0.6
                    200 0.75
                    400 0.5
                    800 0.25)
performTest pulse echo
performTest notes echo
performTest noise echo

soxCommands=(echos 0.9 0.6
                      300 0.75
                      600 0.5
                     1200 0.25)
performTest pulse echos
performTest notes echos
performTest noise echos

soxCommands=(equalizer 520 2o -3)
performTest sine-sweep equalizer

soxCommands=(gain -7.5)
performTest noise gain

soxCommands=(highpass -1 2750)
performTest sine-sweep highpass

soxCommands=(lowpass -2 250 2o)
performTest noise lowpass

soxCommands=(overdrive 3 40)
performTest noise overdrive

soxCommands=(mcompand "0.02,0.15 4:-60,0,-20 +4.5")
soxCommands+=(1000 "0.15,0.4 -20,0,-10 -2")
performTest noise mcompander

soxCommands=(phaser 0.6 0.66 3 0.6 0.5 -t)
performTest noise phaser

soxCommands=(reverb 60 22 87.5 34.88 20 -3)
performTest noise reverb

soxCommands=(treble +2.75 5200 2o)
performTest noise treble

soxCommands=(tremolo 0.395 94.67)
performTest sine-sweep tremolo

_popd
