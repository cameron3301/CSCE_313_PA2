/*
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date  : 2/8/19
 */
#include "common.h"
#include "FIFOreqchannel.h"
#include <stdexcept>

using namespace std;


// convert a character array of numbers to an integer
double charsToDouble(char* c) {
  string s(c);
  stringstream foo(s);

  double out = 0;
  foo >> out;

  return out;
}


// convert a character array of numbers to an integer
int charsToInt(char* c) {
  string s(c);
  stringstream foo(s);

  int out = 0;
  foo >> out;

  return out;
}


// ensures that valid input data is given for DATA_MSG
void validateDataMsgArgs(int p, double s, int e) {
    // Person must be between 1 and 15, inclusive
    if ((p < 1) || (p > 15)) {
        throw invalid_argument("DataMsg Input Error: Person must be between 1 and 15, inclusive");
    }

    // Seconds must be between 0 and 59.996 inclusive
    if ((s < 0) || (s > 59.996)) {
        throw invalid_argument("DataMsg Input Error: Seconds must be between 0 and 59.996, inclusive");
    }

    // Seconds must be an interval of 0.004
    double sRounded = round(s * 1000.0);
    int sRoundedInt = (int) sRounded;
    if ((sRoundedInt % 4) != 0) {
        throw invalid_argument("DataMsg Input Error: Seconds must be divisible by 0.004");
    }


    // Ecgno must either be 1 or 2
    if ((e != 1) && (e != 2)) {
        throw invalid_argument("DataMsg Input Error: Ecgno must either be 1 or 2");
    }
}


char* createFileMsgBuff(__int64_t _offset, int _length, char* _filename) {
    filemsg fm(_offset, _length);
    const int size = sizeof(filemsg) + strlen(_filename) + 1;
    char* buff = new char[size];
    memcpy(buff, &fm, sizeof(filemsg));
    memcpy(buff + sizeof(filemsg), _filename, strlen(_filename) + 1);

    return buff;
}


