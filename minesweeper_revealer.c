#include <stdio.h>
#include <stdlib.h>
#include <memoryapi.h>
#include <processthreadsapi.h>
#include <errhandlingapi.h>
#include <handleapi.h>
#include <tlhelp32.h>

/********************************************************
 **************** Minesweeper Revealer ******************
 ********************************************************
 * The program identifies a Minesweeper.exe (Windows XP)
 * process and prints a table that reveals where the
 * mines are by accessing process memory.
 * The program is guaranteed to work for up to 13 rows,
 * above that it might be bugged.
 ********************************************************
 */

// Function Prototype
DWORD FindProcessId(const char *processname);

int main( int argc, char *argv[] ) {

    // Constants definition
    const DWORD ARRAY_FIELD_ADDRESS = 0x01005361;
    const DWORD HEIGHT_ADDRESS      = 0x010056a8;
    const DWORD WIDTH_ADDRESS       = 0x010056ac;
    const int   BUFFER_SIZE         = 4096;
    const char* PROCESS_NAME        = "Winmine__XP.exe";

    // Variable definition
    unsigned char *arrayBuffer = (unsigned char *)malloc(sizeof(unsigned char) * BUFFER_SIZE);
    unsigned char *p;
    unsigned char  value;
    HANDLE         hProcess;
    DWORD          dwProcessId;
    int           *nHeight;
    int           *nWidth;
    int            nRow;
    int            nCol;

    // Code Section

    // Open Winmine__XP.exe process
    dwProcessId = FindProcessId(PROCESS_NAME);
    hProcess    = OpenProcess( PROCESS_VM_READ, FALSE, dwProcessId);

    // Check if handle is valid
    if (hProcess == NULL)
    {
        printf("Error with process handler\n");
        printf("Error: %x\n", GetLastError());
    }
    else
    {

        // Read Height, Width and FieldArray from the process
        ReadProcessMemory(hProcess, HEIGHT_ADDRESS, &nHeight, 1, NULL);
        ReadProcessMemory(hProcess, WIDTH_ADDRESS, &nWidth, 1, NULL);
        ReadProcessMemory(hProcess, ARRAY_FIELD_ADDRESS, &arrayBuffer, BUFFER_SIZE, NULL);
        nHeight = (int)nHeight & 0xff;
        nWidth  = (int)nWidth  & 0xff;

        if (arrayBuffer == NULL)
        {
            printf("Error while reading\n");

        }
        else
        {
            printf("\n");
            for(int i = 0; i < nWidth; i++)
            {
                printf(" _");
            }
            printf("\n");

            p = (unsigned char*)&arrayBuffer;

            // Go over all rows of the field
            for(nRow = 0; nRow < nHeight; nRow++)
            {
                printf("|");

                // Go over each cell in the row
                for(nCol = 0; nCol < nWidth; nCol++)
                {

                    value = p[32*nRow + nCol];


                    if (value == 0x8f)                      // Cell contains Mine
                    {
                        printf("X|");
                    }
                    else if(value == 0x40)                  // Cell is empty)
                    {
                        printf(".|");

                    }
                    else if(value >= 0x41 && value <= 0x48) // Cell contains a number
                    {
                        value = value - 0x41 + '1';
                        printf("%c|", value);
                    }
                    else
                    {
                        printf("_|");
                    }
                }

                printf("\n");
            }
        }

        getchar();
        CloseHandle(hProcess);
        return 0;
    }

}


/*
 * The function gets a Process Name and finds its corresponding ID
 * Input:  Name of process
 * Output: ID of the process
 */
DWORD FindProcessId(const char *processname)
{
    // Local Variables
    PROCESSENTRY32 pe32;
    HANDLE         hProcessSnap;
    DWORD          result = NULL;

    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == hProcessSnap)
        return(FALSE);

    pe32.dwSize = sizeof(PROCESSENTRY32); // <----- IMPORTANT

    // Retrieve information about the first process,
    // and exit if unsuccessful
    if (!Process32First(hProcessSnap, &pe32))
    {
        CloseHandle(hProcessSnap);          // Clean the snapshot object
        printf("Failed to gather information on system processes! \n");
        return(NULL);
    }

    do
    {
        if (0 == strcmp(processname, pe32.szExeFile))
        {
            result = pe32.th32ProcessID;
            break;
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);

    return result;
}
