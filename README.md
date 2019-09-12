# Adaptive Sync Demo
![Adaptive Sync GUI](image.PNG)

This project illustrates how to implement Intels Adaptive Sync capability in a Directx 12 application. More information can be found [here](https://wiki.ith.intel.com/display/hdmtdiagspxie/Adaptive+Sync)

It is recommended to build this library on a machine that has intel graphics.

To build this project, first download the Intel Graphics Command Center here:
https://www.microsoft.com/en-us/p/intel-graphics-command-center/9plfnlnt3g5g?activetab=pivot:regionofsystemrequirementstab

the .net frameworm 4.7.2 Developer Pack sdk also needs to be installed: https://dotnet.microsoft.com/download/dotnet-framework/net472

Having visual studio installed is required to build this project. Once the IGCC is installed, open "x64 Native Tools Command Prompt for Vs" command prompt for your version of visual studio.

Within the command prompt, nasssvigate to the root of the adaptive sync project folder and run "run.bat"

If this is successful, all dll's from the IGCC will be gathered and placed into the project. 

Once the command has finished, you can now run the demo located at "adaptive-sync-demo\AdaptiveSyncDemo\src\bin\x64\Release\D3D12Fullscreen.exe"

The complete adaptive sync library will be compiled into "adaptive-sync-demo\AdaptiveSyncLib\Complete Library" including all dll, lib, and ehader files to implement the library into your project.


Known configurations:
Machine: Icelake
Visual Studio: 2019 Preview 



Troubleshooting:
If after running run.bat, dll files do not apear within "adaptive-sync-demo\AdaptiveSyncLib\Complete Library":

It is possible that the location of Intel dll has changed. If this is the case, naviagat to "C:\Program Files\WindowsApps\" and search for GCP.Core.dll. This should be in a directory such as "AppUp.IntelGraphicsExperience_1.100.1725.0_x64__8j3eq9eme6ctt\" take note of the version number, open run.bat and modify the path within the .bat file if needed.
.