int main(int argc, char *argv[])
{
    
    // getopt() functionality
    int opt;

    char* d_msg_person_chars = NULL;
    char* d_msg_sec_chars = NULL;
    char* d_msg_ecgno_chars = NULL;
    char* f_msg_filename = NULL;
    bool new_chan_msg_bool = false;

    while ((opt = getopt(argc, argv, ":p:t:e:f:c")) != -1) {
        switch(opt)
        {
            case 'p':
                d_msg_person_chars = optarg;
                break;
            case 't':
                d_msg_sec_chars = optarg;
                break;
            case 'e':
                d_msg_ecgno_chars = optarg;
                break;
            case 'f':
                f_msg_filename = optarg;
                break;
            case 'c':
                new_chan_msg_bool = true;
                break;
            case ':':
                throw invalid_argument("Missing input value");
                break;
            case '?':
                throw invalid_argument("Unknown tag");
                break;
        }
    }

    int d_msg_person;
    double d_msg_sec;
    int d_msg_ecgno;
    bool d_msg_bool = false;
    bool d_msg_file_bool = false;

    // validate datamsg
    // Check if at least one of the 3 tags for DATAMSG input was provided.
    //   -> if at least 1 was provided, ensure that all 3 were provided.
    //      -> if so, retrieve the numeric input values (convert from char*)
    //      -> if not, throw an error
    if (d_msg_person_chars || d_msg_sec_chars || d_msg_ecgno_chars) {
        if (d_msg_person_chars && d_msg_sec_chars && d_msg_ecgno_chars)
        {
            // assign numeric values
            d_msg_person = charsToInt(d_msg_person_chars);
            d_msg_sec = charsToDouble(d_msg_sec_chars);
            d_msg_ecgno = charsToInt(d_msg_ecgno_chars);

            // validate datamsg input values
            validateDataMsgArgs(d_msg_person, d_msg_sec, d_msg_ecgno);

            // set d_msg_bool boolean to true -> perform d_msg operation
            d_msg_bool = true;
        }
        else if (d_msg_person_chars && (d_msg_sec_chars == NULL) && (d_msg_ecgno_chars == NULL))
        {
            // assign numeric value
            d_msg_person = charsToInt(d_msg_person_chars);

            // validate input
            if ((d_msg_person < 1) || (d_msg_person > 15)) {
                throw invalid_argument("DataMsg Input Error: Person must be between 1 and 15, inclusive");
            }

            // set d_msg_file_bool to true -> perform d_msg file operation
            d_msg_file_bool = true;
        }
        else
        {
            throw invalid_argument("All three input tags (-p -t -s) or just the (-p) input tag must be provided for a datamsg request");
        }
    }

    // validate filemsg
    if (f_msg_filename) {
        // check if file exists
        // ...
    }




    // FOOOOOOORK
    if (!fork())
    {
        char* args[] = {"./dataserver", NULL};
        execvp(args[0], args);
    }
    else
    {
        int n = 100;    // default number of requests per "patient"
        int p = 15;		// number of patients
        srand(time_t(NULL));
    
        FIFORequestChannel chan ("control", FIFORequestChannel::CLIENT_SIDE);
        struct timeval start, end;
        double time_taken;



        // if -c is passed, create a new channel
        if (new_chan_msg_bool)
        {
            // Create a new channel
            MESSAGE_TYPE msg = NEWCHANNEL_MSG;
            chan.cwrite(&msg, sizeof(MESSAGE_TYPE));
            string chan_name = chan.cread();
            FIFORequestChannel new_chan (chan_name, FIFORequestChannel::CLIENT_SIDE);


            if (d_msg_bool)
            {
                datamsg d_msg(d_msg_person, d_msg_sec, d_msg_ecgno);

                new_chan.cwrite(&d_msg, sizeof(d_msg));
                double dp = *(double*) new_chan.cread();

                cout << "---------------------------------------------------------------" << endl;
                cout << "DATA_MSG: retreived using '" << chan_name << "' channel" << endl;
                cout << "Person: " << d_msg_person << endl;
                cout << "Seconds: " << d_msg_sec << endl;
                cout << "Ecgno: " << d_msg_ecgno << endl;
                cout << "Data Value = " << dp << endl;
                cout << "---------------------------------------------------------------" << endl;
            }


            if (d_msg_file_bool)
            {
                // Initiliaze first data_msg request
                datamsg d_msg(d_msg_person, 0.0, 1);
                string out_file = "received/x" + to_string(d_msg_person) + ".csv"; 

                // Open connection for output file
                ofstream myfile;
                myfile.open(out_file);

                gettimeofday(&start, NULL);

                for (double t = 0; t <= 59.996; t += 0.004) {
                    d_msg.seconds = t;

                    // Get data point for ecg1
                    d_msg.ecgno = 1;
                    new_chan.cwrite(&d_msg, sizeof(d_msg));
                    double d_point1 = *(double*) new_chan.cread();

                    // Get data point for ecg2
                    d_msg.ecgno = 2;
                    new_chan.cwrite(&d_msg, sizeof(d_msg));
                    double d_point2 = *(double*) new_chan.cread();

                    // Write data to output csv file
                    myfile << t << "," << d_point1 << "," << d_point2 << "\n";
                }

                // Close connection for output file
                myfile.close();

                gettimeofday(&end, NULL);
                time_taken = (end.tv_sec - start.tv_sec) * 1e6;
                time_taken = (time_taken + (end.tv_sec - start.tv_sec)) * 1e-6;

                cout << "---------------------------------------------------------------" << endl;
                cout << "DATA_MSG: retreived using '" << chan_name << "' channel" << endl;
                cout << "Person: " << d_msg_person << endl;
                cout << "Input File: BIMDC/" << d_msg_person << ".csv" << endl;
                cout << "Output File: " << out_file << endl;
                cout << "Time Taken = " << time_taken << endl;
                cout << "---------------------------------------------------------------" << endl;
            }


            // If FILE_MSG request was provided
            if (f_msg_filename)
            { 
                char* file_data;

                char* buff = createFileMsgBuff(0, 0, f_msg_filename);
                const int size = sizeof(filemsg) + strlen(f_msg_filename) + 1;

                int w = new_chan.cwrite(buff, size);
                __int64_t file_length = *(__int64_t*) new_chan.cread();

                // Open output file stream
                string in_file = f_msg_filename;
                string out_file = "received/y" + in_file;
                ofstream wf(out_file, ios::binary);

                // if output file stream cannot be opened
                //   -> close all channels
                //   -> throw error
                if (!wf) {
                    cout << "Cannot Open File!" << endl;
                    MESSAGE_TYPE m = QUIT_MSG;
                    new_chan.cwrite(&m, sizeof(MESSAGE_TYPE));
                    chan.cwrite(&m, sizeof(MESSAGE_TYPE));
                    wait();

                    throw invalid_argument("FileMsg Error: Output file could not be opened");
                }

                int offset = 0;

                gettimeofday(&start, NULL);
                while (true)
                {
                    if (file_length - offset >= MAX_MESSAGE)
                    {
                        buff = createFileMsgBuff(offset, MAX_MESSAGE, f_msg_filename);
                        w = new_chan.cwrite(buff, size);
                        file_data = new_chan.cread();
                        wf.write(file_data, MAX_MESSAGE);
                    }
                    else
                    {
                        int final_chunk = file_length - offset;

                        buff = createFileMsgBuff(offset, final_chunk, f_msg_filename);
                        w = new_chan.cwrite(buff, size);
                        file_data = new_chan.cread();
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

                // ...........
                cout << "---------------------------------------------------------------" << endl;
                cout << "FILE_MSG: retreived using '" << chan_name << "' channel" << endl;
                cout << "Input File: BIMDC/" << in_file << endl;
                cout << "Output File: " << out_file << endl;
                cout << "Time Taken = " << time_taken << endl;
                cout << "---------------------------------------------------------------" << endl;
            }


            // Close the channels
            MESSAGE_TYPE m_quit = QUIT_MSG;
            new_chan.cwrite(&m_quit, sizeof(MESSAGE_TYPE));
            chan.cwrite(&m_quit, sizeof(MESSAGE_TYPE));
            wait();

        }
        else
        {
            if (d_msg_bool) {
                // initialize datamsg object
                datamsg d_msg(d_msg_person, d_msg_sec, d_msg_ecgno);

                // Write d_msg to the dataserver and Read the data point value from the dataserver
                chan.cwrite(&d_msg, sizeof(d_msg));
                double dp = *(double*) chan.cread();

                // Print the data point value
                cout << "---------------------------------------------------------------" << endl;
                cout << "DATA_MSG: retreived using 'control' channel" << endl;
                cout << "Person: " << d_msg_person << endl;
                cout << "Seconds: " << d_msg_sec << endl;
                cout << "Ecgno: " << d_msg_ecgno << endl;
                cout << "Data Value = " << dp << endl;
                cout << "---------------------------------------------------------------" << endl;
            }


            if (d_msg_file_bool)
            {
                // Initiliaze first data_msg request
                datamsg d_msg(d_msg_person, 0.0, 1);
                string out_file = "received/x" + to_string(d_msg_person) + ".csv"; 

                // Open connection for output file
                ofstream myfile;
                myfile.open(out_file);

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
                    myfile << t << "," << d_point1 << "," << d_point2 << "\n";
                }

                // Close connection for output file
                myfile.close();

                gettimeofday(&end, NULL);
                time_taken = (end.tv_sec - start.tv_sec) * 1e6;
                time_taken = (time_taken + (end.tv_sec - start.tv_sec)) * 1e-6;

                cout << "---------------------------------------------------------------" << endl;
                cout << "DATA_MSG: retreived using 'control' channel" << endl;
                cout << "Person: " << d_msg_person << endl;
                cout << "Input File: BIMDC/" << d_msg_person << ".csv" << endl;
                cout << "Output File: " << out_file << endl;
                cout << "Time Taken = " << time_taken << endl;
                cout << "---------------------------------------------------------------" << endl;
            }


            // If FILE_MSG request was provided
            if (f_msg_filename)
            { 
                char* file_data;

                char* buff = createFileMsgBuff(0, 0, f_msg_filename);
                const int size = sizeof(filemsg) + strlen(f_msg_filename) + 1;

                int w = chan.cwrite(buff, size);
                __int64_t file_length = *(__int64_t*) chan.cread();

                // Open output file stream
                string in_file = f_msg_filename;
                string out_file = "received/y" + in_file;
                ofstream wf(out_file, ios::binary);

                // if output file stream cannot be opened
                //   -> close all channels
                //   -> throw error
                if (!wf) {
                    cout << "Cannot Open File!" << endl;
                    MESSAGE_TYPE m = QUIT_MSG;
                    chan.cwrite(&m, sizeof(MESSAGE_TYPE));
                    wait();

                    throw invalid_argument("FileMsg Error: Output file could not be opened");
                }

                int offset = 0;

                gettimeofday(&start, NULL);
                while (true)
                {
                    if (file_length - offset >= MAX_MESSAGE)
                    {
                        buff = createFileMsgBuff(offset, MAX_MESSAGE, f_msg_filename);
                        w = chan.cwrite(buff, size);
                        file_data = chan.cread();
                        wf.write(file_data, MAX_MESSAGE);
                    }
                    else
                    {
                        int final_chunk = file_length - offset;

                        buff = createFileMsgBuff(offset, final_chunk, f_msg_filename);
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

                // ...........
                cout << "---------------------------------------------------------------" << endl;
                cout << "FILE_MSG: retreived using 'control' channel" << endl;
                cout << "Input File: BIMDC/" << in_file << endl;
                cout << "Output File: " << out_file << endl;
                cout << "Time Taken = " << time_taken << endl;
                cout << "---------------------------------------------------------------" << endl;
            }


            // Close the channel
            MESSAGE_TYPE m_quit = QUIT_MSG;
            chan.cwrite(&m_quit, sizeof(MESSAGE_TYPE));
            wait();
        }






/*
        // --------------- Task 1 ---------------

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

/*

        // --------------- Task 2 --------------- 


        char* filename;
        char* buff;
        char* file_data;
        int w, offset;
        __int64_t file_length;

        filename = "2.csv";
        buff = createFileMsgBuff(0, 0, filename);
        const int size = sizeof(filemsg) + strlen(filename) + 1;

        w = chan.cwrite(buff, size);
        file_length = *(__int64_t*) chan.cread();

        offset = 0;

        ofstream wf("received/y2.csv", ios::binary);
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


        // -------------------------------------------------------------- 

        filename = "file.txt";
        buff = createFileMsgBuff(0, 0, filename);
        const int size2 = sizeof(filemsg) + strlen(filename) + 1;

        w = chan.cwrite(buff, size2);
        file_length = *(__int64_t*) chan.cread();

        offset = 0;

        ofstream wf2("received/y_file.txt", ios::binary);
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


        // --------------- Task 3 --------------- 


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







    */
        
    }


    


    // closing the channel    
/*
    MESSAGE_TYPE m = QUIT_MSG;
    new_chan.cwrite (&m, sizeof(MESSAGE_TYPE));
    chan.cwrite (&m, sizeof(MESSAGE_TYPE));
*/
   
   return 0;
}
