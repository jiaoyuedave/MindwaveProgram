#include <iostream>
#include <ctime>

using namespace std;

#include "thinkgear.h"

#define ALPHARANGE 4.25
#define BETARANGE  16.75
#define THETARANGE 3.25	

/**
* Prompts and waits for the user to press ENTER.
*/
void wait()
{
	cout << '\n';
	cout << "Press the Enter key...\n";
	fflush(stdout);
	getc(stdin);
}

/**
*Calculate R value for fatigue measure
*/
inline void calculateR(int *alpha, int *beta, int *theta)
{
	double alpha_m = 0;     //Store mean value
	double beta_m = 0;
	double theta_m = 0;

	double r = 0;

	for (int index = 0; index < 5; ++index){
		alpha_m += alpha[index];
		beta_m += beta[index];
		theta_m += theta[index];
	}
	alpha_m = alpha_m / (5 * ALPHARANGE);
	beta_m = beta_m / (5 * BETARANGE);
	theta_m = theta_m / (5 * THETARANGE);
	r = (alpha_m + theta_m) / beta_m;
	cout << "R: " << r << endl;

	if (r < 1.15){
		cout << "status: 清醒";
	}
	else if (r >= 1.15 && r <= 1.25){
		cout << "status: 轻度疲劳";
	}
	else if (r >= 1.25 && r <= 1.35){
		cout << "status: 中度疲劳";
	}
	else if (r >= 1.35 && r <= 1.45){
		cout << "status: 重度疲劳";
	}
	else{
		cout << "status: 瞌睡" << endl;
	}
	cout << endl;
}
/**
* Program which prints ThinkGear Raw Wave Values to stdout.
*/
int main(void) {

	/* Print driver version number */
	int dllVersion = TG_GetDriverVersion();
	cout << "ThinkGear DLL version: " << dllVersion << endl;

	/* Get a connection ID handle to ThinkGear */
	int connectionId = TG_GetNewConnectionId();
	if (connectionId < 0) {
		cerr << "ERROR: TG_GetNewConnectionId() returned " << connectionId << endl;
		wait();
		exit(EXIT_FAILURE);
	}

	/* Set/open stream (raw bytes) log file for connection */
	int errCode = TG_SetStreamLog(connectionId, "streamLog.txt");
	if (errCode < 0) {
		cerr << "ERROR: TG_SetStreamLog() returned " << errCode << endl;
		wait();
		exit(EXIT_FAILURE);
	}

	/* Set/open data (ThinkGear values) log file for connection */
	errCode = TG_SetDataLog(connectionId, "dataLog.txt");
	if (errCode < 0) {
		fprintf(stderr, "ERROR: TG_SetDataLog() returned %d.\n", errCode);
		cerr << "ERROR: TG_SetDataLog() returned " << errCode << endl;
		wait();
		exit(EXIT_FAILURE);
	}

	/* Attempt to connect the connection ID handle to serial port "COM5" */
	/* NOTE: On Windows, COM10 and higher must be preceded by \\.\, as in
	*       "\\\\.\\COM12" (must escape backslashes in strings).  COM9
	*       and lower do not require the \\.\, but are allowed to include
	*       them.  On Mac OS X, COM ports are named like
	*       "/dev/tty.MindSet-DevB-1".
	*/
	char *comPortName = "\\\\.\\COM3";
	errCode = TG_Connect(connectionId,
		comPortName,
		TG_BAUD_57600,
		TG_STREAM_PACKETS);
	if (errCode < 0) {
		cerr << "ERROR: TG_Connect() returned " << errCode << endl;
		wait();
		exit(EXIT_FAILURE);
	}

	/*Break until not able to read value.*/
	while (1) {
		
        /*Store recent 10 values*/
		int alpha[10];
		int beta[10];
		int theta[10];

		/* Read all currently available Packets, one at a time... */
		int packetsRead = 0;
		do {

			/* Read a single Packet from the connection */
			packetsRead = TG_ReadPackets(connectionId, 1);

			/* If TG_ReadPackets() was able to read a Packet of data... */
			if (packetsRead == 1) {
				
                /* If the Packet containted a new raw wave value... */
				if ((TG_GetValueStatus(connectionId, TG_DATA_ALPHA1) != 0) ||
					(TG_GetValueStatus(connectionId, TG_DATA_ALPHA2) != 0) ||
					(TG_GetValueStatus(connectionId, TG_DATA_BETA1) != 0) ||
					(TG_GetValueStatus(connectionId, TG_DATA_BETA2) != 0) ||
					(TG_GetValueStatus(connectionId, TG_DATA_THETA) != 0)) {

					/* Get the current time as a string */
					time_t currTime = time(NULL);
					char *currTimeStr = ctime(&currTime);

					/* Get and print out the new raw value */
					static int count = 0;
					alpha[count] = (int)(TG_GetValue(connectionId, TG_DATA_ALPHA1) + TG_GetValue(connectionId, TG_DATA_ALPHA2));
					beta[count] = (int)(TG_GetValue(connectionId, TG_DATA_BETA1) + TG_GetValue(connectionId, TG_DATA_BETA2));
					theta[count] = (int)TG_GetValue(connectionId, TG_DATA_THETA);
					double r = (double)(alpha[count] / ALPHARANGE + theta[count] / THETARANGE) / (beta[count] / BETARANGE);
					cout << currTimeStr;
					cout << '\t' << "alpha: " << alpha[count] << '\t' << "beta: " << beta[count]
						<< '\t' << "theta: " << theta[count] << '\t' << "r: " << r << endl;
					fflush(stdout);

					count++;
					if (count >= 10){
						calculateR(alpha, beta, theta);
						count = 0;
					}

				} /* end "If Packet contained a raw wave value..." */

			} /* end "If TG_ReadPackets() was able to read a Packet..." */
			else{
				break;
			}

		} while (packetsRead > 0); /* Keep looping until all Packets read */

	} 

	



	/* Clean up */
	TG_FreeConnection(connectionId);

	/* End program */
	wait();
	return(EXIT_SUCCESS);
}
