/* Print Debug Info. Uncomment the #define below to print debugging to stderr */

//#define DEBUG

/* Number of seconds to timeout */
#define CONNECT_TIMEOUT 5
#define MAX_RETRIES 5
/******************************************************************************

Esensors EM01 Plugin.
Description: This plugin is written mainly for Nagios, but can be
easily used for other software too.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Credits:
Duncan Robertson [Duncan.Robertson@vsl.com.au] - 64bits fix
Ali Rahimi [ali@XCF.Berkeley.EDU] - Sockets code
David W. Deley [deleyd@cox.net] - Random numbers

$Id: check_em01.c,v 2.1 3:47 PM 1/26/2009 $

******************************************************************************/
/** This value is to multiple the Voltage value detected by a fixed constant. (
    * i.e. if you are using a regulator supply to measure AC voltage)
    */
const float VOLTAGE_MULTIPLIER = 1.0;
const float VOLTAGE_OFFSET = 0.0; 

/**
   *  Tells the plugin to reset contact closure on trigger. Change to 0 to turn this off.
   */
const int RESETCONTACT = 1;


const int DEFAULTPORT = 80;
const char *progname = "check_em01";
const char *revision = "$Revision: 2.1 $";
const char *copyright = "2009";
const char *email = "techhelp@eEsensors.com";

typedef int SOCKET;
typedef enum {E_NET_ERRNO=-1, E_NET_OK=0} NetErrnoType;

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

void     INThandler(int);

static NetErrnoType net_errno = E_NET_OK;
static int saved_errno = 0;
static SOCKET s;


/* Translations between ``net_errno'' values and human readable strings.
*/
static const char *net_syserrlist[] = {
	"All was chill"
};




#ifdef STRERROR_NOT_DEFINED
const char *strerror(int errno) { return sys_errlist[errno]; }
#endif

static void NetSetErrno(NetErrnoType e)
{
	if(e == E_NET_ERRNO)saved_errno = errno;
	net_errno = e;
}



/* NetErrStr()
*--------------------------------------------------------------------
* Returns a diagnostic message for the last failure.
*/
const char *NetErrStr()
{
	return net_errno==E_NET_ERRNO ? strerror(saved_errno) :
		net_syserrlist[net_errno];
}

/* NetErrNo()
*--------------------------------------------------------------------
* Returns a diagnostic number for the last failure.
*/
NetErrnoType NetErrNo()
{
	return net_errno;
}

/* NetMakeContact()
*--------------------------------------------------------------------
* Makes a tcp connection to a host:port pair.
*--------------------------------------------------------------------
* ``Hostname'' can either be in the form of a hostname or an IP address
* represented as a string. If the hostname is not found as it is,
* ``hostname'' is assumed to be an IP address, and it is treated as such.
*
* If the lookup succeeds, a TCP connection is established with the
* specified ``port'' number on the remote host and a stream socket is
* returned.
*
* On any sort of error, an error code can be obtained with @NetErrNo()
* and a message with @NetErrStr().
*/
SOCKET
NetMakeContact(const char *hname, int port)
{
	int fd;
	struct sockaddr_in addr;
	struct hostent *hent;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1)
	{
		NetSetErrno(E_NET_ERRNO);
		return -1;
	}


	hent = gethostbyname(hname);
	if(hent == NULL)
		addr.sin_addr.s_addr = inet_addr(hname);
	else
		memcpy(&addr.sin_addr, hent->h_addr, hent->h_length);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

#ifdef DEBUG
	fprintf(stderr, "Creating Connection... ");
#endif

	if(connect(fd, (struct sockaddr *)&addr, sizeof(addr)))
	{
		NetSetErrno(E_NET_ERRNO);
		return -1;
	}

	NetSetErrno(E_NET_OK);
	return fd;
}



/*********************************************************************************/

float Exp10(int n)
{
	int i;
	float result = 1;
	for(i =n; i; i--)
		result *= 10;
	return result;
}

