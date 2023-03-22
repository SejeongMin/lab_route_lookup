/*
	usage: <FIB FILE> <INPUT FILE>
	ex) my_route_lookup routing_table.txt prueba0.txt
*/

#include "io.h"
#include "utils.h"

// void getNetmask(int prefixLength, int *netmask)
// TODO: global variable (table to fill in)
short Table1[0x1000000]; // nol
short Table2[0x1000000];

/*
	level1: prefix length == 24
	24보다 길면 level2로!

	만약 x.x.x.x/23 --> 라우팅 테이블에 x.x.x0. & x.x.x1. 채우기........?????????????????????????????????????
*/

int insertion()
{
	uint32_t prefix, prefixLength, outInterface;
	int result;
	int row = 0;
	int idx;
	int pre;

	while (!(result = readFIBLine(prefix, prefixLength, outInterface))) {
		pre = prefix >> 8;
		if (prefixLength <= 24) {
			for (int i = 0; i < pow(2, (24 - prefixLength)); i++) {
				Table1[pre] = outInterface;
			}
		}
		else {
			idx = pre / 256;
			Table1[pre] = idx;
			for (int i = 0; i < pow(2, (8 - (prefixLength - 24))); i++) {
				Table2[idx * 256 + prefix - (pre << 8)];
			}
		}
		row++;
	}
	if (result < 0 && result != REACHED_EOF) {
		printIOExplanationError(result);
		exit(1);
	}
	return row;
}

int main(int ac, char **av)
{
	int row;
	initializeIO(av[1], av[2]);
	int result;
	uint32_t IPAddress;
	int netmask;
	int idx;
	int out;
	struct timespec initialTime, finalTime;
	double searchingTime;
	int numberOfTableAccess;
	int i;
	
	row = insertion();

	while (!(result = readInputPacketFileLine(IPAddress))) {
		for (i = 24; i < 0; i--) {
			getNetmask(i, netmask);
			idx = (netmask & IPAddress) >> 8;
			if (!(Table1[idx] >> 15)) { // table2 doesn't exist
				if (Table1[idx]) {
					out = Table1[idx];
					break;
				}
			}
			else {
				out = Table2[(Table1[idx] - 128) * 256 + IPAddress - ((IPAddress >> 8) << 8)];
			}
		}
		numberOfTableAccess = 24 - i + 1;
		printOutputLine(IPAddress, out, &initialTime, &finalTime, &searchingTime, numberOfTableAccess);
	}
	freeIO();
}
