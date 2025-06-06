#include "pch.h"
#include <Psapi.h>
#include <assert.h>
#include "WinLowLevel.h"

namespace WinLL {
	static_assert(sizeof(PerformanceInformation) <= sizeof(SYSTEM_PERFORMANCE_INFORMATION));

	PerformanceInformation SystemInformation::GetPerformanceInformation() {
		PerformanceInformation info{};
		ULONG len;
		auto status = ::NtQuerySystemInformation(SystemPerformanceInformation, &info, sizeof(info), &len);
		assert(NT_SUCCESS(status));
		return info;
	}

	const WindowsVersion& SystemInformation::GetWindowsVersion() {
		static WindowsVersion version;
		if (version.Major == 0) {
			auto sharedData = USER_SHARED_DATA;
			version.Build = sharedData->NtBuildNumber;
			version.Major = sharedData->NtMajorVersion;
			version.Minor = sharedData->NtMinorVersion;
		}
		return version;
	}

	const BasicSystemInfo& SystemInformation::GetBasicSystemInfo() {
		static BasicSystemInfo info;
		if (info.PageSize == 0) {
			SYSTEM_INFO si;
			::GetNativeSystemInfo(&si);
			info.NumberOfProcessors = si.dwNumberOfProcessors;
			info.PageSize = si.dwPageSize;
			info.ProcessorArchitecture = static_cast<ProcessorArchitecture>(si.wProcessorArchitecture);
			info.ProcessorLevel = si.wProcessorLevel;
			info.ProcessorRevision = si.wProcessorRevision;
			info.MinimumAppAddress = si.lpMinimumApplicationAddress;
			info.MaximumAppAddress = si.lpMaximumApplicationAddress;

			PERFORMANCE_INFORMATION pi = { sizeof(pi) };
			::GetPerformanceInfo(&pi, sizeof(pi));
			info.TotalPhysicalInPages = pi.PhysicalTotal;
			info.CommitLimitInPages = pi.CommitLimit;
		}
		return info;
	}

	uint64_t SystemInformation::GetBootTime() {
		static int64_t time = 0;
		if (time == 0) {
			SYSTEM_TIMEOFDAY_INFORMATION info;
			if (NT_SUCCESS(::NtQuerySystemInformation(SystemTimeOfDayInformation, &info, sizeof(info), nullptr)))
				time = info.BootTime.QuadPart;
		}

		return time;
	}

	wstring SystemInformation::GetSystemDirectory() {
		WCHAR path[MAX_PATH];
		::GetSystemDirectoryW(path, _countof(path));
		return path;
	}
}
