#pragma once


class __declspec(dllexport) AdaptiveSyncWrapper
{

public:
	AdaptiveSyncWrapper();
	~AdaptiveSyncWrapper();

	bool AdaptiveSyncSupported();
	bool IsAdaptiveSyncEnabled();
	void EnableAdaptiveSync();
	void DisableAdaptiveSync();
	int	AdaptiveSyncMinRefresh();
	int	AdaptiveSyncMaxRefresh();

};