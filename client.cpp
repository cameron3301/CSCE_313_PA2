/*
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date  : 2/8/19
 */
#include "common.h"
#include "FIFOreqchannel.h"

using namespace std;


char* createFileMsgBuff(__int64_t _offset, int _length, char* _filename) {
    filemsg fm(_offset, _length);
    const int size = sizeof(filemsg) + strlen(_filename) + 1;
    char* buff = new char[size];
    memcpy(buff, &fm, sizeof(filemsg));
    memcpy(buff + sizeof(filemsg), _filename, strlen(_filename) + 1);

    return buff;
}


int main(int argc, char *argv[]){
    int n = 100;    // default number of requests per "patient"
	int p = 15;		// number of patients
    srand(time_t(NULL));

    FIFORequestChannel chan ("control", FIFORequestChannel::CLIENT_SIDE);
    struct timeval start, end;
    double time_taken;

    /* -------------- */
    /* --- TASK 1 --- */
    /* -------------- */

    // Initiliaze first data_msg request
    datamsg d_msg(1, 0.0, 1);

    // Open connection for output file
    ofstream myfile;
    myfile.open("received/x1.csv");

    gettimeofday(&start, NULL);

    for (double t = 0; t <= 59.996; t += 0.004) {
        d_msg.seconds = t;

        // Get data point for ecg1
        d_msg.ecgno = 1;
        chan.cwrite(&d_msg, sizeof(d_msg));
        double d_point1 = *(double*) chan.cread();

        // Get data point for ecg2
        d_msg.ecgno = 2;
        chan.cwrite(&d_msg, sizeof(d_msg));
        double d_point2 = *(double*) chan.cread();

        // Write data to output csv file
        cout << t << " , " << d_point1 << " , " << d_point2 << endl;
        myfile << t << "," << d_point1 << "," << d_point2 << "\n";
    }

    // Close connection for output file
    myfile.close();

    gettimeofday(&end, NULL);
    time_taken = (end.tv_sec - start.tv_sec) * 1e6;
    time_taken = (time_taken + (end.tv_sec - start.tv_sec)) * 1e-6;
    cout << "Time Taken (Data Pts) = " << time_taken << endl;


    /* -------------------------- */
    /* --------- TASK 2 --------- */
    /* -------------------------- */

    char* filename;
    char* buff;
    char* file_data;
    int w, offset;
    __int64_t file_length;

    /* --- PART A: Transfer a .csv file --- */
    filename = "2.csv";
    buff = createFileMsgBuff(0, 0, filename);
    const int size = sizeof(filemsg) + strlen(filename) + 1;

    w = chan.cwrite(buff, size);
    file_length = *(__int64_t*) chan.cread();

    offset = 0;

    ofstream wf("received/y2.csv", ios_base::app | ios::binary);
    if (!wf) {
        cout << "Cannot Open File!" << endl;
        MESSAGE_TYPE m = QUIT_MSG;
        chan.cwrite (&m, sizeof (MESSAGE_TYPE));
    }

    gettimeofday(&start, NULL);
    while (true)
    {
        if (file_length - offset >= MAX_MESSAGE)
        {
            buff = createFileMsgBuff(offset, MAX_MESSAGE, filename);
            w = chan.cwrite(buff, size);
            file_data = chan.cread();
            wf.write(file_data, MAX_MESSAGE);
        }
        else
        {
            int final_chunk = file_length - offset;

            buff = createFileMsgBuff(offset, final_chunk, filename);
            w = chan.cwrite(buff, size);
            file_data = chan.cread();
            wf.write(file_data, final_chunk);

            break;
        }
        offset += MAX_MESSAGE;
    }

    wf.close();

    // Calculate the time taken during the file transfer
    gettimeofday(&end, NULL);
    time_taken = (end.tv_sec - start.tv_sec) * 1e6;
    time_taken = (time_taken + (end.tv_sec - start.tv_sec)) * 1e-6;
    cout << "Time Taken (CSV File) = " << time_taken << endl;
    cout << "File size of 2.csv = " << get_file_size("BIMDC/2.csv") << endl;
    cout << "File size of y2.csv = " << get_file_size("y2.csv") << endl;



    /* --- Part B: Transfer a .txt file --- */
    filename = "file.txt";
    buff = createFileMsgBuff(0, 0, filename);
    const int size2 = sizeof(filemsg) + strlen(filename) + 1;

    w = chan.cwrite(buff, size2);
    file_length = *(__int64_t*) chan.cread();

    offset = 0;

    ofstream wf2("received/y_file.txt", ios_base::app | ios::binary);
    if (!wf2) {
        cout << "Cannot Open File!" << endl;
        MESSAGE_TYPE m = QUIT_MSG;
        chan.cwrite (&m, sizeof (MESSAGE_TYPE));
    }

    gettimeofday(&start, NULL);
    while (true) 
    {
        cout << "file_length - offset = " << (file_length - offset) << endl;
        if (file_length - offset > MAX_MESSAGE)
        {
            buff = createFileMsgBuff(offset, MAX_MESSAGE, filename);
            w = chan.cwrite(buff, size2);
            file_data = chan.cread();
            wf2.write(file_data, MAX_MESSAGE);
        }
        else
        {
            int final_chunk = file_length - offset;

            buff = createFileMsgBuff(offset, final_chunk, filename);
            w = chan.cwrite(buff, size2);
            file_data = chan.cread();
            wf2.write(file_data, final_chunk);

            break;
        }
        offset += MAX_MESSAGE;
    }

    wf2.close();

    // Calculate the time taken during the file transfer
    gettimeofday(&end, NULL);
    time_taken = (end.tv_sec - start.tv_sec) * 1e6;
    time_taken = (time_taken + (end.tv_sec - start.tv_sec)) * 1e-6;
    cout << "Time Taken (TXT File) = " << time_taken << endl;
    cout << "File size of file.txt = " << get_file_size("BIMDC/file.txt") << endl;
    cout << "File size of y_file.txt = " << get_file_size("y_file.txt") << endl;



    /* -------------- */
    /* --- TASK 3 --- */
    /* -------------- */

    // Create NewChannelMsg

    MESSAGE_TYPE msg = NEWCHANNEL_MSG;
    chan.cwrite(&msg, sizeof(MESSAGE_TYPE));
    string chan_name= chan.cread();
    FIFORequestChannel new_chan (chan_name, FIFORequestChannel::CLIENT_SIDE);

    cout << "New channel's name = " << chan_name << endl;
    
    // Get Test datamsg 1
    datamsg test_d_msg(3, 0, 1);
    new_chan.cwrite(&test_d_msg, sizeof(test_d_msg));
    double dp = *(double*) new_chan.cread();
    cout << "Value for Person 3 at 0 seconds for ecg1 = " << dp << endl;

    // Get Test datamsg 2
    test_d_msg.seconds = 2.1;
    new_chan.cwrite(&test_d_msg, sizeof(test_d_msg));
    dp = *(double*) new_chan.cread();
    cout << "Value for Person 3 at 2.1 seconds for ecg1 = " << dp << endl;

    // Get Test datamsg 3
    test_d_msg.ecgno = 2;
    test_d_msg.person = 4;
    new_chan.cwrite(&test_d_msg, sizeof(test_d_msg));
    dp = *(double*) new_chan.cread();
    cout << "Value for Person 4 at 2.1 seconds for ecg2 = " << dp << endl;


    /* --- Task 4 --- */






    // closing the channel    
    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite (&m, sizeof (MESSAGE_TYPE));

   
}
