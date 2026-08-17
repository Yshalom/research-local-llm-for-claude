#include <windows.h>
#include <cstring>
#include <iostream>

#include "settings.h"
#include "command_processor.h"

constexpr char JSON_SETTINGS_PATH[] = "proxy-settings.json";

int launch_command(char* command)
{
    // --- LOGIC TO PREVENT ORPHANED PROCESSES ---
    // Orphaned process won't terminate when the father do, therefore we avoid it.

    // Create a Job Object
    HANDLE hJob = CreateJobObject(NULL, NULL);
    if (NULL == hJob) {
        fprintf(stderr, "Error creating Job Object\n");
        delete[] command;
        return 1;
    }

    // Set the "Kill on Job Close" limit
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limit = { 0 };
    limit.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;;
    if (!SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &limit, sizeof(limit))) {
        fprintf(stderr, "Error setting Job Object info\n");
        CloseHandle(hJob);
        delete[] command;
        return 1;
    }

    // Prepare Process Startup info
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    // Start the process in SUSPENDED state (so we can assign it to the job before it starts running)
    // We use CreateProcess instead of system() to have direct control
    if (!CreateProcessA(
        NULL,           // Application name (not used when using command line)
        command,        // Command line
        NULL,           // Process handle
        NULL,           // Thread handle
        FALSE,          // Inherit handles
        CREATE_SUSPENDED, // Start suspended so we can attach to Job
        NULL,           // Environment
        NULL,           // Current directory
        &si,            // Startup info
        &pi             // Process info
    )) {
        fprintf(stderr, "Failed to launch process. Error: %lu\n", GetLastError());
        CloseHandle(hJob);
        delete[] command;
        return 1;
    }

    // Assign the process to the Job Object
    if (!AssignProcessToJobObject(hJob, pi.hProcess)) {
        fprintf(stderr, "Failed to assign process to job. Error: %lu\n", GetLastError());
        TerminateProcess(pi.hProcess, 0);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(hJob);
        delete[] command;
        return 1;
    }

    // Resume the process now that it's inside the Job
    ResumeThread(pi.hThread);

    // Wait for the child process to exit
    // This mimics the behavior of system()
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Cleanup
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hJob); // When this handle closes, all processes in the job are killed

    return 0;
}

int main(int argc, char** argv)
{
    Settings settings(JSON_SETTINGS_PATH);
    CommandProcessor command_processor(argc, argv, settings);

    launch_command(const_cast<char*>(command_processor.get_command().c_str()));

    return 0;
}