float myatof(const char *s)
{
	float result;
	int val = 0, dec = 0, n = 0;;
	int neg = 0;

	/* skip white space */
	while (*s == ' ' || *s == '\t') {
		s++;
	}

	if (*s == '-') {
		neg = 1;
		s++;
	} else if (*s == '+') {
		s++;
	}
	while (*s >= '0' && *s <= '9') {
		val *= 10;
		val += *s++ - '0';
	}
	result = val;

	if(*s == '.') {
		*s++;
		while (*s >= '0' && *s <= '9') {
			dec *= 10;
			dec += *s++ - '0';
			n++;
		}

		if(n)
			result += dec/Exp10(n);
	}

	if (neg) {
		result = -result;
	}

	return result;
}

/*
This function is an abstraction layer between app and sockets.
Currently, it only passes down all the arguments it receives from
app. However, in the future, it will be easier to change 
sockets library and not affect the main app code at all.
*/
SOCKET connectWebsensor (char* hostname, int port){
	//SOCKET s;
	fd_set readfds;

	int i;
	s = NetMakeContact(hostname,port);
	if(s==-1) {
		return -1;
	}
	else{
		/* Make the socket non-blocking */
		ioctl(s, FIONBIO, 1);
		FD_ZERO(&readfds);
		FD_SET(0, &readfds);
		FD_SET(s, &readfds);
		return s;

	}
}

/*
This function is called when a timeout has occurred.
It shutsdown the socket and allows the main loop to continue.
*/
void  INThandler(int sig)
{
     signal(SIGALRM, SIG_IGN);
     shutdown(s, SHUT_RDWR);
     alarm(CONNECT_TIMEOUT);
     signal(SIGALRM, INThandler);
}


