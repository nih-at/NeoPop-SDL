//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

/*
//---------------------------------------------------------------------------
//=========================================================================

	system_comms.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

28 AUG 2002 - neopop_uk
=======================================
- Created this file to handle inter-NeoPop link-up communications.
- Added dialog box handler.
- Connect and listen operations.

30 AUG 2002 - neopop_uk
=======================================
- Tidied up the "error disconnect" messages.

04 SEP 2002 - neopop_uk
=======================================
- Will now use the dialog translation when it's full supported.

08 SEP 2002 - neopop_uk
=======================================
- Added partial support for translations.

//---------------------------------------------------------------------------
*/

#include "neopop.h"
#include <windows.h>
#include <windowsx.h>
#include "resource.h"

#include "system_main.h"
#include "system_comms.h"
#include "system_language.h"
#include "system_config.h"

//=============================================================================

typedef struct
{
	_u8 control;
	_u8 data;
}
COMMSDATA;

/*

	The packet structure is incredibly simple, the control byte
	determines what the data byte means. It should be quite
	possible for different ports to be link-up compatible!

Control
=======
	0	= "Data" - Emulated data, return this to the emulation core.
	
	1	= "Pause" 
	
	 Data:	1	- The other system is not able to emulate right
					now, please pause, but keep checking for "UnPause" message.

			0	- The other system is ready again, un-pause!

*/

//=============================================================================

static HWND h_linkup = NULL;
static HWND h_messages, h_port, h_remoteip, h_localip, h_connect, h_listen, h_ok;

static int messagecount;

static BOOL winsock_initialised = FALSE;
static BOOL connection_established = FALSE;

//Remote Socket & Address
static SOCKET remotesocket = INVALID_SOCKET;
static SOCKADDR_IN remote_sockaddr;

#define CONNECT_TIMEOUT	5000
#define LISTEN_TIMEOUT	45000
BOOL waiting;

//=============================================================================

static void __cdecl comms_message(char* vaMessage,...)
{
	char message[1001], print[1001];
    va_list vl;
	DWORD i, start;

    va_start(vl, vaMessage);
    vsprintf(message, vaMessage, vl);
	va_end(vl);

	start = 0;

	for (i = 0; i < strlen(message); i++)
	{
		if (message[i] == '\n')
		{
			memset(print, 0, 1000);
			memcpy(print, &(message[start]), i - start);
			start = i+1;

			ListBox_AddString(h_messages, print);
			ListBox_SetCurSel(h_messages, messagecount);
			messagecount++;
		}

		if (message[i] < 32)
			message[i] = '?';
	}

	//Last bit
	if (start <= strlen(message))
	{
		memset(print, 0, 1000);
		memcpy(print, &(message[start]), i - start);
		start = i+1;

		ListBox_AddString(h_messages, print);
		ListBox_SetCurSel(h_messages, messagecount);
		messagecount++;
	}
}

static void getPort(void)
{
	char port_str[10];
	_u16 tmp_port;
	
	Edit_GetText(h_port, port_str, 10);
	tmp_port = (_u16)(atol(port_str) & 0xFFFF);

	if (tmp_port > 0)
	{
		sprintf(port_str, "%d", tmp_port);
		comms_port = tmp_port;
	}
	else
	{
		sprintf(port_str, "%d", comms_port);
	}

	Edit_SetText(h_port, port_str);
}

static BOOL valid_ip(char* address_str)
{
	if (inet_addr(address_str) == INADDR_NONE)
		return FALSE;
	return TRUE;
}

static void getRemoteAddress(void)
{
	char address_str[64];

	Edit_GetText(h_remoteip, address_str, 64);

	if (valid_ip(address_str))
	{
		strcpy(comms_remoteaddress, address_str);
	}
	else
	{
		Edit_SetText(h_remoteip, comms_remoteaddress);
	}
}

//=============================================================================

