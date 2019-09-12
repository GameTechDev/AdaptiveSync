@echo off

REM files in the adaptivesynclib
del "AdaptiveSyncLib\Complete Library\*" / q
del "AdaptiveSyncLib\csproj\projcs\bin\*" / q
del "AdaptiveSyncLib\csproj\projcs\obj\*" / q
del "AdaptiveSyncLib\wrapproj\Adaptivecs.dll" / q
del "AdaptiveSyncLib\wrapproj\AdaptiveLib.dll" / q
del "AdaptiveSyncLib\wrapproj\AdaptiveLib.exp" / q
del "AdaptiveSyncLib\wrapprojct\AdaptiveLib.obj" / q
del  "AdaptiveSyncLib\wrapproj\AdaptiveLib.lib" / q
del "AdaptiveSyncLib\csproj\dll\*" /q
del "AdaptiveSyncLib\csproj\projcs\obj\*" /q /s
del "AdaptiveSyncLib\csproj\projcs\bin\*" / q / s

REM files in the demo folder
del "AdaptiveSyncDemo\src\bin\*" /s /q
del "AdaptiveSyncDemo\src\obj\*" / s / q
del "AdaptiveSyncDemo\src\dll\*" /s /q

