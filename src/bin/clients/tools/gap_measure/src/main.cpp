#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "stinger_core/stinger.h"
#include "stinger_core/xmalloc.h"
#include "stinger_net/mon_handling.h"
#include "stinger_net/stinger_mon.h"
#include "stinger_utils/timer.h"

#include "gap_measure.h"

using namespace gt::stinger;

int main(int argc, char *argv[]) {
	init_timer();

	if (-1 == mon_connect(10103, "localhost", "alg_to_mongo")) {
		LOG_E_A ("Connect to STINGER on %s:%d failed", "localhost", 10103);
		return -1;
	}

	StingerMon & mon = StingerMon::get_mon();

	while (1) {
		mon.wait_for_sync();

		tic();

		mon.get_alg_read_lock();

		stinger_t * S = mon.get_stinger();

		if (!S) {
			LOG_E ("bad stinger pointer");
			return -1;
		}

		int64_t nv = stinger_mapping_nv(S);

		mon.release_alg_read_lock();

		double time = toc();
		LOG_I_A ("elapsed: %20.15e sec", time);
		LOG_I_A ("nv: %ld", nv);
		LOG_I_A ("rate: %20.15e vtx/sec", (double) nv / time);
	}

	return 0;
}