static void comms_init(void)
{
	WORD versionneeded = MAKEWORD(2,2);
	WSADATA wsadata;
	HOSTENT* phe;
	char temp[255];

	//Clear message list.
	ListBox_ResetContent(h_messages);
	messagecount = 0;

	//Already connected?
	if (winsock_initialised)
	{
		// Get local IP
		gethostname(temp,255);
		phe = gethostbyname(temp);
		if (phe)
		{
			strcpy(temp, inet_ntoa(*(struct in_addr*)phe->h_addr));
			Edit_SetText(h_localip, temp);
		}

		if (connection_established)
		{
			comms_message(system_get_string(IDS_CONNECTED));

			//Change dialog
			Button_SetText(h_connect, system_get_string(IDS_DISCONNECT));
			Button_Enable(h_listen, FALSE);
			Button_Enable(h_remoteip, FALSE);
			Button_Enable(h_port, FALSE);
		}
		else
		{
			comms_message("Not connected...");
			comms_message("Click \"Connect\" or \"Listen\" to begin link-up.");
		}
		
		return;
	}

	winsock_initialised = FALSE;

	// Start WinSock
	if (WSAStartup(versionneeded, &wsadata) != 0)
	{
		comms_message("WinSock: startup FAILED.");
		return;
	}

	// Verify WinSock version
	if (wsadata.wVersion != versionneeded)
	{
		comms_message("Incorrect WinSock version (%d.%d), need WinSock 2.2", 
			wsadata.wVersion & 0x00FF, (wsadata.wVersion & 0xFF00) >> 8);
		WSACleanup();
		comms_message("Initialisation... FAILED.");
		return;
	}


	// Get local IP
	gethostname(temp,255);
    phe = gethostbyname(temp);
	if (phe)
	{
		strcpy(temp, inet_ntoa(*(struct in_addr*)phe->h_addr));
		Edit_SetText(h_localip, temp);
	}

	//Done!
	comms_message("Found \"%s\" and initialised.", wsadata.szDescription);
	comms_message("Click \"Connect\" or \"Listen\" to begin link-up.");
	winsock_initialised = TRUE;
}

//=============================================================================

//-----------------------------------------------------------------------------
// comms_connect()
//-----------------------------------------------------------------------------
static void comms_connect(void)
{
	int retval;
	_u32 argp;
	MSG msg;
	char address_str[64];

	//Clear message list.
	ListBox_ResetContent(h_messages);
	messagecount = 0;

	if (winsock_initialised == FALSE)
	{
		comms_message("Link-up is not available.");
		return;
	}

	// Get Connection information
	getPort();
	Edit_GetText(h_remoteip, address_str, 64);
	if (valid_ip(address_str) == FALSE)
	{
		comms_message("Remote IP address is invalid.");
		return;
	}
	else
	{
		strcpy(comms_remoteaddress, address_str);
	}

	// Create a remote TCP socket, verify it.
	remotesocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (remotesocket == INVALID_SOCKET)
	{
		comms_message("Create remote TCP socket FAILED.");
		return;
	}

	// Address the socket
	remote_sockaddr.sin_family = AF_INET;
	remote_sockaddr.sin_addr.s_addr = inet_addr(comms_remoteaddress);
	remote_sockaddr.sin_port = htons(comms_port);

	//Change dialog.
	Button_Enable(h_connect, FALSE);
	Button_Enable(h_listen, FALSE);
	Button_Enable(h_ok, FALSE);
	Button_Enable(h_remoteip, FALSE);
	Button_Enable(h_port, FALSE);

	//Connect...
	comms_message("Connecting to %s on port %d\nThis may take a few moments...\n", comms_remoteaddress, comms_port);

	//Message Pump
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (!TranslateAccelerator(g_hWnd, g_accelerator, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	//Connect to the remote NeoPop socket
	retval = connect(remotesocket, (LPSOCKADDR)&remote_sockaddr,
		 sizeof(struct sockaddr));

	//Change dialog.
	Button_Enable(h_connect, TRUE);
	Button_Enable(h_listen, TRUE);
	Button_Enable(h_ok, TRUE);
	Button_Enable(h_remoteip, TRUE);
	Button_Enable(h_port, TRUE);

	//Connection failed.
	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			closesocket(remotesocket);
			remotesocket = INVALID_SOCKET;
			comms_message("Connection FAILED.");
			return;
		}
	}

	// Configure the socket to be non-blocking
	argp = 1;
	retval = ioctlsocket(remotesocket, FIONBIO, &argp);
	if (retval == SOCKET_ERROR)
	{
		closesocket(remotesocket);
		remotesocket = INVALID_SOCKET;
		comms_message("Configure remote TCP socket FAILED.");
		return;
	}

	//Clear message list.
	ListBox_ResetContent(h_messages);
	messagecount = 0;

	//Success!
	comms_message(system_get_string(IDS_CONNECTED));

	connection_established = TRUE;

	//Change dialog
	Button_SetText(h_connect, system_get_string(IDS_DISCONNECT));
	Button_Enable(h_listen, FALSE);
	Button_Enable(h_remoteip, FALSE);
	Button_Enable(h_port, FALSE);
}

