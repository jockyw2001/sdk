#include <stdio.h>

#include "Mdb.h"
#include "mdb_service.h"

void mdb_service(void)
{
	Mdb::Run();
	Mdb::Wait();
}