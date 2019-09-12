///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016, Intel Corporation
// Permission is hereby granted, free of charge, to any person obtaining a   copy of this software and associated
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation 
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is    furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of 
// the Software.// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A    PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using GCP.Core.Service;
using GCP.Core.Service.BackgroundTasks;
using GCP.Core.Service.DesktopBridge;
using GCP.Core.Service.Display;
using GCP.Core.Service.Home;
using GCP.Core.Service.Hotkey;
using GCP.Core.Service.Power;
using GCP.Core.Service.Preferences;
using GCP.Core.Service.Promotions;
using GCP.Core.Service.Setting;
using GCP.Core.Service.SystemInfomation;
using GCP.Core.Service.Update;
using GCP.Core.Util;
using GCP.ML.BackgroundSysTray;
using ML.CommonLibrary.Model.Bridge.Display;
using System;
using System.Threading.Tasks;
using GCP.Core.Service.Localization;
using GCP.Core.Service.Performance;
using GCP.Core.Service.Program;
using GCP.Core.Service.Telemetry;
using GCP.Core.Service.Video;
using GCP.Service.Display;
using ML.CommonLibrary.Model.Bridge;
using GCP.ML.BackgroundSysTray.Display;
using ML.CommonLibrary.Model.Display;
using System.Runtime.InteropServices;

namespace ConsoleApp2
{
    public class Adaptive
    {

        static public bool AdaptiveSyncSupported()
        {
            DisplayManager.EnumDisplayDevicesAsync(true);
            foreach (var adapter in DisplayManager.DisplayAdaptersList)
            {
                AdaptiveSync vrrInfo = new AdaptiveSync();

                EscapeCall escapeCall = EscapeCall.GetEscapeInterface();
                escapeCall.GetAdaptiveSyncInfo(adapter.displayUID, ref vrrInfo);

                if (vrrInfo.Supported)
                    return true;
            }
            return false;
        }
        static public bool IsAdaptiveSyncEnabled()
        {
            DisplayManager.EnumDisplayDevicesAsync(true);
            GlobalSettingsPayload globalSettings;
            globalSettings = GlobalSettings.GetGlobalSettings();
            return (globalSettings.AdaptiveSync);
        }
        static public void EnableAdaptiveSync()
        {
            DisplayManager.EnumDisplayDevicesAsync(true);
            GlobalSettingsPayload data;
            data = GlobalSettings.GetGlobalSettings();
            if (!data.AdaptiveSync)
            {
                data.AdaptiveSync = true;
                data.ChangedSetting = ApplyDriverSettings.AdaptiveSync;
                GlobalSettings.SetGlobalSettings(data);
            }
        }
        static public void DisableAdaptiveSync()
        {
            DisplayManager.EnumDisplayDevicesAsync(true);
            GlobalSettingsPayload data;
            data = GlobalSettings.GetGlobalSettings();
            if (data.AdaptiveSync)
            {
                data.AdaptiveSync = false;
                data.ChangedSetting = ApplyDriverSettings.AdaptiveSync;
                GlobalSettings.SetGlobalSettings(data);
            }
        }
        static public uint AdaptiveSyncMaxRefresh()
        {
            DisplayManager.EnumDisplayDevicesAsync(true);
            foreach (var adapter in DisplayManager.DisplayAdaptersList)
            {
                AdaptiveSync vrrInfo = new AdaptiveSync();

                EscapeCall escapeCall = EscapeCall.GetEscapeInterface();
                escapeCall.GetAdaptiveSyncInfo(adapter.displayUID, ref vrrInfo);

                if (vrrInfo.Supported)
                    return vrrInfo.MaxRefreshRate;
            }
            return 0;
        }
        static public uint AdaptiveSyncMinRefresh()
        {
            DisplayManager.EnumDisplayDevicesAsync(true);
            foreach (var adapter in DisplayManager.DisplayAdaptersList)
            {
                AdaptiveSync vrrInfo = new AdaptiveSync();

                EscapeCall escapeCall = EscapeCall.GetEscapeInterface();
                escapeCall.GetAdaptiveSyncInfo(adapter.displayUID, ref vrrInfo);

                if (vrrInfo.Supported)
                    return vrrInfo.MinRefreshRate;
            }
            return 0;
        }


    }
}

