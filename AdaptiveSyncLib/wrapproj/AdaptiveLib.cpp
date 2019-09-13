///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019, Intel Corporation
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

#using "Adaptivecs.dll"
#include "Header.h"
#include <msclr/auto_gcroot.h>


using namespace msclr;

class AdaptiveSyncClass;

AdaptiveSyncClass* Adapt;


class AdaptiveSyncClass
{
public: auto_gcroot<ConsoleApp2::Adaptive^> adaptive;
};

AdaptiveSyncWrapper::AdaptiveSyncWrapper()
{
	Adapt = new AdaptiveSyncClass();
	Adapt->adaptive = gcnew ConsoleApp2::Adaptive();
}

bool AdaptiveSyncWrapper::AdaptiveSyncSupported()
{
	return Adapt->adaptive->AdaptiveSyncSupported();
}

bool AdaptiveSyncWrapper::IsAdaptiveSyncEnabled()
{
	return Adapt->adaptive->IsAdaptiveSyncEnabled();
}

void AdaptiveSyncWrapper::EnableAdaptiveSync()
{
	Adapt->adaptive->EnableAdaptiveSync();
}

void AdaptiveSyncWrapper::DisableAdaptiveSync()
{
	Adapt->adaptive->DisableAdaptiveSync();
}
int AdaptiveSyncWrapper::AdaptiveSyncMaxRefresh()
{
	return Adapt->adaptive->AdaptiveSyncMaxRefresh();
}
int AdaptiveSyncWrapper::AdaptiveSyncMinRefresh()
{
	return Adapt->adaptive->AdaptiveSyncMinRefresh();
}

AdaptiveSyncWrapper::~AdaptiveSyncWrapper()
{
	delete Adapt;
}

