// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#define MaxFileLength 32 // Do dai quy uoc cho file name
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
char* User2System(int virtAddr, int limit)
{
	int i; //chi so index
	int oneChar;
	char* kernelBuf = NULL;
	kernelBuf = new char[limit + 1]; //can cho chuoi terminal
	if (kernelBuf == NULL)
		return kernelBuf;
		
	memset(kernelBuf, 0, limit + 1);
	
	for (i = 0; i < limit; i++)
	{
		machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}
int System2User(int virtAddr, int len, char* buffer)
{
	if (len < 0) return -1;
	if (len == 0)return len;
	int i = 0;
	int oneChar = 0;
	do{
		oneChar = (int)buffer[i];
		machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);
	return i;
}
void increasePC(){
    int pcAfter = machine->ReadRegister(PCReg) + 4;
    machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
    machine->WriteRegister(NextPCReg, pcAfter);
}
void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
    switch (which)
    {
    case NoException:
        return;
    case PageFaultException:
    {
        DEBUG('a', "\n No valid translation found");
        printf("\n No valid translation found");
        interrupt->Halt();
        return;
    }
    case ReadOnlyException:
    {
        DEBUG('a', "\n Write attempted to page marked read-only");
        printf("\n Write attempted to page marked read-only");
        interrupt->Halt();
        return;
    }
    case BusErrorException:
    {
        DEBUG('a', "\n Translation resulted in an invalid physical address");
        printf("\n Translation resulted in an invalid physical address");
        interrupt->Halt();
        return;
    }
    case AddressErrorException:
    {
        DEBUG('a', "\n Unaligned reference or one that was beyond the end of the address space");
        printf("\n Unaligned reference or one that was beyond the end of the address space");
        interrupt->Halt();
        return;
    }
    case OverflowException:
    {
        DEBUG('a', "\n Integer overflow in add or sub.");
        printf("\n Integer overflow in add or sub.");
        interrupt->Halt();
        return;
    }   
    case IllegalInstrException:
    {
        DEBUG('a', "\n Unimplemented or reserved instr.");
        printf("\n Unimplemented or reserved instr.");
        interrupt->Halt();
        return;
    }
    case NumExceptionTypes:
    {
        DEBUG('a', "\n Number exception types.");
        printf("\n Number exception types.");
        interrupt->Halt();
        return;
    }
    case SyscallException:
    {
        switch (type)
        {
        case SC_Halt:
        {
            DEBUG('a', "\nShutdown, initiated by user program.\n");
            printf("\nShutdown, initiated by user program.\n");
            interrupt->Halt();
            break;
        }
        case SC_ReadInt:
        {
            char* buff = new_char[MaxBuffer + 1];
            int len = gSynchConsole->Read(buff, MaxBuffer);
            buff[len] = '\0';   
            
            //kiem tra so am
            int sign = 1; //so duong
            if(buff[0] == '-'){
                buff[0] = '0';
                sign = -1;
            }
            int res = 0;
            for(int i = 0; i < len; i++){
                if(buff[i] < '0' || buff[i] > '9'){
                    DEBUG('a', "\nInvalid number");
                    printf("\nInvalid number");
                    delete buff;
                    machine->WriteRegister(2, 0);//tra ve 0
                    increasePC();
                    return;
                }
                //buff[i] la so chuyen ve int
                res = res * 10 + (buff[i] - '0');
            }
            machine->WriteRegister(2, res);
            delete buff;
            break;  
        }
        case SC_PrintInt:
        {
            int number = machine->ReadRegister(4);
            //neu la so 0
            if(number == 0){
                gSynchConsole->Write("0", 1);
                increasePC();
                break;
            }
            char* buff = new char[MaxBuffer + 1]; //chuoi ki tu den in so nguyen
            bool isNeg = false;
            if(number < 0){
                isNeg = true;
                number = -number;
            }
            int len = 0;
            int temp = number;
            // while(temp > 0){
            //     buff[len++] = (temp % 10) + '0';
            //     temp /= 10;
            // }
            while (temp > 0)
            {
                len++;
                temp /= 10;
            }
            for(int i = len - 1; i >= 0; i--){
                buff[i] = (number % 10) + '0';
                number /= 10;
            }
            buff[len] = '\0';
            if (isNeg)
            {
                gSynchConsole->Write("-", 1);
            }
            gSynchConsole->Write(buff, len + 1);
            delete buff;
            break;   
        }
        case SC_ReadChar:
        {
            
        }
        case SC_Exit:
        {
            int exitStatus = machine->ReadRegister(4);
            printf("\n Exit status: %d", exitStatus);
            currentThread->Finish();
            break;
        }
        case SC_Exec:
        {
            int fileNameAddr = machine->ReadRegister(4);
            char *fileName = new char[100];
            int i = 0;
            do
            {
                machine->ReadMem(fileNameAddr + i, 1, (int *)(fileName + i));
            } while (fileName[i++] != '\0');
            OpenFile *executable = fileSystem->Open(fileName);
            AddrSpace *space;
            if (executable == NULL)
            {
                printf("\n Unable to open file %s", fileName);
                machine->WriteRegister(2, -1);
                delete executable;
                break;
            }
            space = new AddrSpace(executable);
            Thread *thread = new Thread(fileName);
            thread->space = space;
            thread->Fork(StartProcess, (int)thread);
            machine->WriteRegister(2, thread->getPid());
            delete executable;
            break;
        }
        case SC_Join:
        {
            int pid = machine->ReadRegister(4);
            Thread *thread = getThreadFromPid(pid);
            if (thread == NULL)
            {
                printf("\n No thread with pid %d", pid);
                machine->WriteRegister(2, -1);
                break;
            }
            thread->Join();
            machine->WriteRegister(2, thread->getExitStatus());
            break;
        }
        case SC_Create:
        {
            int fileNameAddr = machine->ReadRegister(4);
            char *fileName = new char[100];
            int i = 0;
            do
            {
                machine->ReadMem(fileNameAddr + i, 1, (int *)(fileName + i));
            } while (fileName[i++] != '\0');
            if (fileSystem->Create(fileName, 0))
            {
                printf("\n File %s created", fileName);
                machine->WriteRegister(2, 0);
            }
        }
    }    
    }
} 
