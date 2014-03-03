//
// ExeHandler - Win32 DLL library component to start sub-process, put stdin data once
//                   and periodically get stdout data.
// by Chessforeva, 2014

#include "stdafx.h"
#include <windows.h>

#using <System.dll>
using namespace System;
using namespace System::IO;
using namespace System::Diagnostics;
using namespace System::ComponentModel;

#include <cstdlib>
#include <iostream>

extern "C" int __declspec(dllexport) add_process(char *exeName, char *exeArgs, char *stdinStr, int waitForMs);
extern "C" int __declspec(dllexport) get_status(int id);
extern "C" int __declspec(dllexport) get_stdout(int id, char *b);
extern "C" int __declspec(dllexport) get_errors(int id, char *b);
extern "C" int __declspec(dllexport) put_stdin(int id, char *b);
extern "C" int __declspec(dllexport) kill_process(int id);
extern "C" int __declspec(dllexport) release_all();

#pragma data_seg (".ExeHandlerSec")

public struct buffdata
{
	int id;				// unique id of sub-process, assigned by system
	std::string bk;		// stream returned from sub-process
	std::string err;	// string contains error messages of exceptions
	bool exited;		// if exited event happened
};

const int TB_size = 20;		// max.processes
int tb_cnt=0;				// cnt for sub-processes, except id=-1
buffdata table[TB_size]={};	// table of structs for sub-processes

#pragma data_seg()

#pragma comment(linker, "/section:.ExeHandlerSec,RWS")

// converter Sting^ to std::string
std::string to_std_string(System::String ^s)
{
	std::string r="";
    array<unsigned char> ^chars = System::Text::Encoding::ASCII->GetBytes(s);
    for(int i=0;i<chars->Length;i++) r+=chars[i]; 
	return r;
};

int get_status(int id)
{
	int r=0;
	for(int i=0;i<tb_cnt;i++)
		if(table[i].id==id)
			{
			r|=1;
			if(table[i].exited) r|=2;
			else
				{
				try
					{
					Process^ p = Process::GetProcessById(id);
					if(p) {	r|=4; if(p->HasExited) r|=8; }
					}
				catch ( Exception^ e ) 
					{
					table[i].err.append( "Status-exception:" );
					table[i].err.append( to_std_string(e->Message) );
					table[i].err.append( "\n" );
					}
				}
			break;
			}
	return r;
}

int kill_process(int id)
{
	int n=0;
	for(int i=0;i<tb_cnt;i++)
		if(table[i].id==id)
			{
			if(!table[i].exited)
			{
			try
				{
				Process^ p = Process::GetProcessById(id);
				if(p) p->Kill();
				}
			catch ( Exception^ e ) 
				{
				//table[i].err.append( "Kill-exception:" );
				//table[i].err.append( to_std_string(e->Message) );
				//table[i].err.append( "\n" )
				}
			}
			table[i].id = -1;
			table[i].bk = table[i].err = "";
			n++;
			break;
			}
	return n;
}

int release_all()
{
	int n=0;
	for(int i=0;i<tb_cnt;i++)
		if(table[i].id!=-1) n+=kill_process(table[i].id);
	tb_cnt=0;
	return n;
};

bool IsData( String^ s ) {	return !String::IsNullOrEmpty( s ); }

static void OutputHandler( Object^ o, DataReceivedEventArgs^ outLine )
{
 if ( IsData( outLine->Data ) )
	{
		 Process^ p = (Process^)o;
		 for(int i=0;i<tb_cnt;i++)
			{
			 if(p && p->Id==table[i].id)
			 {
				 table[i].bk.append( to_std_string(outLine->Data) );
				 table[i].bk.append( "\n" );
				 break;
			 }
			}
	}
};

static void ProcessExited( Object^ o, EventArgs^ arg)
{
	Process^ p = (Process^)o;
	for(int i=0;i<tb_cnt;i++)
		if(p && p->Id==table[i].id)
			{
			table[i].exited = true;	// keep information
			break;
			}
};

