#pragma warning(disable : 4996)

#include <iostream>
#include <random>
#include "Data.h"
#include <list>
#include <algorithm>
#include "Item.h"
#include <fstream>
#include <thread>
#include <mutex>
#include "Windows.h"
#include "conio.h"

using namespace std;

#define N_BYTES_TO_READ 1024UL
#define N_BYTES_TO_WRITE 1024UL
#define TIMEOUT 60*1000

bool getCommand(int com, atomic<int>* command)
{
    if (command->load() == com)
    {
        *command = -1;
        return true;
    }
    return false;
}

int GetMonthIndex(string m)
{
    const char MonthNames[12][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    for(int i = 0; i < 12; i++)
    {
        if (m == MonthNames[i]) { return i+1; };
    }
    return 0;
}

class Reader
{
private:
    HANDLE hFileHandle, hExitEvent;
    mutex *mMx;
    Data *pData;
    condition_variable *pCv;
    atomic<bool>* pStop;
    atomic<bool>* pSent;
public:
    Reader(HANDLE& h1, HANDLE& h2, atomic<bool>* pC, atomic<bool>* pS, mutex* mMx, condition_variable *pCv, Data *pData) : hFileHandle(h1), hExitEvent(h2), pStop(pC), pSent(pS), mMx(mMx), pCv(pCv), pData(pData) {}

    void operator()()
    {
        unsigned long nReadBytes = 0; //Prepare for reading
        unsigned char* pBuffer = new unsigned char[N_BYTES_TO_READ];
        OVERLAPPED Overlapped;
        memset(&Overlapped, 0, sizeof Overlapped);
        Overlapped.hEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
        HANDLE hEvents[] = { Overlapped.hEvent, hExitEvent };
        bool NoData = true;
        int error = 0;

        while(pStop->load() != true)
        { 
            unique_lock<mutex> lock(*mMx);
            pCv->wait(lock, [&]()->bool { return pSent->load(); });
            if (!ReadFile(hFileHandle, pBuffer, N_BYTES_TO_READ, &nReadBytes, &Overlapped))
            {
                error = GetLastError();
                switch (error)
                {
                case ERROR_IO_PENDING:
                    switch (WaitForMultipleObjects(2, hEvents, FALSE, TIMEOUT))
                    { // waiting for response from COM1
                    case WAIT_OBJECT_0:
                        GetOverlappedResult(hFileHandle, &Overlapped, &nReadBytes, FALSE);
                        NoData = false; // Got some data, waiting ended
                        break;
                    case WAIT_OBJECT_0 + 1:
                        cout << "Reading broken off" << endl;
                        break; // to user has broken the ending off
                    case WAIT_TIMEOUT:
                        cout << "Timeout period " << TIMEOUT << "ms elapsed, nothing was received. Press a key to exit" << endl;
                        break; // timeout
                    default:
                        cout << "Reading failed, error " << GetLastError() << ". Press a key to exit" << endl;
                        break; // some system errors
                    }
                    break;
                default: // some system errors
                    cout << "Reading failed, error " << GetLastError() << ". Press a key to exit" << endl;
                    break;
                }
            } else { NoData = false; }

            if (!NoData)
            {
                cout << nReadBytes << " bytes were read" << endl;
                char group = pBuffer[0];
                int subGroup;
                string birdName;
                Date Timestamp;
                short int TimestampDay;
                short int TimestampMonth;
                short int TimestampYear;

                string temp = "";
                int item = 0;
                bool startedName = false;
                for(int i = 2; pBuffer[i] != '\0'; i++)
                {
                    if (pBuffer[i] == '>')
                        startedName = false;
                    if(startedName)
                    {
                        temp += pBuffer[i];
                    } else 
                    {
                        if (pBuffer[i] != ' ' && pBuffer[i] != '>')
                        {
                            if (pBuffer[i] != '<')
                            {
                                temp += pBuffer[i];
                            }
                            else
                            {
                                startedName = true;
                            }
                            if (pBuffer[i + 1] == '\0')
                            {
                                TimestampYear = stoi(temp); temp.clear();
                                Timestamp.SetDate(TimestampDay, TimestampMonth, TimestampYear);
                            }
                        }
                        else
                        {
                            if (pBuffer[i - 1] == '>') { continue; }
                            if (!startedName)
                            {
                                switch (item)
                                {
                                case 0: subGroup = stoi(temp); temp.clear(); item++; break;
                                case 1: birdName = temp; temp.clear(); item++; break;
                                case 2: TimestampDay = stoi(temp); temp.clear(); item++; break;
                                case 3: TimestampMonth = GetMonthIndex(temp); temp.clear(); item++; break;
                                }
                            }
                        }
                    }
                }
                //if(pStop->load() != true)
                if(nReadBytes != 0)
                    pData->InsertItem(group, subGroup, birdName, Timestamp);
                *pSent = false;
                pCv->notify_one();  
            }
        }
        CloseHandle(Overlapped.hEvent); // clean
        delete pBuffer;
        //cout << "I am dying Reader" << endl;
    }
};

class Writer
{
private:
    HANDLE hFileHandle, hExitEvent;
    mutex* mMx;
    condition_variable* pCv;
    atomic<bool>* pStop;
    atomic<bool>* pSent;
public:
    Writer(HANDLE& h1, HANDLE& h2, atomic<bool> * pC, atomic<bool> *pS, mutex *mMx, condition_variable* pCv) : hFileHandle(h1), hExitEvent(h2), pStop(pC), pSent(pS), mMx(mMx), pCv(pCv){}

    void operator()()
    {
        unsigned long nBytesToWrite = N_BYTES_TO_WRITE;
        unsigned long nWrittenBytes = 0; //Prepare for writing
        unsigned char* pBuffer = new unsigned char[N_BYTES_TO_WRITE];

        string txt = "ready";
        for (int i = 0; i < txt.size(); i++)
        {
            pBuffer[i] = txt.at(i);
        }
        pBuffer[txt.size()] = '\0';

        OVERLAPPED Overlapped;
        memset(&Overlapped, 0, sizeof Overlapped);
        Overlapped.hEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
        HANDLE hEvents[] = { Overlapped.hEvent, hExitEvent };
        bool WasWritten = false;

        while(pStop->load() != true)
        { 
            unique_lock<mutex> lock(*mMx);
            if (!WriteFile(hFileHandle, pBuffer, txt.size()+1, &nWrittenBytes, NULL))
            {
                cout << "Data was not written, error" << GetLastError() << endl;
                *pStop = true;
                *pSent = true;
                pCv->notify_one();
            }
            else if (nBytesToWrite != nWrittenBytes)
                cout << "Only " << nWrittenBytes << " bytes instead of " << nBytesToWrite << " were written" << endl;
            else
                cout << nWrittenBytes << " bytes were written" << endl;
            WasWritten = nWrittenBytes == txt.size() + 1;
            if (WasWritten)
            { 
                *pSent = true;
                pCv->notify_one();
                pCv->wait(lock);
            }
            else
                continue;
        }
        WriteFile(hFileHandle, "stop", txt.size() + 1, &nWrittenBytes, NULL);
        CloseHandle(Overlapped.hEvent); // clean
        delete pBuffer;
        //cout << "I am dying Writer" << endl;
    }
};

class Controller
{
private:
    atomic<int> *input;
    atomic<bool>* pStop;
    bool &connected;
    string temp;
public:
    Controller(atomic<int>* command, atomic<bool> *pC, bool &con) : input(command), pStop(pC), connected(con) { }
    void operator() ()
    {
        while (true)
        {
            temp.clear();
            getline(cin, temp);
            if (temp == "exit")
            {
                if(connected){
                    cout << "Cannot exit while connection is established. Type 'stop' if you want to stop connection process" << endl;
                    continue;
                }
                *input = 0;
                break;
            }
            else if (temp == "connect")
                if (!connected)
                    *input = 1;
                else
                    cout << "Connection has already been established" << endl;
            else if (temp == "stop")
            {
                if(connected)
                { 
                    *input = 2;
                    cout << "Stopping" << endl;
                    *pStop = true;
                }
                else
                    cout << "Connection is not established yet" << endl;
            }
            else if (temp == "hi")
                *input = 3;
            else
            { 
                cout << "command not recognized" << endl;
                *input = -1;
            }
        }
    }
};

int main()
{
    /*
    Data myData(300);
    myData.PrintAll();
    cout << "There are " << myData.CountItems() << " items" << endl;
    myData.InsertItem('Z', 15, "Item1", Date(5, 1, 2021));
    myData.InsertItem('Z', 15, "Item2", Date(3, 1, 2021));
    myData.InsertItem('Z', 15, "Item3", Date(2, 1, 2021));
    myData.InsertItem('Z', 55, "Item1", Date(1, 1, 2021));
    myData.InsertItem('Z', 55, "Item2", Date(2, 1, 2021));
    myData.InsertItem('Z', 70, "Only Item1", Date(2, 1, 2021));
    cout << endl << "==================================================" << endl;
    cout << "PRINTING GROUP" << endl;
    cout << "==================================================" << endl << endl;
    
    myData.PrintGroup('Z');
    cout << "There are " << myData.CountGroupItems('Z') << " items in group Z" << endl;
    
    cout << endl << "==================================================" << endl;
    cout << "PRINTING SUBGROUP ITEMS" << endl;
    cout << "==================================================" << endl << endl;
    
    cout << "Printing Subgroup 15 of Group Z by Names" << endl;
    myData.PrintSubgroupByNames('Z', 15);
    cout << "Printing Subgroup 15 of Group Z by Dates" << endl;
    myData.PrintSubgroupByDates('Z', 15);
    cout << "There are " << myData.CountSubgroupItems('Z', 15) << " items in subgroup 15 of Group Z" << endl;

    cout << endl << "==================================================" << endl;
    cout << "PRINTING SUBGROUP ONE ITEM" << endl;
    cout << "==================================================" << endl << endl;
    
    cout << "Printing Subgroup 70 of Group Z by Names" << endl;
    myData.PrintSubgroupByNames('Z', 70);
    cout << "Printing Subgroup 70 of Group Z by Dates" << endl;
    myData.PrintSubgroupByDates('Z', 70);
    cout << "There is only " << myData.CountSubgroupItems('Z', 70) << " item in subgroup 70 of Group Z" << endl;
    
    cout << endl << "==================================================" << endl;
    cout << "PRINTING NON-EXISTING GROUPS" << endl;
    cout << "==================================================" << endl << endl;

    cout << "Printing Nonexisting group by Names" << endl;
    myData.PrintSubgroupByNames('Z', 99);
    cout << "Printing Nonexisting group by Dates" << endl;
    myData.PrintSubgroupByDates('Z', 99);
    cout << "There are " << myData.CountSubgroupItems('Z', 15) << " items in nonexisting group" << endl;

    cout << endl << "==================================================" << endl;
    cout << "PRINT EXISTING AND NON-EXISTING ITEM" << endl;
    cout << "==================================================" << endl << endl;

    myData.PrintItem('Z', 55, "Item1");
    myData.PrintItem('Z', 70, "Item1");

    cout << endl << "==================================================" << endl;
    cout << "PRINT GROUP FOR NON-EXISTING GROUP" << endl;
    cout << "==================================================" << endl << endl;

    Data secondData(30);
    secondData.PrintAll();
    secondData.InsertItem('Z', 10, "ItemZ", Date(5, 1, 2021));
    secondData.PrintGroup('Y');
    cout << "Nonexisting group has " << secondData.CountGroupItems('Y') << " items" << endl;

    cout << endl << "==================================================" << endl;
    cout << "INSERTING ITEMS" << endl;
    cout << "==================================================" << endl << endl;

    secondData.InsertItem('Z', 10, "ItemZNEW", Date(10, 1, 2021));
    secondData.InsertItem('Z', 11, "ItemZ", Date(5, 1, 2021));
    secondData.InsertItem('X', 10, "ItemZ", Date(5, 1, 2021));
    secondData.InsertItem('Z', 10, "ItemZNEW", Date(10, 1, 2021));
    secondData.PrintAll();
    
    cout << endl << "==================================================" << endl;
    cout << "INSERTING SUBGROUPS" << endl;
    cout << "==================================================" << endl << endl;

    initializer_list<Item*> ilItems = { new Item('X', 15, "ItemX", Date(10, 10, 2021)) };
    initializer_list<Item*> ilItems2 = { new Item('T', 40, "ItemT", Date(10, 10, 2021)) };
    initializer_list<Item*> ilItems3 = { new Item('Z', 10, "ItemZNEW", Date(10, 10, 2021)) };
    initializer_list<Item*> ilItems4 = { new Item('E', 10, "ItemE1", Date(10, 10, 2021)), new Item('E', 11, "ItemE2", Date(12, 10, 2021)) };
    initializer_list<initializer_list<Item*>> ilList = { ilItems4 };
    initializer_list<int> ilSubgroups = { 10, 11 };

    secondData.InsertSubgroup('X', 15, ilItems);
    secondData.InsertSubgroup('T', 40, ilItems2);
    secondData.InsertSubgroup('Z', 10, ilItems3);
    secondData.PrintAll();

    cout << endl << "==================================================" << endl;
    cout << "INSERTING GROUPS" << endl;
    cout << "==================================================" << endl << endl;

    secondData.InsertGroup('E', ilSubgroups, ilList);
    secondData.InsertGroup('Z', ilSubgroups, ilList);
    secondData.PrintAll();

    Data thirdData(5);
    thirdData.InsertItem('Z', 15, "Item1", Date(5, 1, 2021));
    thirdData.InsertItem('Z', 15, "Item2", Date(3, 1, 2021));
    thirdData.InsertItem('Z', 15, "Item3", Date(2, 1, 2021));
    thirdData.InsertItem('Z', 55, "Item1", Date(1, 1, 2021));
    thirdData.InsertItem('Z', 55, "Item2", Date(2, 1, 2021));
    thirdData.InsertItem('Z', 99, "Item1", Date(20, 1, 2021));
    thirdData.InsertItem('X', 90, "ItemX", Date(20, 1, 2021));
    thirdData.InsertItem('Y', 90, "ItemY", Date(20, 1, 2021));
    thirdData.PrintAll();
    
    cout << endl << "==================================================" << endl;
    cout << "REMOVING ITEMS" << endl;
    cout << "==================================================" << endl << endl;

    cout << boolalpha << thirdData.RemoveItem('Z', 15, "Item1") << endl;
    cout << boolalpha << thirdData.RemoveItem('Z', 99, "Item1") << endl;
    cout << boolalpha << thirdData.RemoveItem('X', 90, "ItemX") << endl;
    cout << boolalpha << thirdData.RemoveItem('X', 90, "ItemX") << endl;

    thirdData.PrintAll();

    cout << endl << "==================================================" << endl;
    cout << "REMOVING SUBGROUPS" << endl;
    cout << "==================================================" << endl << endl;

    cout << boolalpha << thirdData.RemoveSubgroup('Z', 15) << endl;
    cout << boolalpha << thirdData.RemoveSubgroup('Y', 90) << endl;
    cout << boolalpha << thirdData.RemoveSubgroup('H', 15) << endl;

    thirdData.PrintAll();

    cout << endl << "==================================================" << endl;
    cout << "REMOVING GROUPS" << endl;
    cout << "==================================================" << endl << endl;

    cout << boolalpha << thirdData.RemoveGroup('Z') << endl;
    cout << boolalpha << thirdData.RemoveGroup('O') << endl;
    thirdData.PrintAll();
    
    */
   
    atomic<int> command = -1;
    atomic<bool> stopConnection = false;
    atomic<bool> sentMessage = false;
    bool connected = false;
    Data* pData = new Data();
    mutex mx;
    condition_variable cv;

    cout << "Program is ready for connection with ICS0025 Pipe server, type exit to end session" << endl;

    thread ControllerThread{ Controller(&command, &stopConnection, connected) };
    
    while(true)
    { 
        if(getCommand(1, &command))
        {
            cout << "Connecting" << endl;
            HANDLE hFile = CreateFileA("\\\\.\\pipe\\ICS0025",
                GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                FILE_FLAG_OVERLAPPED, NULL);

            if (hFile == INVALID_HANDLE_VALUE)
            {
                cout << "File was not created. Error code: " << GetLastError() << endl;
                continue;
            }
            cout << "Connected succesfully! To stop connection type 'stop'" << endl;
            HANDLE hExitEvent = CreateEventA(NULL, TRUE, FALSE, NULL);

            thread WriterThread(Writer(hFile, hExitEvent, &stopConnection, &sentMessage, &mx, &cv));
            thread ReaderThread(Reader(hFile, hExitEvent, &stopConnection, &sentMessage, &mx, &cv, pData));
            connected = true;
            WriterThread.join();
            ReaderThread.join();
            CloseHandle(hExitEvent);
            CloseHandle(hFile);
            stopConnection = false;
            connected = false;
            cout << "Succesfully stopped" << endl << "To view the collected data type 'exit'" << endl;
        } else if(getCommand(0, &command))
        {
            cout << "Exiting" << endl;
            if (pData->CountItems() != 0)
            {
                cout << "Here are the " << pData->CountItems() << " items I have collected so far" << endl;
                pData->PrintAll();
            }
            break;
        } else if(getCommand(3, &command))
        {
            cout << "Hello my Friend :D" << endl;
        }
    }
    ControllerThread.join();
    delete pData;
    
    return 0;
}
