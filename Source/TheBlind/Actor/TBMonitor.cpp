#include "TBMonitor.h"

ATBMonitor::ATBMonitor()
{
	// 모니터 메시를 생성합니다.
	{
		Monitor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Monitor"));
		Monitor->SetupAttachment(Mesh);
	}
}