static void ErrorHandler( Object^ o, DataReceivedEventArgs^ outLine )
{
 if ( IsData( outLine->Data ) )
	{
		 Process^ p = (Process^)o;
		 for(int i=0;i<tb_cnt;i++)
			{
			 if(p && p->Id==table[i].id)
			 {
				 table[i].err.append( "ErrorStream:" );
				 table[i].err.append( to_std_string(outLine->Data) );
				 table[i].err.append( "\n" );
				 break;
			 }
			}
	}
};

String^ folderOnly(char *f)
{
	LPTSTR currentDir;
	String^ s = gcnew String(f);
	int a = s->LastIndexOf("\\");
	if(a<0)
		{
			GetCurrentDirectory(MAX_PATH, currentDir);
			s = gcnew String( currentDir );
		}
	else s=s->Substring(0,a);
	return s;
}

int add_process(char *exeName, char *exeArgs, char *stdinStr, int waitForMs)
{
	int i;
	for(i=0;i<tb_cnt;i++) if(table[i].id==-1) break;
	if(i==tb_cnt)
		{
		if(tb_cnt==TB_size) return -1;
		tb_cnt++;
		}
	table[i].bk = table[i].err = "";
	table[i].exited = false;
	table[i].id = -1;

	try
	{

	Process ^p = gcnew Process();
	if(p)
	{
	p->StartInfo->CreateNoWindow = true;
	p->StartInfo->WorkingDirectory = folderOnly(exeName);
    p->StartInfo->FileName = gcnew String(exeName);

	p->StartInfo->Arguments = gcnew String(exeArgs);
    p->StartInfo->UseShellExecute = false;

	String^ instr = gcnew String(stdinStr);
	bool in_rdr = (instr->Length>0);

	p->StartInfo->RedirectStandardInput = in_rdr;
	
	p->StartInfo->RedirectStandardOutput = true;
	p->StartInfo->RedirectStandardError = true;

	p->OutputDataReceived += gcnew DataReceivedEventHandler( OutputHandler );
	p->ErrorDataReceived += gcnew DataReceivedEventHandler( ErrorHandler );

	p->EnableRaisingEvents = true;	// catches exiting
	p->Exited += gcnew EventHandler(ProcessExited);

	p->Start();
	
	table[i].id=p->Id;

	p->BeginOutputReadLine();
	p->BeginErrorReadLine();

	if(in_rdr)
		{
		if(!p->StandardInput) table[i].err.append( "no stdin stream\n" );
		else
			{
			//p->StandardInput->AutoFlush = true;
			p->StandardInput->Write(instr);
			p->StandardInput->Flush();
			}
		}
	if(waitForMs==0) p->WaitForExit();
	if(waitForMs>0) p->WaitForExit(waitForMs);
	}
	}
	catch ( Exception^ e ) 
	{
		table[i].err.append( "Exception:" );
		table[i].err.append( to_std_string(e->Message) );
		table[i].err.append( "\n" );
	}

	return table[i].id;
}

void copyDatas(int *l, char *b, std::string *s)
{
	char *q=b;
	int ln=(*s).length();
	for((*l)=0;(*l)<ln;(*l)++) *(q++)=(*s)[(*l)];
	(*s)="";
};

int get_stdout(int id, char *b)
{
	int l=-1;
	for(int i=0;i<tb_cnt;i++)
		if(table[i].id==id) { copyDatas(&l, b,&table[i].bk); break; }
	return l;
};

int get_errors(int id, char *b)
{
	int l=-1;
	for(int i=0;i<tb_cnt;i++)
		if(table[i].id==id) { copyDatas(&l, b,&table[i].err); break; }
	return l;
};

int put_stdin(int id, char *b)		// not working
{
	int l=-1;
	for(int i=0;i<tb_cnt;i++)
		if((table[i].id==id) && (!table[i].exited))
		{
			Process^ p = Process::GetProcessById(id);
			if(p)
			{
				if(p->StartInfo->RedirectStandardInput && p->StandardInput)
				{
				String^ instr = gcnew String(b);
				p->StandardInput->Write(instr);
				p->StandardInput->Flush();
				l = instr->Length;
				}
				else table[i].err.append( "no stdin stream\n" );
			}
			break;
		}
	return l;
};