//-----------------------------------------------------------------------------
// comms_listen()
//-----------------------------------------------------------------------------
static void comms_listen(void)
{
	int retval;
	_u32 argp;
	MSG msg;
	DWORD time;
	SOCKET listensocket = INVALID_SOCKET;
	SOCKADDR_IN listen_sockaddr;

	//Clear message list.
	ListBox_ResetContent(h_messages);
	messagecount = 0;

	if (winsock_initialised == FALSE)
	{
		comms_message("Link-up is not available.");
		return;
	}

	// Get Connection information
	getPort();

	// Create a local TCP socket, verify it.
	listensocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listensocket == INVALID_SOCKET)
	{
		comms_message("Create local TCP socket FAILED.");
		return;
	}

	// Listen for any connecting address on this port.
	listen_sockaddr.sin_family = AF_INET;
	listen_sockaddr.sin_addr.s_addr = INADDR_ANY;
	listen_sockaddr.sin_port = htons(comms_port);

	//Bind the socket
	retval = bind(listensocket, (LPSOCKADDR)&listen_sockaddr, sizeof(struct sockaddr));
	if (retval == SOCKET_ERROR)
	{
		closesocket(listensocket);
		listensocket = INVALID_SOCKET;
		comms_message("Binding to port %d FAILED.", comms_port);
		return;
	}
	
 	//Listen for the remote NeoPop
	retval = listen(listensocket, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		closesocket(listensocket);
		listensocket = INVALID_SOCKET;
		comms_message("Set Listen Mode FAILED.");
		return;
	}

	// Configure the socket to be non-blocking
	argp = 1;
	retval = ioctlsocket(listensocket, FIONBIO, &argp);
	if (retval == INVALID_SOCKET)
	{
		closesocket(listensocket);		
		listensocket = INVALID_SOCKET;
		comms_message("Configure local TCP socket FAILED.");
		return;
	}

	//Change dialog.
	Button_SetText(h_listen, system_get_string(IDS_ABORT));
	Button_Enable(h_connect, FALSE);
	Button_Enable(h_ok, FALSE);
	Button_Enable(h_remoteip, FALSE);
	Button_Enable(h_port, FALSE);

	//Wait for a connection
	waiting = TRUE;
	time = GetTickCount();
	comms_message("Listening on port %d for %d seconds...", comms_port, LISTEN_TIMEOUT/1000);
	while((GetTickCount() - time) < LISTEN_TIMEOUT)
	{
		remotesocket = accept(listensocket, NULL, NULL);
		//Got one?
		if (remotesocket != INVALID_SOCKET)
			break;

		//Message Pump
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT || h_linkup == NULL)
			{
				closesocket(listensocket);		
				listensocket = INVALID_SOCKET;
				return;
			}

			if (!TranslateAccelerator(g_hWnd, g_accelerator, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		//Abort?
		if (waiting == FALSE)
		{
			comms_message("Aborted.");
			break;
		}
	}

	//Finished with the listen socket
	closesocket(listensocket);		
	listensocket = INVALID_SOCKET;

	//Change dialog
	system_language_patch_dialog(IDD_CONNECT, h_linkup);
	Button_Enable(h_connect, TRUE);
	Button_Enable(h_ok, TRUE);
	Button_Enable(h_remoteip, TRUE);
	Button_Enable(h_port, TRUE);

	//Timeout?
	if (waiting)
	{
		waiting = FALSE;
		if (remotesocket == INVALID_SOCKET)
			comms_message("Timeout, remote connection not established.");
	}

	// No connection
	if (remotesocket == INVALID_SOCKET)
		return;

	// Configure the socket to be non-blocking
	argp = 1;
	retval = ioctlsocket(remotesocket, FIONBIO, &argp);
	if (retval == INVALID_SOCKET)
	{
		closesocket(remotesocket);		
		remotesocket = INVALID_SOCKET;
		comms_message("Configure remote TCP socket FAILED.");
		return;
	}

	//Clear message list.
	ListBox_ResetContent(h_messages);
	messagecount = 0;

	//Success!
	comms_message(system_get_string(IDS_CONNECTED));

	connection_established = TRUE;

	//Change dialog
	Button_SetText(h_connect, system_get_string(IDS_DISCONNECT));
	Button_Enable(h_listen, FALSE);
	Button_Enable(h_remoteip, FALSE);
	Button_Enable(h_port, FALSE);
}

