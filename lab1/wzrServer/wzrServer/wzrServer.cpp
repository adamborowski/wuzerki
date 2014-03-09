// wzrServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "siec.h"
#include <iostream>


//using namespace std;



int _tmain(int argc, _TCHAR* argv[])
{
//	  unicast_net *pmt_net=(unicast_net*)ptr;  // wskaŸnik do obiektu klasy unicast_net
	
	const int StateSize = 80;
	const int FrameSize = 88;
	unicast_net *uni_recv = new unicast_net(10001); 
	unicast_net *uni_send = new unicast_net(10002);
	unsigned long ClientsIp[20] = {};
	unsigned long ClientIp = 0;

	char frame[88] = {'r','a','m','k','a'};
	char* message = frame;
	int rozmiar;                             // liczba bajtów ramki otrzymanej z sieci
	
	while(1)
	{
		//reciv((char*)&ramka,&last_sent_ip, sizeof(StanObiektu)
		rozmiar = uni_recv->reciv(frame,&ClientIp, FrameSize);   // oczekiwanie na nadejœcie ramki 
		
		for(int i=0; i<20; i++)
		{
			if(ClientsIp[i] == 0)
			{
				ClientsIp[i] = ClientIp;
				std::cout << "Connected client, IP: " << ClientIp;
			}
			if(ClientsIp[i] == ClientIp)
				break;			
		}
		for(int i=0; i<20; i++)
		{
			if(ClientsIp[i] != 0 && ClientsIp[i] != ClientIp)
			{
				//(char*)&ramka, SERVER_IP, sizeof(Ramka)
				uni_send->send(frame, ClientsIp[i], FrameSize);
				std::cout << "Data send to client, IP: " << ClientsIp[i];
			}
		}

	}  
	return 0;
}

