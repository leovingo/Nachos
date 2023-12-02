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
// Doi thanh ghi Program counter cua he thong ve sau 4 byte de tiep tuc nap lenh
void IncreasePC()
{
	int counter = machine->ReadRegister(PCReg);
   	machine->WriteRegister(PrevPCReg, counter);
    	counter = machine->ReadRegister(NextPCReg);
    	machine->WriteRegister(PCReg, counter);
   	machine->WriteRegister(NextPCReg, counter + 4);
}
void ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
    switch (which)
    {
    case NoException:
        return;
    case PageFaultException:
    {
        DEBUG('a', "\n No valid translation found");
        printf("\n\n No valid translation found");
        interrupt->Halt();
        return;
    }
    case ReadOnlyException:
    {
        DEBUG('a', "\n Write attempted to page marked read-only");
        printf("\n\n Write attempted to page marked read-only");
        interrupt->Halt();
        return;
    }
    case BusErrorException:
    {
        DEBUG('a', "\n Translation resulted in an invalid physical address");
        printf("\n\n Translation resulted in an invalid physical address");
        interrupt->Halt();
        return;
    }
    case AddressErrorException:
    {
        DEBUG('a', "\n Unaligned reference or one that was beyond the end of the address space");
        printf("\n\n Unaligned reference or one that was beyond the end of the address space");
        interrupt->Halt();
        return;
    }
    case OverflowException:
    {
        DEBUG('a', "\n Integer overflow in add or sub.");
        printf("\n\n Integer overflow in add or sub.");
        interrupt->Halt();
        return;
    }   
    case IllegalInstrException:
    {
        DEBUG('a', "\n Unimplemented or reserved instr.");
        printf("\n\n Unimplemented or reserved instr.");
        interrupt->Halt();
        return;
    }
    case NumExceptionTypes:
    {
        DEBUG('a', "\n Number exception types.");
        printf("\n\n Number exception types.");
        interrupt->Halt();
        return;
    }
    case SyscallException:
    {
        switch (type)
        {
        case SC_Halt:
        {
            DEBUG('a', "\n Shutdown, initiated by user program.\n");
            printf("\n\n Shutdown, initiated by user program.\n");
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
                    printf("\n\n Invalid number");
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
        case SC_PrintChar:
        {
            
        }
        case SC_ReadString:
        {
        }
        case SC_PrintString:
        {
        }    
    }
    }
} 
