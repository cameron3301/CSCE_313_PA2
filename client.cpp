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


// Creates a character array `buff` that contains a file message
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

    // get input arguments from the user in the command line
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


    // Create a child process to run the dataserver on
    if (!fork())
    {
        char* args[] = {"./dataserver", NULL};
        execvp(args[0], args);
    }
    // proceed with the parent process
    else
    {
        int n = 100;    // default number of requests per "patient"
        int p = 15;		// number of patients
        srand(time_t(NULL));
    
        FIFORequestChannel chan ("control", FIFORequestChannel::CLIENT_SIDE);
        struct timeval start, end;
        double time_taken;



        // if a new channel was requested, create the new channel and proceed using it
        if (new_chan_msg_bool)
        {
            // Create a new channel
            MESSAGE_TYPE msg = NEWCHANNEL_MSG;
            chan.cwrite(&msg, sizeof(MESSAGE_TYPE));
            string chan_name = chan.cread();
            FIFORequestChannel new_chan (chan_name, FIFORequestChannel::CLIENT_SIDE);


            // if a single data point was requested, execute the content of this conditional statement
            if (d_msg_bool)
            {
                // initialize the datamsg object
                datamsg d_msg(d_msg_person, d_msg_sec, d_msg_ecgno);

                // write the datamsg to the dataserver -> read the data value returned by the dataserver
                new_chan.cwrite(&d_msg, sizeof(d_msg));
                double dp = *(double*) new_chan.cread();

                // Print the data point value
                cout << "---------------------------------------------------------------" << endl;
                cout << "DATA_MSG: retreived using '" << chan_name << "' channel" << endl;
                cout << "Person: " << d_msg_person << endl;
                cout << "Seconds: " << d_msg_sec << endl;
                cout << "Ecgno: " << d_msg_ecgno << endl;
                cout << "Data Value = " << dp << endl;
                cout << "---------------------------------------------------------------" << endl;
            }


            // if a file's worth of data points were requested, execute the content of this conditional statement
            if (d_msg_file_bool)
            {
                // Initiliaze first data_msg request
                datamsg d_msg(d_msg_person, 0.0, 1);
                string out_file = "received/x" + to_string(d_msg_person) + ".csv"; 

                // Open connection for output file
                ofstream myfile;
                myfile.open(out_file);

                // start measuring the execution time
                gettimeofday(&start, NULL);

                // begin iterating through each column of the input CSV file
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

                // calculate total execution time
                gettimeofday(&end, NULL);
                time_taken = (end.tv_sec - start.tv_sec) * 1e6;
                time_taken = (time_taken + (end.tv_sec - start.tv_sec)) * 1e-6;

                // print information regarding the operation performed above 
                cout << "---------------------------------------------------------------" << endl;
                cout << "DATA_MSG: retreived using '" << chan_name << "' channel" << endl;
                cout << "Person: " << d_msg_person << endl;
                cout << "Input File: BIMDC/" << d_msg_person << ".csv" << endl;
                cout << "Output File: " << out_file << endl;
                cout << "Time Taken = " << time_taken << endl;
                cout << "---------------------------------------------------------------" << endl;
            }


            // if a filemsg was requested, execute the content of this conditional statement
            if (f_msg_filename)
            {
                char* file_data;

                // Find the total length of the input file
                char* buff = createFileMsgBuff(0, 0, f_msg_filename);
                const int size = sizeof(filemsg) + strlen(f_msg_filename) + 1;
                int w = new_chan.cwrite(buff, size);
                __int64_t file_length = *(__int64_t*) new_chan.cread();

                // Open output file stream
                string in_file = f_msg_filename;
                string out_file = "received/y" + in_file;
                ofstream wf(out_file, ios::binary);

                // If output file stream cannot be opened
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

                // Start measuring the execution time
                gettimeofday(&start, NULL);

                // Begin iterating through the input file chunk-by-chunk
                int offset = 0;
                while (true)
                {
                    cout << file_length - offset << endl;
                    if (file_length - offset > MAX_MESSAGE)
                    {
                        // Retrieve a chunk of input file data from the dataserver -> write data to the output file
                        buff = createFileMsgBuff(offset, MAX_MESSAGE, f_msg_filename);
                        w = new_chan.cwrite(buff, size);
                        file_data = new_chan.cread();
                        wf.write(file_data, MAX_MESSAGE);
                    }
                    else
                    {
                        // Find the necessary size for the final chunk
                        int final_chunk = file_length - offset;

                        // Retrieve a chunk of input file data from the dataserver -> write data to the output file
                        buff = createFileMsgBuff(offset, final_chunk, f_msg_filename);
                        w = new_chan.cwrite(buff, size);
                        file_data = new_chan.cread();
                        wf.write(file_data, final_chunk);

                        break;
                    }
                    offset += MAX_MESSAGE;
                }

                // Close the output file ostream
                wf.close();

                // Calculate the time taken during the file transfer
                gettimeofday(&end, NULL);
                time_taken = (end.tv_sec - start.tv_sec) * 1e6;
                time_taken = (time_taken + (end.tv_sec - start.tv_sec)) * 1e-6;

                // print information regarding the operation performed above 
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
        // if a new channel was NOT requested, proceed using the 'control' channel
        else
        {
            // if a single data point was requested, execute the content of this conditional statement
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


            // if a series of data points were requested, execute the content of this conditional statement
            if (d_msg_file_bool)
            {
                // Initiliaze first data_msg request
                datamsg d_msg(d_msg_person, 0.0, 1);
                string out_file = "received/x" + to_string(d_msg_person) + ".csv"; 

                // Open connection for output file
                ofstream myfile;
                myfile.open(out_file);

                // start measuring the execution time
                gettimeofday(&start, NULL);

                // begin iterating through each column of the input CSV file
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

                // calculate the total execution time
                gettimeofday(&end, NULL);
                time_taken = (end.tv_sec - start.tv_sec) * 1e6;
                time_taken = (time_taken + (end.tv_sec - start.tv_sec)) * 1e-6;

                // print information regarding the operation performed above 
                cout << "---------------------------------------------------------------" << endl;
                cout << "DATA_MSG: retreived using 'control' channel" << endl;
                cout << "Person: " << d_msg_person << endl;
                cout << "Input File: BIMDC/" << d_msg_person << ".csv" << endl;
                cout << "Output File: " << out_file << endl;
                cout << "Time Taken = " << time_taken << endl;
                cout << "---------------------------------------------------------------" << endl;
            }


            // if a filemsg was requested, execute the content of this conditional statement
            if (f_msg_filename)
            { 
                char* file_data;

                // Find the total length of the input file
                char* buff = createFileMsgBuff(0, 0, f_msg_filename);
                const int size = sizeof(filemsg) + strlen(f_msg_filename) + 1;
                int w = chan.cwrite(buff, size);
                __int64_t file_length = *(__int64_t*) chan.cread();

                // Open output file stream
                string in_file = f_msg_filename;
                string out_file = "received/y" + in_file;
                ofstream wf(out_file, ios::binary);

                // If output file stream cannot be opened
                //   -> close all channels
                //   -> throw error
                if (!wf) {
                    cout << "Cannot Open File!" << endl;
                    MESSAGE_TYPE m = QUIT_MSG;
                    chan.cwrite(&m, sizeof(MESSAGE_TYPE));
                    wait();

                    throw invalid_argument("FileMsg Error: Output file could not be opened");
                }
                
                // Start measuring the execution time
                gettimeofday(&start, NULL);

                // Begin iterating through the input file chunk-by-chunk
                int offset = 0;
                while (true)
                {
                    if (file_length - offset > MAX_MESSAGE)
                    {
                        // Retrieve a chunk of input file data from the dataserver -> write data to the output file
                        buff = createFileMsgBuff(offset, MAX_MESSAGE, f_msg_filename);
                        w = chan.cwrite(buff, size);
                        file_data = chan.cread();
                        wf.write(file_data, MAX_MESSAGE);
                    }
                    else
                    {
                        // Find the necessary size for the final chunk
                        int final_chunk = file_length - offset;

                        // Retrieve a chunk of input file data from the dataserver -> write data to the output file
                        buff = createFileMsgBuff(offset, final_chunk, f_msg_filename);
                        w = chan.cwrite(buff, size);
                        file_data = chan.cread();
                        wf.write(file_data, final_chunk);

                        break;
                    }
                    offset += MAX_MESSAGE;
                }

                // Close the output file ostream
                wf.close();

                // Calculate the time taken during the file transfer
                gettimeofday(&end, NULL);
                time_taken = (end.tv_sec - start.tv_sec) * 1e6;
                time_taken = (time_taken + (end.tv_sec - start.tv_sec)) * 1e-6;

                // Print information regarding the operation performed above 
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
    }   
    return 0;
}
