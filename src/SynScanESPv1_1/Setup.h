// Setup.h
// Hardware specific config for MyFocuser
//
// Copyright (c) 2018 Roman Hujer   http://hujer.net
//
//
//

#define DEBUG_OFF

// UART setup
#define UART_BAUD       9600
#define UART_TIMEOUT    5       // ms (if nothing more on UART, then send packet)
#define UART_BUFFERSIZE 512

// WiFi setup
#define PKT_BUFFERSIZE  2048

// Ports
#define PROTOCOL_UDP            // Synscan default UDP port 11880
#define UDP_PORT    11880
#undef  PROTOCOL_TCP            // Synscan default TCP port  4030
#define TCP_PORT    4030

// Mode
// phone connects to ESP via WiFi router (needs router ssid, password)
#undef MODE_STA
// phone connects to ESP directly (default SynScan ssid, password)
#define MODE_AP 
