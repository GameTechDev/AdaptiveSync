@echo off

robocopy "C:\Program Files\WindowsApps\AppUp.IntelGraphicsExperience_1.100.1725.0_x64__8j3eq9eme6ctt\GCP.ML.BackgroundSysTray"  "AdaptiveSyncLib\csproj\dll" GalaSoft.MvvmLight.dll GalaSoft.MvvmLight.Platform.dll GCP.Core.dll IGCCTray.exe ML.CommonLibrary.dll Newtonsoft.Json.dll Newtonsoft.Json.Schema.dll Serilog.dll Serilog.Sinks.File.dll
robocopy "C:\Program Files\WindowsApps\AppUp.IntelGraphicsExperience_1.100.1725.0_x64__8j3eq9eme6ctt\GCP.ML.BackgroundSysTray"  "AdaptiveSyncLib\Complete Library" GalaSoft.MvvmLight.dll GalaSoft.MvvmLight.Platform.dll GCP.Core.dll IGCCTray.exe ML.CommonLibrary.dll Newtonsoft.Json.dll Newtonsoft.Json.Schema.dll Serilog.dll Serilog.Sinks.File.dll
echo files have been copied over
msbuild "AdaptiveSyncLib\csproj\projcs.sln" -property:Configuration=Release 
echo csproj built
xcopy "AdaptiveSyncLib\csproj\projcs\bin\Release\Adaptivecs.dll" "AdaptiveSyncLib\wrapproj"
xcopy "AdaptiveSyncLib\csproj\projcs\bin\x64\Release\Adaptivecs.dll" "AdaptiveSyncLib\wrapproj"
echo copied over adaptivecs.dll tp wrapproj

cd "AdaptiveSyncLib\wrapproj"
cl /clr /LD AdaptiveLib.cpp
cd ..
cd ..
echo compiled wrapper

robocopy "AdaptiveSyncLib\wrapproj" "AdaptiveSyncLib\Complete Library" Adaptivecs.dll AdaptiveLib.dll AdaptiveLib.lib Header.h
echo all library files in complete library

xcopy "AdaptiveSyncLib\Complete Library\*" "AdaptiveSyncDemo\src\bin\x64\Release"
xcopy "AdaptiveSyncLib\Complete Library\*" "AdaptiveSyncDemo\src\bin\x64\Debug"
xcopy "AdaptiveSyncLib\Complete Library\*" "AdaptiveSyncDemo\src\dll"

msbuild "AdaptiveSyncDemo\src\D3D12Fullscreen.sln" -property:Configuration=Release

REM START "AdaptiveSyncDemo\src\bin\x64\Release\D3D12Fullscreen.exe"

