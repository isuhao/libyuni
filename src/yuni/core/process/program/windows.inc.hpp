#pragma once
// file included by program.cpp

#ifdef YUNI_OS_WINDOWS

// http://msdn.microsoft.com/en-us/library/ms682499%28v=vs.85%29.aspx
// http://blogs.msdn.com/b/oldnewthing/archive/2011/12/16/10248328.aspx
// http://stackoverflow.com/questions/53208/how-do-i-automatically-destroy-child-processes-in-windows

# include "../../system/windows.hdr.h"
# include <WinNT.h>
# include "../../string/wstring.h"
# if _WIN32_WINNT >= 0x0600
#	include <processthreadsapi.h>
# endif
# include <iostream>
# include <io.h>


namespace Yuni
{
namespace Process
{

	namespace // anonymous
	{

		void closeHD(HANDLE& handle)
		{
			if (INVALID_HANDLE_VALUE != handle)
			{
				BOOL success = ::CloseHandle(handle);
				(void) success;
				assert(success != FALSE);
				handle = INVALID_HANDLE_VALUE;
			}
		}

		bool pipe(HANDLE (&fd)[2], SECURITY_ATTRIBUTES& attr, const char* const pipeName)
		{
			// Create a pipe for the child process's STDOUT.
			// CreatePipe(read, write, attr, null)
			if (FALSE == ::CreatePipe(fd, fd + 1, &attr, 0))
			{
				std::cerr << "pipe failed : failed to create pipe for " << pipeName << '\n';
				return false;
			}

			// Ensure the read handle to the pipe for STDOUT is not inherited.
			if (FALSE == ::SetHandleInformation(fd[0], HANDLE_FLAG_INHERIT, 0))
			{
				std::cerr << "pipe failed : failed to set handle information on " << pipeName << " pipe\n";
				return false;
			}

			return true;
		}


		#if _WIN32_WINNT >= 0x0600
		bool fillProcAttributeList(HANDLE (&handles)[3], LPPROC_THREAD_ATTRIBUTE_LIST& attrList)
		{
			SIZE_T size = 0;
			if (FALSE == ::InitializeProcThreadAttributeList(nullptr, 1, 0, &size))
				return false;
			if ((attrList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(
				::HeapAlloc(::GetProcessHeap(), 0, size))) != nullptr)
				return false;
			if (FALSE == ::InitializeProcThreadAttributeList(attrList, 1, 0, &size))
				return false;

			return (FALSE != ::UpdateProcThreadAttribute(attrList, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
				handles, 3 * sizeof(HANDLE), nullptr, nullptr));
		}
		#endif


	} // namespace anonymous