main(int argc, char **argv)
{
	int l, retry, i, random_backoff;
	long ms;
	float data;
	char iobuf[1024], timestr[32], datachar[7];
	char* pos;
	char rcvd_checksum, calc_checksum;
	time_t start_time, cur_time;
	double r, x;
	SOCKET contacts;

	progname = argv[0];
	if(argc < 2 || strcmp(argv[1],"--help") == 0) {
		print_help();
		return(3);
	}

	time(&cur_time);
	sprintf(timestr, "%s", ctime(&cur_time));

	signal(SIGALRM, INThandler);
	alarm(CONNECT_TIMEOUT);
	
	for(retry = 0; retry < MAX_RETRIES; retry++){

		srand((unsigned int)time(NULL));

		r = (   (double)rand() / ((double)(RAND_MAX)+(double)(1)) ); 
		x = (r * 20); 	
		random_backoff = (int) x; 

#ifdef DEBUG
		fprintf(stderr, "Sleeping: %d ", 100+random_backoff);
#endif
		for(ms=0; ms < 100+random_backoff; ms++){
			usleep(5000); 
		}


		/* make connection to websensor */
		s = connectWebsensor(argv[1], DEFAULTPORT);
#ifdef DEBUG
		fprintf(stderr, "Socket created ");
#endif

		if(NetErrNo() != 0){
#ifdef DEBUG
			fprintf(stderr, "Could not connect to Websensor because %s, will retry %d more times.\n", NetErrStr(), 10-retry);
#endif
			shutdown(s, SHUT_RDWR);
			continue;
		}


		/* send HTTP GET request to obtain data */
		if(argc>2){

			switch (toupper(argv[2][0])){
	case 'R':
		write(s, "GET /index.html?eR HTTP/1.1\r\nUser-Agent: EsensorsPlugin\r\nHost: localhost\r\n\r\n", 77);
		break;

	case 'V':
		write(s, "GET /index.html?eV HTTP/1.1\r\nUser-Agent: EsensorsPlugin\r\nHost: localhost\r\n\r\n", 77);
		break;

	default:
		write(s, "GET /index.html?em123456 HTTP/1.1\r\nUser-Agent: EsensorsPlugin\r\nHost: localhost\r\n\r\n", 83);
		break;
			}
		}

		else{ // Not enough arguments from command line. Use default websensor command.
			write(s, "GET /index.html?em123456 HTTP/1.1\r\nUser-Agent: EsensorsPlugin\r\nHost: localhost\r\n\r\n", 83);
		}

#ifdef DEBUG
		fprintf(stderr, "Wrote to Socket ");
#endif

		l = read(s, iobuf, sizeof(iobuf));

#ifdef DEBUG
		fprintf(stderr, "Read from socket ");
#endif
	
		/* No data returned from websensor. Will retry again. */

		if(l<=0){
#ifdef DEBUG
			fprintf(stderr, "No Data Read, will retry %d more times.\n", MAX_RETRIES-retry);
#endif
			shutdown(s, SHUT_RDWR);
			continue;
		}

		pos = strstr(iobuf, "<body>");
		if(pos == 0){
			printf("Invalid data received.\n");
			return(3);  
		}



		//Search for the sensor data string
		pos = strstr(iobuf, "TF:");
		if(pos == 0){
			pos = strstr(iobuf, "TC:");
			if(pos == 0){
#ifdef DEBUG			
				fprintf(stderr, "Using default parsing parameters.\n");			
#endif
				pos=&iobuf[167];
			}
		}

		if(argc>2 && toupper(argv[2][0]) == 'V'){ //If the command was for voltage measurement, try looking for the string 'RV' instead.
			pos = strstr(iobuf, "RV");
			if(pos == 0 || pos[1] != 'V'){
#ifdef DEBUG
				fprintf(stderr, "Invalid Data Received. Will retry %d more times.\n", MAX_RETRIES-retry);
#endif
				shutdown(s, SHUT_RDWR);
				continue;
			}
			else{
				pos = pos - 2;
				break; //Voltage data looks good
			}
		}
	
		pos = pos - 2;
		/* Unsupported command. OBSOLETE CODE */
		if(pos[0] == '#'){
			printf("Invalid Command. Option %c selected may not be available for this websensor.\n", argv[2][0]);
			return 3;
		}	

		/* The http data is not properly formatted. */
		if(pos[2] != 'T' && pos[2] != 'R' && pos[1] != 'v'){
#ifdef DEBUG
			fprintf(stderr, "Data input incorrect, will retry %d more times.\n", MAX_RETRIES-retry);
#endif
			shutdown(s, SHUT_RDWR);
			continue;
		}
		else{
			break; /* All data looks good. Break out of loop. */
		}
	}

	time(&cur_time);
	sprintf(timestr, "%s", ctime(&cur_time));

    /* Retried 3 times earlier and still no good data. Time to exit */
	if(retry >= MAX_RETRIES){
		printf("NO DATA\n");
		return 3;
	}
	else{
		shutdown(s, SHUT_RDWR);
	}

	if(argc > 2){
		switch(toupper(argv[2][0])){

		
	case 'G':  //Cacti/RRDTool Output TempF:** Humid:**
		{
			strncpy(datachar, pos+5, 5);
			datachar[5] = '\0';
			data = myatof(datachar);
			data += 0.01;
			printf("Temp:%3.2f ", data);
			//printf("TempUnit:%c ", pos[3]); //prints the temperature unit
			
			strncpy(datachar, pos+13, 4);
			datachar[4] = '\0';
			data = myatof(datachar);
			data += 0.01;
			printf("Humid:%3.2f ", data);
			
			strncpy(datachar, pos+21, 5);
			datachar[5] = '\0';
			data = myatof(datachar);
			data += 0.01;
			printf("Illum:%3.2f\n", data);
			
			return(0);
		}
		break;
		
		
		
	case 'T': 
		{
			strncpy(datachar, pos+5, 5);
			datachar[5] = '\0';
			data = myatof(datachar);
			data += 0.01;
			if(argc != 7){
				printf("(No limits specified) Temperature: %s %c | Temp%c=%3.2f\n", datachar, pos[3], pos[3], data);
				return(0);
			}
			if(data < myatof(argv[5]) || data > myatof(argv[6])){
				printf("CRITICAL ( %s< or >%s ) Temperature: %s %c | Temp%c=%3.2f\n", argv[5], argv[6], datachar, pos[3], pos[3], data);
				return(2);
			}
			else if(data < myatof(argv[3]) || data > myatof(argv[4])){
				printf("WARNING ( %s< or >%s ) Temperature: %s %c | Temp%c=%3.2f\n", argv[3], argv[4], datachar, pos[3], pos[3], data);
				return(1);
			}
			else{
				printf("OK Temperature: %s %c | Temp%c=%3.2f\n", datachar, pos[3], pos[3], data);
				return(0);
			}
		}
		break;

	case 'H': 
		{
			strncpy(datachar, pos+13, 4);
			datachar[4] = '\0';
			data = myatof(datachar);
			data += 0.01;
			if(argc != 7){
				printf("(No limits specified) %s% | Humid=%3.2f\n", datachar, data);
				return(0);
			}
			if(data < myatof(argv[5]) || data > myatof(argv[6])){
				printf("CRITICAL ( %s< or >%s ) Humidity: %s% | Humid=%3.2f\n", argv[5], argv[6], datachar, data);
				return(2);
			}
			else if(data < myatof(argv[3]) || data > myatof(argv[4])){
				printf("WARNING ( %s< or >%s ) Humidity: %s% | Humid=%3.2f\n", argv[3], argv[4], datachar, data);
				return(1);
			}
			else{
				printf("OK Humidity: %s% | Humid=%3.2f\n", datachar, data);
				return(0);
			}
		}
		break;

	case 'I': 
		{
			strncpy(datachar, pos+21, 5);
			datachar[5] = '\0';
			data = myatof(datachar);
			data += 0.01;
			if(argc != 7){
				printf("(No limits specified) Illumination: %s | Illum=%3.2f\n", datachar, data);
				return(0);
			}
			if(data < myatof(argv[5]) || data > myatof(argv[6])){
				printf("CRITICAL ( %s< or >%s ) Illumination: %s | Illum=%3.2f\n", argv[5], argv[6], datachar, data);
				return(2);
			}
			else if(data < myatof(argv[3]) || data > myatof(argv[4])){
				printf("WARNING ( %s< or >%s ) Illumination: %s | Illum=%3.2f\n", argv[3], argv[4], datachar, data);
				return(1);
			}
			else{
				printf("OK Illumination: %s | Illum=%3.2f\n", datachar,data);
				return(0);
			}
		}
		break;

	case 'C':
		{
			if(iobuf[160] == 'W'){
				printf("OK Contacts Close. | Contacts=0\n");
				return(0);
			}
			else if(iobuf[160] == 'N'){
				printf("CRITICAL Contacts Open! | Contacts=1\n");
				
				/* Reset the Contact Closure back to Closed */
				if(RESETCONTACT == 1){
					contacts = connectWebsensor(argv[1], DEFAULTPORT);
					if(NetErrNo() != 0){
						fprintf(stderr, "Could not reset contact closure", NetErrStr());
						shutdown(contacts, SHUT_RDWR);
						return(2);
					}
					write(contacts, "GET /index.html?eL HTTP/1.1\r\nUser-Agent: EsensorsPlugin\r\nHost: localhost\r\n\r\n", 77);
					l = read(contacts, iobuf, sizeof(iobuf));
					//printf("%s", iobuf);
					shutdown(contacts, SHUT_RDWR);	
				}
				return(2);
			}
			else{
				printf("WARNING Unknown status. Try Reset Device.\n");
				return(1);
			}
		}
		break;

	case 'R':
		{
			strncpy(datachar, pos+4, 6);
			datachar[6] = '\0';
			data = myatof(datachar);
			data += 0.01;
			if(argc != 7){
				printf("(No limits specified) Ext. Temperature: %s %c | XTemp%c=%3.2f\n", datachar, pos[3], pos[3], data);
				return(0);
			}
			if(data < myatof(argv[5]) || data > myatof(argv[6])){
				printf("CRITICAL ( %s< or >%s ) Ext. Temperature: %s %c | XTemp%c=%3.2f\n", argv[5], argv[6], datachar, pos[3], pos[3], data);
				return(2);
			}
			else if(data < myatof(argv[3]) || data > myatof(argv[4])){
				printf("WARNING ( %s< or >%s ) Ext. Temperature: %s %c | XTemp%c=%3.2f\n", argv[3], argv[4], datachar, pos[3], pos[3], data);
				return(1);
			}
			else{
				printf("OK Ext. Temperature: %s %c | XTemp%c=%3.2f\n", datachar, pos[3], pos[3], data);
				return(0);
			}
		}
		break;

	case 'V':
		{
			strncpy(datachar, pos+21, 5);
			datachar[5] = '\0';
			data = myatof(datachar);
			data = (data * VOLTAGE_MULTIPLIER) + VOLTAGE_OFFSET;
			data += 0.01;
			if(data <= VOLTAGE_OFFSET+(0.5*VOLTAGE_MULTIPLIER)){ //Anything less than VOLTAGE_OFFSET AC should be considered as zero
				data = 0.00;
			}
			
			if(argc != 7){
				printf("(No limits specified) Voltage: %3.2f V | Voltage=%3.2f\n", data,data);
				return(0);
			}
			if(data < myatof(argv[5]) || data > myatof(argv[6])){
				printf("CRITICAL ( %s< or >%s ) Voltage: %3.2f V | Voltage=%3.2f\n", argv[5], argv[6], data, data);
				return(2);
			}
			else if(data < myatof(argv[3]) || data > myatof(argv[4])){
				printf("WARNING ( %s< or >%s ) Voltage: %3.2f V | Voltage=%3.2f\n", argv[3], argv[4], data, data);
				return(1);
			}

			else{
				printf("OK Voltage: %3.2f V | Voltage=%3.2f\n", data,data);
				return(0);
			}
		}

	default : 
		printf("Please choose only 'T', 'H', 'I', 'R', 'V' or 'C'.\n Please refer to README for further instructions.\n");
		break;
		}
	}
	else{
		iobuf[195] = 0;
		fprintf(stderr, "2");
		printf("%s\t%s", pos+2, timestr);
		fprintf(stderr, "3");
	}

	return 0;
}

