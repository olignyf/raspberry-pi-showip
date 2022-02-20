
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <arpa/inet.h>

#include "toolbox.h"

int main(int argc, char *argv[])
{
	int iret;
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
	char ifconfig[20000] = "";
	printf("eth0:");

	struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) 
    {
        if (!ifa->ifa_addr) 
        {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) // check it is IP4
        { 
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
            if (strcmp(ifa->ifa_name, "lo") != 0)
            {
                strncpy(ip, addressBuffer, sizeof(ip)-1);
		    }
        }
    }
    
    // loop ipv6 if we couldnt find ipv4 eth0
    if (ip[0] == '\0') for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) 
    {
        if (!ifa->ifa_addr) 
        {
            continue;
        }
        
        if (ifa->ifa_addr->sa_family == AF_INET6) 
        {
            tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer); 
            if (strcmp(ifa->ifa_name, "lo") != 0)
            {
                strncpy(ip, addressBuffer, sizeof(ip)-1);
		    }
        } 
    }
    
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
    
	printf("ip(%s)\n",ip);/*
	printf("subnet(%s)\n",subnet);
	printf("broadcast(%s)\n",broadcast);
	printf("mac(%s)\n",mac);
	printf("RX_packets(%s)\n",RX_packets);
	printf("RX_errors(%s)\n",RX_errors);
	printf("RX_bytes(%s)\n",RX_bytes);
	printf("TX_packets(%s)\n",TX_packets);
	printf("TX_errors(%s)\n",TX_errors);
	printf("TX_bytes(%s)\n",TX_bytes);
	printf("collisions(%s)\n",collisions);*/

	printf("=wlan0:");
	
	if (iret < 0)
	{
		printf("failed to get wlan0, iret(%d)", iret);
	}
	else if (iret == 0)
	{
		printf("no wlan0");
	}
	else
	{
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
	}
	return 0;
}