	bool Program::ThreadMonitor::spawnProcess()
	{
		SECURITY_ATTRIBUTES saAttr;

		// Set the bInheritHandle flag so pipe handles are inherited.
		saAttr.nLength = (uint32) sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = nullptr;

		if (not pipe(channels.outfd, saAttr, "stdout"))
			return false;

		if (not pipe(channels.infd, saAttr, "stdin"))
			return false;

		if (not pipe(channels.errd, saAttr, "stderr"))
			return false;


		#if _WIN32_WINNT >= 0x0600
		// Prepare inherited handle list for STARTUPINFOEX
		LPPROC_THREAD_ATTRIBUTE_LIST attrList = nullptr;
		HANDLE inheritedHandles[3] =
		{
			channels.outfd[0],
			channels.infd[1],
			channels.errd[1],
		};
		fillProcAttributeList(inheritedHandles, attrList);

		// Set up members of the STARTUPINFOEX structure.
		// This structure specifies the handles for redirection.
		// It is necessary to use the EX version, otherwise _all_ inheritable
		// handles in the program are inherited by the child.
		STARTUPINFOEX startInfo;
		::ZeroMemory(&startInfo, sizeof(STARTUPINFOEX));
		startInfo.StartupInfo.cb = (uint32) sizeof(STARTUPINFOEX);
		startInfo.lpAttributeList = attrList;
		startInfo.StartupInfo.dwFlags |= STARTF_USESTDHANDLES;
		startInfo.StartupInfo.hStdOutput = channels.infd[1];
		startInfo.StartupInfo.hStdInput = channels.outfd[0];
		startInfo.StartupInfo.hStdError = channels.errd[1];
		#else
		STARTUPINFO startInfo;
		::ZeroMemory(&startInfo, sizeof(STARTUPINFO));
		startInfo.cb = (uint32) sizeof(STARTUPINFO);
		startInfo.dwFlags |= STARTF_USESTDHANDLES;
		startInfo.hStdOutput = channels.infd[1];
		startInfo.hStdInput = channels.outfd[0];
		startInfo.hStdError = channels.errd[1];
		#endif // _WIN32_WINNT < 0x0600


		// Set up members of the PROCESS_INFORMATION structure.
		PROCESS_INFORMATION winProcInfo;
		::ZeroMemory(&winProcInfo, sizeof(PROCESS_INFORMATION));

		String cmd;
		{
			String tmpstr = procinfo.executable;
			tmpstr.replace("\"", "\\\"");
			cmd << '"' << tmpstr << '"';
			uint32_t count = static_cast<uint32_t>(procinfo.arguments.size());
			for (uint32_t i = 0; i != count; ++i) {
				tmpstr = procinfo.arguments[i];
				tmpstr.replace("\"", "\\\"");
				cmd << " \"" << tmpstr << '"';
			}
		}
		WString cmdLine(cmd);

		// Getting the start time of execution
		pStartTime = currentTime();

		// Create the child process.
		// ** FORK **
		bool success = (0 != ::CreateProcessW(nullptr,
			cmdLine.data(),  // command line
			nullptr,         // process security attributes
			nullptr,         // primary thread security attributes
			true,            // handles are inherited (necessary for STARTF_USESTDHANDLES)
			0,               // creation flags
			nullptr,         // use parent's environment
			nullptr,         // use parent's current directory
			&startInfo,      // STARTUPINFO pointer
			&winProcInfo));  // receives PROCESS_INFORMATION

		// Close streams passed to the child process
		closeHD(channels.outfd[0]);
		closeHD(channels.infd[1]);
		closeHD(channels.errd[1]);

		// If an error occurs, give up
		if (!success)
		{
			//DWORD error = ::GetLastError();
			//std::wcerr << L"Fork failed with:\n";
			//if (error)
			//{
			//	LPVOID lpMsgBuf = nullptr;
			//	DWORD bufLen = ::FormatMessageW(
			//		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			//		nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			//		(LPTSTR) &lpMsgBuf, 1024, nullptr);
			//	if (bufLen)
			//	{
			//		LPCTSTR lpMsgStr = (LPCTSTR)lpMsgBuf;
			//		std::wcerr << TEXT("  ") << lpMsgStr << std::endl;
			//		::HeapFree(::GetProcessHeap(), 0, lpMsgBuf);
			//		return false;
			//	}
			//}
			//std::wcerr << "  Unknown error !" << std::endl;
			return false;
		}

		processHandle = winProcInfo.hProcess;
		threadHandle = winProcInfo.hThread;
		procinfo.processID = (int)winProcInfo.dwProcessId;
		procinfo.processInput = _open_osfhandle(reinterpret_cast<intptr_t>(channels.infd[0]), 0);

		// timeout for the sub process
		procinfo.createThreadForTimeoutWL();

		return true;
	}


