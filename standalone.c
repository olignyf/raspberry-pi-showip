

#include "toolbox.h"

int main(int argc, char *argv[])
{

	char ip[256];
	char mac[256];
	char subnet[256];
	char broadcast[256];
	char RX_packets[256];
	char RX_errors[256];
	char RX_bytes[256];
	char TX_packets[256];
	char TX_errors[256];
	char TX_bytes[256];
	char collisions[256];
	C_GetNetworkInformation(ip, subnet, broadcast, mac, RX_packets, RX_errors, RX_bytes, TX_packets, TX_errors, TX_bytes, collisions);
	printf("ip(%s)\n",ip);
	printf("subnet(%s)\n",subnet);
	printf("broadcast(%s)\n",broadcast);
	printf("mac(%s)\n",mac);
	printf("RX_packets(%s)\n",RX_packets);
	printf("RX_errors(%s)\n",RX_errors);
	printf("RX_bytes(%s)\n",RX_bytes);
	printf("TX_packets(%s)\n",TX_packets);
	printf("TX_errors(%s)\n",TX_errors);
	printf("TX_bytes(%s)\n",TX_bytes);
	printf("collisions(%s)\n",collisions);
	return 0;
}