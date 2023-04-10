#include "io.h"
#include "utils.h"

unsigned short Table1[0x1000000];
unsigned short *Table2; // dynamic memory allocation

int insertion() // fill the tables with the routing table
{
	uint32_t prefix;
	int prefixLength, outInterface;
	int result;
	int row = 0; // variable to keep track of position of 2nd level table index
	int N;
	int pre;
	int idx2;

	while (!(result = readFIBLine(&prefix, &prefixLength, &outInterface))) {
		pre = prefix >> 8;
		if (prefixLength <= 24) { // if the prefix length is shorter than 24, it doesn't need 2nd table
			for (int i = 0; i < pow(2, (24 - prefixLength)); i++) {
				Table1[pre++] = outInterface;
			}
		}
		else { // if the prefix length is longer than 24, make a 2nd level table entries and link it on the 1st table
			if (Table1[pre] >> 15) { // If there is already a link to 2nd level, override them
				N = Table1[pre] - 32768;
				idx2 = N * 256 + prefix - (pre << 8);
				for (int i = 0; i < pow(2, (8 - (prefixLength - 24))); i++) {
					Table2[idx2++] = outInterface;
				}
			}
			else { // If there is no link to 2nd level, re-allocate the table 2 and fill it
				Table1[pre] = row + 32768;
				Table2 = (unsigned short *)realloc(Table2, sizeof(unsigned short) * (row + 1) * 256);
				for(int i = row * 256; i < row * 256 + 256; i++) {
					Table2[i] = 0;
				}
				idx2 = row * 256 + prefix - (pre << 8);
				for (int i = 0; i < pow(2, (8 - (prefixLength - 24))); i++) {
					Table2[idx2++] = outInterface;
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
	int idx, result, row;
	struct timespec initialTime, finalTime;
	double searchingTime;
	int accessedTable;
	int out = 0;
	int totalProcessTime = 0;
	int totalTableAccess = 0;
	int totalPackets = 0;

	Table2 = (unsigned short *)malloc(0); // initial allocation of table 2
	row = insertion();

	while (!(result = readInputPacketFileLine(&IPAddress))) {
		clock_gettime(CLOCK_MONOTONIC, &initialTime);
		idx = IPAddress >> 8;
		if (!(Table1[idx] >> 15)) { // if 2nd table entry doesn't exist
			out = Table1[idx];
			accessedTable = 1;
		}
		else { // if 2nd table entry exists
			out = Table2[(Table1[idx] - 32768) * 256 + IPAddress - ((IPAddress >> 8) << 8)];
			accessedTable = 2;
		}
		clock_gettime(CLOCK_MONOTONIC, &finalTime); // to measure the processing time
		printOutputLine(IPAddress, out, &initialTime, &finalTime, &searchingTime, accessedTable);
		totalProcessTime += searchingTime;
		totalTableAccess += accessedTable;
		++totalPackets;
	}
	printSummary(totalPackets, (double)totalTableAccess / totalPackets, (double)totalProcessTime / totalPackets);
	freeIO();
	free(Table2);
}