	void Program::ThreadMonitor::waitForSubProcess()
	{
		constexpr uint32_t bufferSize = 4096;
		constexpr uint32_t nbHandles = 3;
		// Wait for all these handles
		const HANDLE handleList[nbHandles] = { channels.infd[0], channels.errd[0], processHandle };

		bool finished = false;
		do
		{
			YUNI_STATIC_ASSERT(WAIT_OBJECT_0 == 0, waitObject0IsNotZero_CodeRequiresAttention);
			assert(nbHandles < MAXIMUM_WAIT_OBJECTS);

			DWORD waitStatus = ::WaitForMultipleObjectsEx(nbHandles, handleList, false, INFINITE, true);

			if (/*waitStatus >= WAIT_OBJECT_0 and*/ waitStatus < (WAIT_OBJECT_0 + (uint) nbHandles))
			{
				assert(waitStatus - WAIT_OBJECT_0 < nbHandles);

				HANDLE signalled = handleList[waitStatus - WAIT_OBJECT_0];
				DWORD exitCode = 0;

				// If the process was signalled
				if (signalled == processHandle)
				{
					pEndTime = currentTime();
					finished = true;

					if (0 != ::GetExitCodeProcess(processHandle, &exitCode))
					{
						// It is forbidden to use STILL_ACTIVE as an exit code
						// However, if it happens in real life, let it happen
						assert(exitCode != STILL_ACTIVE and "Child process uses STILL_ACTIVE as exit status !");
						pExitStatus = (int)exitCode;

						// TODO : How to know if the process was signalled or exited ?
						// pKilled = true;
					}
					else
						pExitStatus = STILL_ACTIVE - 1; // arbitrary
				}
				else // If the child wrote to stdout or stderr
				{
					DWORD readBytes = 0;
					char buffer[bufferSize];
					bool cout = (0 == (waitStatus - WAIT_OBJECT_0)); // otherwise, cerr
					HANDLE readFrom = cout ? channels.infd[0] : channels.errd[0];
					std::ostream& out = (cout) ? std::cout : std::cerr;
					//LPOVERLAPPED overlap = cout ? &outOverlap : &errOverlap;
					//DWORD totalAvailBytes = 0;

					//bool success = (FALSE != ::PeekNamedPipe(readFrom, nullptr, 0, nullptr, &totalAvailBytes, nullptr));
					//if (success and totalAvailBytes > 0)
					do
					{
						bool success = (FALSE != ::ReadFile(readFrom, buffer, sizeof(char) * 1, &readBytes, nullptr));
						if (not success or readBytes == 0)
							break;

						out.write(buffer, readBytes);
						out << std::flush;
					}
					while (true);
				}
			}
			else // Abnormal completion
			{
				switch (waitStatus)
				{
					// Async I/O event
					case WAIT_IO_COMPLETION:
						// TODO Manage I/O for IPC
						break;
					case WAIT_FAILED: // if WaitFSO failed, give up looping
						break;
					case WAIT_TIMEOUT: // Normally not possible with INFINITE
						assert(false and "Program::ThreadMonitor::waitForSubProcess : Timeout on infinite !");
						// Keep looping
						break;
					default: // WAIT_ABANDONED_X is only for mutexes, this should never occur
						assert(false and "Program::ThreadMonitor::waitForSubProcess : Unmanaged WaitForSingleObject return code !");
						break;
				}

				if (WAIT_IO_COMPLETION != waitStatus)
				{
					pEndTime = currentTime();
					finished = true;
				}
			}
		}
		while (not finished);

		// end of the chrono
		assert(pEndTime != 0 and "pEndTime is not properly initialized");
	}


	void Program::ThreadMonitor::cleanupAfterChildTermination()
	{
		if (procinfo.timeoutThread.get()) // stop the thread dedicated to handle the timeout
		{
			procinfo.timeoutThread->stop();
			procinfo.timeoutThread.reset();
		}
		closeHD(channels.outfd[1]);
		closeHD(channels.infd[0]);
		closeHD(channels.errd[0]);
		closeHD(processHandle);
		closeHD(threadHandle);
	}


	void Program::ThreadMonitor::onKill()
	{
		// the thread has been killed - killing the sub process if any
		bool killed = false;

		// try to kill the attached child process if any
		{
			// killing the sub-process, until it is really dead
			::TerminateProcess(processHandle, (uint)-127);
			uint32 waitStatus = ::WaitForSingleObject(processHandle, INFINITE);
			// getting the current time as soon as possible
			pEndTime = currentTime();
			switch (waitStatus)
			{
				case WAIT_FAILED:
					// the process was probably already dead
					break;
				default:
					// the process existed and has been killed
					killed = true;
					pExitStatus = -127;
					break;
			}
			MutexLocker locker(procinfo.mutex);
			procinfo.processID = 0;

			cleanupAfterChildTermination();
		}

		theProcessHasStopped(killed, -127);
	}




} // namespace Process
} // namespace Yuni

#endif // YUNI_OS_WINDOWS
