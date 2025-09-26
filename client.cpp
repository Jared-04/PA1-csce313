
/*
	Original author of the starter code
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 2/8/20
	
	Please include your Name, UIN, and the date below
	Name: Jared White
	UIN: 134007415
	Date: 9/16/2025
*/
#include "common.h"
#include "FIFORequestChannel.h"
#include <unistd.h>

using namespace std;

int main (int argc, char *argv[]) {
    int opt;
    int p = -1;
    double t = -1;
    int e = -1;
    int m = MAX_MESSAGE;
    bool c = false;

    string filename = "";
    while ((opt = getopt(argc, argv, "p:t:e:f:m:c")) != -1) {
        switch (opt) {
            case 'p':
                p = atoi(optarg);
                break;
            case 't':
                t = atof(optarg);
                break;
            case 'e':
                e = atoi(optarg);
                break;
            case 'f':
                filename = optarg;
                break;
            case 'm':
                m = atoi(optarg);
                break;
            case 'c':
                c = true;
                break;
        }
    }

    // create child, execute server
    pid_t pid1 = fork();

    if (pid1 == 0) {
        char* args[] = {(char*) "./server", nullptr};
        execvp(args[0], args);
    } else if (pid1 < 0) {
        cout << "Inside error" << endl;
        EXITONERROR("Fork error.");
    } else {
        FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);
        char buf[MAX_MESSAGE]; // 256
        FIFORequestChannel* chan2 = nullptr;

        if (c) {
            MESSAGE_TYPE newchan = NEWCHANNEL_MSG;
            chan.cwrite(&newchan, sizeof(MESSAGE_TYPE));
            char channelName[30];
            chan.cread(&channelName, sizeof(channelName));

            chan2 = new FIFORequestChannel(channelName, FIFORequestChannel::CLIENT_SIDE);
            chan = (*chan2);
        }

        if (t != -1 || e != -1) {
            // example data point request
            datamsg x(p, t, e);

            memcpy(buf, &x, sizeof(datamsg));
            chan.cwrite(buf, sizeof(datamsg)); // question
            double reply;
            chan.cread(&reply, sizeof(double)); // answer
            cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
        } else if (p != -1) {
            ofstream out("received/x1.csv");

            for (int i = 0; i < 1000; i++) {
                double current_time = i * 0.004;
                datamsg x1(p, current_time, 1);
                memcpy(buf, &x1, sizeof(datamsg));
                chan.cwrite(buf, sizeof(datamsg));
                double ecg1;
                chan.cread(&ecg1, sizeof(double));

                datamsg x2(p, current_time, 2);
                memcpy(buf, &x2, sizeof(datamsg));
                chan.cwrite(buf, sizeof(datamsg));
                double ecg2;
                chan.cread(&ecg2, sizeof(double));

                out << current_time << "," << ecg1 << "," << ecg2 << endl;
            }
            out.close();
        } else if (filename != "") {
            ofstream out("received/" + filename);
            filemsg fm(0, 0);

            if (access(filename.c_str(), F_OK) == 0) {
                system(("cp " + filename + " BIMDC/" + filename).c_str());
            }
            
            string fname = filename;

            int len = sizeof(filemsg) + (fname.size() + 1);
            char* buf2 = new char[len];
            memcpy(buf2, &fm, sizeof(filemsg));
            strcpy(buf2 + sizeof(filemsg), fname.c_str());
            chan.cwrite(buf2, len);  // I want the file length;

            char* buf3 = new char[m];
            int64_t filesize;
            chan.cread(&filesize, sizeof(int64_t));

            int64_t remainder = filesize;
            int i = 0;
            while (remainder > 0) {
                filemsg* file_req = (filemsg*)buf2; // 
                file_req->offset = i * m;
                int read_size = (remainder < m) ? remainder : m;
                file_req->length = read_size;
                chan.cwrite(buf2, len);
                int bytes_read = chan.cread(buf3, read_size);
                out.write(buf3, bytes_read);

                i++;
                remainder -= bytes_read;
            }

            out.close();
			delete[] buf2;
			delete[] buf3;
        }
		//closing the channel
		MESSAGE_TYPE m = QUIT_MSG;
		chan.cwrite(&m, sizeof(MESSAGE_TYPE));
		if (c) {delete chan2;}
    }
}