int print_help (void)
{
	fprintf (stdout, "Copyright (c) 2005-2009 Esensors, Inc <TechHelp@eEsensors.com>\n");
	fprintf (stdout,("This plugin is written mainly for Nagios/Cacti/RRDTool, but can work standalone too. \nIt returns the HVAC data from the EM01 Websensor\n\n"));
	print_usage ();
	return 0;
}



int print_usage (void)
{
	fprintf(stdout, "usage: \n %s [hostname] [T/H/I] [LowWarning HighWarning LowCritical HighCritical]\n\n", progname); 
	fprintf(stdout, "Only the hostname is mandatory. The rest of the arguments are optional\n");
	fprintf(stdout, "T is for Temperature data, H for Humidity Data and I for Illumination data\nExamples:\n");
	fprintf(stdout, "This will return all HVAC data: \n %s 192.168.0.2\n\n", progname); 
	fprintf(stdout, "This will return only Illumination data: \n %s 192.168.0.2 I\n\n", progname); 
	fprintf(stdout, "This will return temperature data with status: \n %s 192.168.0.2 T 65 85 60 90\n\n", progname); 
	fprintf(stdout, "This will return humidity data with status: \n %s 192.168.0.2 H 25.5 50 15 70.5\n\n", progname); 
	fprintf(stdout, "This will return Cacti/RRDTool format: \n %s 192.168.0.2 G\n\n", progname); 	
	fprintf(stdout, "For further information, please refer to the README file included with the package\n");
	fprintf(stdout, "or available for download from http://www.eesensors.com\n");
	return 0;
}