//-----------------------------------------------------------------------------
// comms_disconnect()
//-----------------------------------------------------------------------------
static void comms_disconnect(void)
{
	if (connection_established)
	{
		// Disconnect socket
		if (remotesocket != INVALID_SOCKET)
		{
			closesocket(remotesocket);
			remotesocket = INVALID_SOCKET;
		}

		comms_message("Disconnected.\n");
		connection_established = FALSE;

		//Change dialog
		Button_SetText(h_connect, "Connect to remote...");
		Button_Enable(h_listen, TRUE);
		Button_Enable(h_remoteip, TRUE);
		Button_Enable(h_port, TRUE);
	}
}

//=============================================================================

void system_comms_shutdown(void)
{
	comms_disconnect();
	
	// Shutdown WinSock?
	if (winsock_initialised)
	{
		WSACleanup();
		winsock_initialised = FALSE;
	}

	// Close dialog box
	if (h_linkup)
	{
		DestroyWindow(h_linkup);
		h_linkup = NULL;
	}
}

//-----------------------------------------------------------------------------
// system_comms_pause()
//-----------------------------------------------------------------------------
void system_comms_pause(BOOL remote_enable)
{
	if (connection_established)
	{
		COMMSDATA packet;
		int count;

		//Build Packet.
		packet.control = 1;				//"Pause"
		packet.data = remote_enable;	//enable

		//Send it.
		count = send(remotesocket, (char*)&packet, sizeof(COMMSDATA), 0);
		if (count == SOCKET_ERROR)
		{
			comms_message("Write: Socket Error");
			comms_disconnect();
			return;
		}
	}
}

