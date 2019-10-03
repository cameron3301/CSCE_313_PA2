/*
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date  : 2/8/19
 */
#include "common.h"
#include "FIFOreqchannel.h"

using namespace std;

int main(int argc, char *argv[]){
    int n = 100;    // default number of requests per "patient"
	int p = 15;		// number of patients
    srand(time_t(NULL));

    FIFORequestChannel chan ("control", FIFORequestChannel::CLIENT_SIDE);

    // sending a non-sense message, you need to change this
    /*
    char x = 55;
    chan.cwrite (&x, sizeof (x));
    char* buf = chan.cread ();
    */

    /* -------------- */
    /* --- TASK 1 --- */
    /* -------------- */

    // Initiliaze first data_msg request
    datamsg d_msg(1, 0.0, 1);
    double d_point1;
    double d_point2;

    // Open connection for output file
    ofstream myfile;
    myfile.open("x1.csv");

    for (double t = 0; t < 60; t += 0.004) {
        d_msg.seconds = t;

        // Get data point for ecg1
        d_msg.ecgno = 1;
        chan.cwrite(&d_msg, sizeof(d_msg));
        d_point1 = *(double*) chan.cread();

        // Get data point for ecg2
        d_msg.ecgno = 2;
        chan.cwrite(&d_msg, sizeof(d_msg));
        d_point2 = *(double*) chan.cread();

        // Write data to output csv file
        myfile << t << "," << d_point1 << "," << d_point2 << "\n";
    }

    // Close connection for output file
    myfile.close();



    // closing the channel    
    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite (&m, sizeof (MESSAGE_TYPE));

   
}
