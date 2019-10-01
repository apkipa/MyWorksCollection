#include <Windows.h>
#include <Wlanapi.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment(lib, "Wlanapi.lib")

#define IsMainChoiceInRange(nChoice) (0 <= (nChoice) && (nChoice) <= 4)

HANDLE hWlan;

HANDLE OpenWlanHandle(void) {
	DWORD dwNegotiatedVersion;
	HANDLE hWlan = NULL;
	WlanOpenHandle(2, NULL, &dwNegotiatedVersion, &hWlan);
	return hWlan;
}

void CloseWlanHandle(HANDLE hWlan) {
	WlanCloseHandle(hWlan, NULL);
}

bool StartHostedNetwork(void) {
	return WlanHostedNetworkStartUsing(hWlan, NULL, NULL) == ERROR_SUCCESS;
}

bool StopHostedNetwork(void) {
	return WlanHostedNetworkStopUsing(hWlan, NULL, NULL) == ERROR_SUCCESS;
}

bool ForceStopHostedNetwork(void) {
	return WlanHostedNetworkForceStop(hWlan, NULL, NULL) == ERROR_SUCCESS;
}

bool QueryHostedNetworkStatus(void) {
	PWLAN_HOSTED_NETWORK_STATUS *ppWhns;
	int nLength;

	ppWhns = WlanAllocateMemory(sizeof(PWLAN_HOSTED_NETWORK_STATUS));

	if (WlanHostedNetworkQueryStatus(hWlan, ppWhns, NULL) != ERROR_SUCCESS)
		return false;

	puts("Hosted network status:");
	printf("State: ");
	switch ((*ppWhns)->HostedNetworkState) {
	case wlan_hosted_network_unavailable:
		puts("Unavailable");
		break;
	case wlan_hosted_network_idle:
		puts("Idle");
		break;
	case wlan_hosted_network_active:
		puts("Active");

		printf("Channel frequency: %lu\n", (unsigned long)(*ppWhns)->ulChannelFrequency);
		printf("Number of peers: %u\n", (unsigned int)(*ppWhns)->dwNumberOfPeers);
		for (unsigned int i = 0; i < (*ppWhns)->dwNumberOfPeers; i++) {
			nLength = printf("Peer %d: ", i + 1);
			printf(
				"MAC address: %02hhX-%02hhX-%02hhX-%02hhX-%02hhX-%02hhX\n",
				(*ppWhns)->PeerList[i].PeerMacAddress[0], (*ppWhns)->PeerList[i].PeerMacAddress[1],
				(*ppWhns)->PeerList[i].PeerMacAddress[2], (*ppWhns)->PeerList[i].PeerMacAddress[3],
				(*ppWhns)->PeerList[i].PeerMacAddress[4], (*ppWhns)->PeerList[i].PeerMacAddress[5]
			);
			printf("%*sAuth state: ", nLength, "");
			switch ((*ppWhns)->PeerList[i].PeerAuthState) {
			case wlan_hosted_network_peer_state_invalid:
				puts("Invalid");
				break;
			case wlan_hosted_network_peer_state_authenticated:
				puts("Authenticated");
				break;
			default:
				puts("Unknown");
				break;
			}
		}
		break;
	}

	WlanFreeMemory(*ppWhns);
	WlanFreeMemory(ppWhns);

	return true;
}

void MainCycle(void) {
	int nChoice;

	while (true) {
		puts("WlanTest - Test for opening / closing hosted network");
		puts("Enter your choice:");
		puts("0. Exit");
		puts("1. Start hosted network");
		puts("2. Stop hosted network");
		puts("3. Query hosted network status");
		puts("4. Force stop hosted network");

		while (scanf("%d", &nChoice) != 1 || !IsMainChoiceInRange(nChoice)) {
			puts("Invalid input. Try again.");
			scanf("%*[^\n]");
		}

		switch (nChoice) {
		case 0:
			return;
		case 1:
			puts(StartHostedNetwork() ? "Succeeded." : "Failed.");
			break;
		case 2:
			puts(StopHostedNetwork() ? "Succeeded." : "Failed.");
			break;
		case 3:
			if (!QueryHostedNetworkStatus())
				puts("Failed.");
			break;
		case 4:
			puts(ForceStopHostedNetwork() ? "Succeeded." : "Failed.");
			break;
		}
	}
}

int main(void) {
	hWlan = OpenWlanHandle();

	if (hWlan == NULL) {
		puts("Cannot init wlan handle. Exiting.");
	}
	else {
		MainCycle();
		CloseWlanHandle(hWlan);
	}
}