/*
	usage: <FIB FILE> <INPUT FILE>
	ex) my_route_lookup routing_table.txt prueba0.txt
*/

#include "io.h"
#include "utils.h"

unsigned short Table1[0x1000000];
unsigned short Table2[0x1000000];

/*
	level1: prefix length == 24
	24보다 길면 level2로!

	만약 x.x.x.x/23 --> 라우팅 테이블에 x.x.x0. & x.x.x1. 채우기........?????????????????????????????????????
*/

int insertion()
{
	uint32_t prefix;
	int prefixLength, outInterface;
	int result;
	int row = 0;
	int N;
	int pre;
	int ddd;

	while (!(result = readFIBLine(&prefix, &prefixLength, &outInterface))) {
		pre = prefix >> 8;
		if (prefixLength <= 24) {
			for (int i = 0; i < pow(2, (24 - prefixLength)); i++) {
				Table1[pre++] = outInterface;
			}
		}
		else {
			if (Table1[pre] >> 15) { // If there is already a link to 2nd level (ppt pg.39)
				N = Table1[pre] - 32768;
				ddd = N * 256 + prefix - (pre << 8);
				for (int i = 0; i < pow(2, (8 - (prefixLength - 24))); i++) {
					Table2[ddd++] = outInterface;
				}
			}
			else {
				Table1[pre] = row + 32768;
				ddd = row * 256 + prefix - (pre << 8); // think
				for (int i = 0; i < pow(2, (8 - (prefixLength - 24))); i++) {
					Table2[ddd++] = outInterface;
				}
				row++;
			}
		}
	}
	if (result < 0 && result != REACHED_EOF) {
		printIOExplanationError(result);
		exit(1);
	}
	return row;
}

int main(int ac, char **av)
{
	initializeIO(av[1], av[2]);
	uint32_t IPAddress;
	int netmask, idx, i, result, row;
	struct timespec initialTime, finalTime;
	double searchingTime;
	int accessedTable;
	int out = 0;
	int totalProcessTime = 0;
	int totalTableAccess = 0;
	int totalPackets = 0;
	
	row = insertion();

	while (!(result = readInputPacketFileLine(&IPAddress))) {
		clock_gettime(CLOCK_MONOTONIC, &initialTime);
		for (i = 24; i > 0; i--) {
			getNetmask(i, &netmask);
			idx = (netmask & IPAddress) >> 8;
			if (!(Table1[idx] >> 15)) { // table2 doesn't exist
				if (Table1[idx]) {
					out = Table1[idx];
					accessedTable = 1;
					break;
				}
			}
			else {
				out = Table2[(Table1[idx] - 32768) * 256 + IPAddress - ((IPAddress >> 8) << 8)];
				accessedTable = 2;
				break;
			}
		}
		clock_gettime(CLOCK_MONOTONIC, &finalTime);
		printOutputLine(IPAddress, out, &initialTime, &finalTime, &searchingTime, accessedTable);
		totalProcessTime += searchingTime;
		totalTableAccess += accessedTable;
		++totalPackets;
	}
	freeIO();
	printSummary(totalPackets, (double)totalTableAccess / totalPackets, (double)totalProcessTime / totalPackets);
}
