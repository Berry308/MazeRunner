// Copyright Epic Games, Inc. All Rights Reserved.

#include "MRAssetManagerStartupJob.h"

#include "MazeRunnerLogChannels.h"

TSharedPtr<FStreamableHandle> FMRAssetManagerStartupJob::DoJob() const
{
	const double JobStartTime = FPlatformTime::Seconds();

	TSharedPtr<FStreamableHandle> Handle;
	UE_LOG(LogMR, Display, TEXT("Startup job \"%s\" starting"), *JobName);
	JobFunc(*this, Handle);//执行函数

	if (Handle.IsValid())
	{
		Handle->BindUpdateDelegate(FStreamableUpdateDelegate::CreateRaw(this, &FMRAssetManagerStartupJob::UpdateSubstepProgressFromStreamable));
		Handle->WaitUntilComplete(0.0f, false);
		Handle->BindUpdateDelegate(FStreamableUpdateDelegate());
	}

	UE_LOG(LogMR, Display, TEXT("Startup job \"%s\" took %.2f seconds to complete"), *JobName, FPlatformTime::Seconds() - JobStartTime);

	return Handle;
}
