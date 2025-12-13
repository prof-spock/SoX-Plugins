@ECHO OFF
REM generate SOX test files for cancellation test

SETLOCAL ENABLEDELAYEDEXPANSION
SET scriptDirectory=%~dp0.

SET configuration=Release
IF NOT "%1"=="" SET configuration=%1

REM --- sox command path ---
SET sox=CALL sox

REM --- internal configuration ---
SET soxFileType=.flac
SET soxFileSettings=-b 24 -r 44100
SET durationInSeconds=16
SET /A durationInSecondsBy4="%durationInSeconds% / 4"
SET pulseLength=0.04
SET pulseFrequency=400
SET durationInSecondsBy4MinusPLMinus1=2.96

REM ==========================
REM === prepare test files ===
REM ==========================

PUSHD %scriptDirectory%

ECHO === preparing test files ===

SET soxCommandsSuffix=fade 0.1 -0 pad 1 1
REM -- a stereo file with noise on one channel and sine on the other
REM -- (both with two bursts)
SET soxCommands=synth %durationInSecondsBy4% pinknoise
CALL :makeBurstFile channelA false
SET soxCommands=synth %durationInSecondsBy4% sine 135
CALL :makeBurstFile channelB true
%sox% -M channelA%soxFileType% channelB%soxFileType% noise%soxFileType%

REM -- sine file with a sweep --
SET soxCommands=synth %durationInSeconds% sine 100-5000 gain -6 remix 1 1
%sox% -n %soxFileSettings% sine-sweep%soxFileType% %soxCommands% %soxCommandsSuffix%

REM -- four triangle bursts
SET soxCommands=synth %pulseLength% noise %pulseFrequency% gain -6 remix 1 1
%sox% -n %soxFileSettings% onePulse%soxFileType% %soxCommands% pad 0.05 1

SET soxCommands=%soxCommands% pad 1 %durationInSecondsBy4MinusPLMinus1%
SET soxCommands=%soxCommands% repeat 3
%sox% -n %soxFileSettings% pulse%soxFileType% %soxCommands% %soxCommandsSuffix%

REM -- four notes --
SET notes=E2 A2 D3 G3
SET soxCommand=synth %durationInSecondsBy4% pluck
SET prefix=%sox% -n %soxFileSettings%
FOR %%n IN (%notes%) DO %prefix% note-%%n%soxFileType% %soxCommand% %%n
SET fileList=
FOR %%n IN (%notes%) DO SET fileList=!fileList! note-%%n%soxFileType%
%sox% %fileList% notes%soxFileType% %soxCommandsSuffix%

REM =====================
REM === perform tests ===
REM =====================

ECHO === performing tests ===

SET soxCommands=allpass 1050 3q
CALL :performTest noise allpass

SET soxCommands=band 1222 2.3q
CALL :performTest noise band

SET soxCommands=bandpass -c 520 2o
CALL :performTest noise bandpass

SET soxCommands=bandreject 1531 1q
CALL :performTest noise bandreject

SET soxCommands=bass -1.23 536 2q
CALL :performTest sine-sweep bass

SET soxCommands=biquad 0.3 -0.5 0.3 2 -0.4 0.1
CALL :performTest noise biquad

SET stageList=50 0.75 1.25 1 -s  75 0.5 2.5 2 -t  100 0.25 5 4 -s
SET soxCommands=chorus 1.0 0.5 %stageList%
CALL :performTest notes chorus
CALL :performTest onePulse chorus

SET soxCommands=compand 0.02,0.15 -4:-60,0,-20 +4.5
CALL :performTest noise compander

SET soxCommands=echo 0.9 0.6 200 0.75 400 0.5 800 0.25
CALL :performTest pulse echo
CALL :performTest notes echo
CALL :performTest noise echo

SET soxCommands=echos 0.9 0.6 300 0.75 600 0.5 1200 0.25
CALL :performTest pulse echos
CALL :performTest notes echos
CALL :performTest noise echos

SET soxCommands=equalizer 520 2o -3
CALL :performTest sine-sweep equalizer

SET soxCommands=gain -7.5
CALL :performTest noise gain

SET soxCommands=highpass -1 2750
CALL :performTest sine-sweep highpass

SET soxCommands=lowpass -2 250 2o
CALL :performTest noise lowpass

SET soxCommands=overdrive 3 40
CALL :performTest noise overdrive

SET soxCommands=mcompand "0.02,0.15 4:-60,0,-20 +4.5" 1000
SET soxCommands=%soxCommands% "0.15,0.4 -20,0,-10 -2"
CALL :performTest noise mcompander

SET soxCommands=phaser 0.6 0.66 3 0.4 0.5 -t
CALL :performTest noise phaser

SET soxCommands=reverb 60 22 87.5 34.88 20 -3
CALL :performTest noise reverb

SET soxCommands=treble +2.75 5200 2o
CALL :performTest noise treble

SET soxCommands=tremolo 0.395 94.67
CALL :performTest sine-sweep tremolo

POPD
ENDLOCAL
GOTO :EOF

REM ============================================================

:makeBurstFile
    REM Makes file from <targetFileStem> with two bursts on
    REM <soxCommands> for <durationInSeconds>

    SET targetFileStem=%1
    SET isChannelB=%2

    SET noiseFile=part%soxFileType%
    %sox% -n %soxFileSettings% %noiseFile% %soxCommands%

    IF "%isChannelB%"=="true" GOTO ELSE1
        SET fileList=-v 0.2 %noiseFile% -v 0.6 %noiseFile%
        GOTO ENDIF1
    :ELSE1
        SET fileList=-v 0.6 %noiseFile% -v 0.2 %noiseFile%
    :ENDIF1

    SET fileList=%fileList% %fileList%
    ECHO --- burst file %targetFileStem% --- %soxCommandsSuffix%
    %sox% %fileList% %targetFileStem%%soxFileType% %soxCommandsSuffix%
GOTO :EOF

REM --------------------

:performTest
    REM Performs test on source file given by <sourceStem>
    REM for effect kind <category> with <soxCommands>

    SET sourceStem=%1
    SET category=%2

    ECHO --- %category% (%sourceStem%) test --- %soxCommands%
    SET sourceFile=%sourceStem%%soxFileType%
    SET targetFile=%sourceStem%-%category%test%soxFileType%
    %sox% %sourceFile% %targetFile% %soxCommands%
GOTO :EOF