//-----------------------------------------------------------------------------
// system_comms_pause_check()
//-----------------------------------------------------------------------------
void system_comms_pause_check(void)
{
	if (connection_established)
	{
		COMMSDATA packet;
		int count;
		
		count = recv(remotesocket, (char*)&packet, sizeof(COMMSDATA), MSG_PEEK);

		if (count == SOCKET_ERROR)
		{
			//No data available
			if (WSAGetLastError() == WSAEWOULDBLOCK)
				return;

			comms_message("Read: Socket Error");
			comms_disconnect();
			system_set_paused(FALSE);
			return;
		}

		switch(packet.control)
		{
		case 1:
			count = recv(remotesocket, (char*)&packet, sizeof(COMMSDATA), 0);
			system_set_paused(packet.data);
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// system_comms_read()
//-----------------------------------------------------------------------------
BOOL system_comms_read(_u8* buffer)
{
#ifdef NEOPOP_DEBUG
	if (filter_comms && buffer)
		system_debug_message("Comms: Read Byte");
#endif

	if (connection_established)
	{
		COMMSDATA packet;
		int count;
		int i;
		
		for (i = 0; i < 3; i++)
		{
			count = recv(remotesocket, (char*)&packet, sizeof(COMMSDATA), MSG_PEEK);

			if (count == SOCKET_ERROR)
			{
				//No data available
				if (WSAGetLastError() == WSAEWOULDBLOCK)
					continue;

				//Clear message list.
				ListBox_ResetContent(h_messages);
				messagecount = 0;

				comms_message("Read: Socket Error, connection broken.");
				comms_disconnect();
				return FALSE;
			}

			switch(packet.control)
			{
			case 0:
				if (buffer) //Actually get the data?
				{
					count = recv(remotesocket, (char*)&packet, sizeof(COMMSDATA), 0);
					*buffer = packet.data;
				}
				return TRUE;

			case 1:
				count = recv(remotesocket, (char*)&packet, sizeof(COMMSDATA), 0);
				system_set_paused(packet.data);
				return FALSE;
			}
		}

		return FALSE;
	}
	else
	{
		//No connection.
		return FALSE;
	}
}

//-----------------------------------------------------------------------------
// system_comms_poll()
//-----------------------------------------------------------------------------
BOOL system_comms_poll(_u8* buffer)
{
#ifdef NEOPOP_DEBUG
	if (filter_comms && buffer)
		system_debug_message("Comms: Poll Byte");
#endif

	if (connection_established)
	{
		COMMSDATA packet;
		int count;
		int i;
		
		for (i = 0; i < 3; i++)
		{
			count = recv(remotesocket, (char*)&packet, sizeof(COMMSDATA), MSG_PEEK);

			if (count == SOCKET_ERROR)
			{
				//No data available
				if (WSAGetLastError() == WSAEWOULDBLOCK)
					continue;

				//Clear message list.
				ListBox_ResetContent(h_messages);
				messagecount = 0;

				comms_message("Read: Socket Error, connection broken.");
				comms_disconnect();
				return FALSE;
			}

			switch(packet.control)
			{
			case 0:
				if (buffer) //Actually get the data?
					*buffer = packet.data;
				return TRUE;

			case 1:
				count = recv(remotesocket, (char*)&packet, sizeof(COMMSDATA), 0);
				system_set_paused(packet.data);
				return FALSE;
			}
		}

		return FALSE;
	}
	else
	{
		//No connection.
		return FALSE;
	}
}

//-----------------------------------------------------------------------------
// system_comms_write()
//-----------------------------------------------------------------------------
void system_comms_write(_u8 data)
{
#ifdef NEOPOP_DEBUG
	if (filter_comms)
		system_debug_message("Comms: Write Byte %02X", data);
#endif

	if (connection_established)
	{
		COMMSDATA packet;
		int count;

		//Build Packet.
		packet.control = 0;	//"Data"
		packet.data = data;

		//Send it.
		count = send(remotesocket, (char*)&packet, sizeof(COMMSDATA), 0);
		if (count == SOCKET_ERROR)
		{
			//Clear message list.
			ListBox_ResetContent(h_messages);
			messagecount = 0;

			comms_message("Write: Socket Error, connection broken.");
			comms_disconnect();
			return;
		}
	}
}

//=============================================================================

BOOL CALLBACK ConnectProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		system_language_patch_dialog(IDD_CONNECT, hDlg);
		return TRUE;

	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDOK:
			case IDCANCEL:
				if (connection_established == FALSE)
				{
					getRemoteAddress();
					getPort();
				}
				EndDialog(hDlg, 0);
				h_linkup = NULL;
				return 1;

			case IDC_CONNECT_BUTTON:
				system_sound_silence();
				if (connection_established)
					comms_disconnect();
				else
					comms_connect();
				return 1;

			case IDC_LISTEN_BUTTON:
				if (waiting)
				{
					waiting = FALSE;	//Stop!
				}
				else
				{
					system_sound_silence();
					comms_listen();
				}
				return 1;
			}
		}

		break;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// system_comms_connect_dialog()
//-----------------------------------------------------------------------------
void system_comms_connect_dialog(void)
{
	//Ignore multiple attempts.
	if (h_linkup)
		return;

	h_linkup = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_CONNECT), g_hWnd, ConnectProc);
	if (h_linkup == NULL)
	{
		system_message("Cannot open the link-up dialog. box");
		return;
	}

	ShowWindow(h_linkup, SW_SHOWNORMAL);

	h_ok = GetDlgItem(h_linkup, IDOK);

	h_messages = GetDlgItem(h_linkup, IDC_MESSAGES);
	h_remoteip = GetDlgItem(h_linkup, IDC_REMOTEIP_EDIT);
	h_localip = GetDlgItem(h_linkup, IDC_LOCALIP_EDIT);
	h_port = GetDlgItem(h_linkup, IDC_PORT_EDIT);

	h_connect = GetDlgItem(h_linkup, IDC_CONNECT_BUTTON);
	h_listen = GetDlgItem(h_linkup, IDC_LISTEN_BUTTON);

	//Set Remote Address
	Edit_SetText(h_remoteip, comms_remoteaddress);
	getRemoteAddress();

	//Set Port
	Edit_SetText(h_port, 0);
	getPort();

	//Set Local Address
	Edit_SetText(h_localip, "Unknown");

	comms_init();
}

//=============================================================================
