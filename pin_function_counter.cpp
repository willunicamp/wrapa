/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2015 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

std::ofstream TraceFile;
char *file;
/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "callfunctions.out", "specify trace file name");

/* ===================================================================== */
/* Analysis routines                                                     */
/* ===================================================================== */
 
/*Saves the name of each function executed*/
VOID ArgBefore(const char * image, const char *section, const char *name, const char *caller, void* size){
	//TraceFile << file << " : " << image << endl;
 	if((strstr(image,file) != NULL) || (strstr(file,image) != NULL)){
		if(strstr(section,".text")){
		TraceFile << name << endl;
		//TraceFile << "Image: " << image << " :: Section: " << section << " :: Function: " << name << "(" << size << ")" << endl;
		}
	}
}


/* ===================================================================== */
/* Instrumentation routines                                              */
/* ===================================================================== */
VOID Image(IMG img, VOID *v){
    const char *image, *section, *name, *caller;
    for(SEC sec = IMG_SecHead(img);SEC_Valid(sec); sec = SEC_Next(sec)){
	    	for ( RTN funcRtn = SEC_RtnHead(sec); RTN_Valid(funcRtn); funcRtn = RTN_Next(funcRtn)){
			RTN_Open(funcRtn);
			section = SEC_Name(sec).c_str();
			name = RTN_Name(funcRtn).c_str();
			image = IMG_Name(img).c_str();
			RTN_InsertCall(funcRtn, IPOINT_BEFORE, (AFUNPTR)ArgBefore,
			       IARG_ADDRINT, image,
			       IARG_ADDRINT, section,
			       IARG_ADDRINT, name,
			       IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
			       IARG_END);
	
			RTN_Close(funcRtn);
	}
    }
}


VOID Fini(INT32 code, VOID *v){
    TraceFile.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */
   
INT32 Usage(){
    cerr << "This tool produces a trace of calls to malloc." << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[]){
    // Initialize pin & symbol manager
    PIN_InitSymbols();
    if( PIN_Init(argc,argv) ) {
        return Usage();
    }
    file=argv[12];
    // Write to a file since cout and cerr maybe closed by the application
    TraceFile.open(KnobOutputFile.Value().c_str());
    TraceFile << hex;
    TraceFile.setf(ios::showbase);
    TraceFile << file << endl << endl;   
    
    // Register Image to be called to instrument functions.
    IMG_AddInstrumentFunction(Image, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();
    
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
