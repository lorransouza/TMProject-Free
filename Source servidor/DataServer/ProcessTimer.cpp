#include "framework.h"
#include "DataServer.h"
#include "CPSock.h"
#include "CUser.h"
#include "Base.h"
#include "ProcessTimer.h"
#include "CFileDB.h"


void ProcessSecTimer()
{
	cServer.SecCounter++;

	ImportUser();
	ImportPass();
	ImportItem();
	ImportBan();
	ImportCash();


	time_t rawnow = time(NULL);
	struct tm now;
	localtime_s(&now, &rawnow);

	if (cServer.SecCounter % 10)
		WriteConfig();
}