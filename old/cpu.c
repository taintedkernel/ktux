#include <stdio.h> 
#include <string.h> 

void main() { 
  char VendorSign[13];   //We need somewhere to store our vendorstring 
  unsigned long MaxEAX;  //This will be used to store the maximum EAX 
                         //possible to call CPUID with. 

  _asm { 
	//MOV		  EAX, 1
    XOR       EAX,                        EAX 
    //An efficient alternatvie to MOV EAX, 0x0 

    CPUID 
    //This instruction will load our registers with the data we need. 

    MOV       dword ptr [VendorSign],     EBX 
    //Copy the first 4 bytes in the VendorString from EBX. 

    MOV       dword ptr [VendorSign+4],   EDX 
    //Copy the next 4 bytes. 

    MOV       dword ptr [VendorSign+8],   ECX 
    //Copy the next 4 bytes. 

    MOV       dword ptr MaxEAX,           EAX 
    //EAX contains the maximum value to call CPUID with. Copy it to the 
    //MaxEAX variable. 
  } 
  VendorSign[12]=0;  //The last character in the VendorSign can be anything. 
                     //To make sure that it stops at the last character we add 
                     //a zero character at the end 


  printf("Vendor string: %s\n", VendorSign); 
  printf("Maximum EAX value: %i\n", MaxEAX);

    char* Comp1[32];  //This is the array that will hold the short names
                    //for our features bitmap. The names added below is 
                    //valid for the Intel processors and many of them
                    //are valid for other processors as well.

  if(strcmp(VendorSign, "GenuineIntel")==0) {
    Comp1[0]="FPU";   //Floating Point Unit
    Comp1[1]="VME";   //Virtual Mode Extension
    Comp1[2]="DE";    //Debugging Extension
    Comp1[3]="PSE";   //Page Size Extension
    Comp1[4]="TSC";   //Time Stamp Counter
    Comp1[5]="MSR";   //Model Specific Registers
    Comp1[6]="PAE";   //Physical Address Extesnion
    Comp1[7]="MCE";   //Machine Check Extension
    Comp1[8]="CX8";   //CMPXCHG8 Instruction
    Comp1[9]="APIC";  //On-chip APIC Hardware
    Comp1[10]="";     //Reserved
    Comp1[11]="SEP";  //SYSENTER SYSEXIT
    Comp1[12]="MTRR"; //Machine Type Range Registers
    Comp1[13]="PGE";  //Global Paging Extension
    Comp1[14]="MCA";  //Machine Check Architecture
    Comp1[15]="CMOV"; //Conditional Move Instrction
    Comp1[16]="PAT";  //Page Attribute Table
    Comp1[17]="PSE-36"; //36-bit Page Size Extension
    Comp1[18]="PSN";  //96-bit Processor Serial Number
    Comp1[19]="CLFSH"; //CLFLUSH Instruction
    Comp1[20]="";     //Reserved
    Comp1[21]="DS";   //Debug Trace Store
    Comp1[22]="ACPI"; //ACPI Support
    Comp1[23]="MMX";  //MMX Technology
    Comp1[24]="FXSR"; //FXSAVE FXRSTOR (Fast save and restore)
    Comp1[25]="SSE";  //Streaming SIMD Extensions
    Comp1[26]="SSE2"; //Streaming SIMD Extensions 2
    Comp1[27]="SS";   //Self-Snoop
    Comp1[28]="HTT";  //Hyper-Threading Technology
    Comp1[29]="TM";   //Thermal Monitor Supported
    Comp1[30]="IA-64"; //IA-64 capable
    Comp1[31]="";     //Reserved
  }
  else if(strcmp(VendorSign, "AuthenticAMD")==0) {
    Comp1[0]="FPU";   //Floating Point Unit
    Comp1[1]="VME";   //Virtual Mode Extension
    Comp1[2]="DE";    //Debugging Extension
    Comp1[3]="PSE";   //Page Size Extension
    Comp1[4]="TSC";   //Time Stamp Counter
    Comp1[5]="MSR";   //Model Specific Registers
    Comp1[6]="PAE";   //Physical Address Extesnion
    Comp1[7]="MCE";   //Machine Check Extension
    Comp1[8]="CX8";   //CMPXCHG8 Instruction
    Comp1[9]="APIC";  //On-chip APIC Hardware
    Comp1[10]="";     //Reserved
    Comp1[11]="SEP";  //SYSENTER SYSEXIT
    Comp1[12]="MTRR"; //Machine Type Range Registers
    Comp1[13]="PGE";  //Global Paging Extension
    Comp1[14]="MCA";  //Machine Check Architecture
    Comp1[15]="CMOV"; //Conditional Move Instrction
    Comp1[16]="PAT";  //Page Attribute Table
    Comp1[17]="PSE-36"; //36-bit Page Size Extension
    Comp1[18]="";     //?
    Comp1[19]="MPC";  //MultiProcessing Capable (I made this short up, correct?)
    Comp1[20]="";     //Reserved
    Comp1[21]="";     //?
    Comp1[22]="MIE";  //AMD Multimedia Instruction Extensions (I made this short up, correct?)
    Comp1[23]="MMX";  //MMX Technology
    Comp1[24]="FXSR"; //FXSAVE FXRSTOR (Fast save and restore)
    Comp1[25]="SSE";  //Streaming SIMD Extensions
    Comp1[26]="";     //?
    Comp1[27]="";     //?
    Comp1[28]="";     //?
    Comp1[29]="";     //?
    Comp1[30]="3DNowExt"; //3DNow Instruction Extensions (I made this short up, correct?)
    Comp1[31]="3DNow"; //3DNow Instructions (I made this short up, correct?)
  }

  unsigned long REGEAX, REGEBX, REGECX, REGEDX;
  int dFamily, dModel, dStepping, dFamilyEx, dModelEx;
  char dType[10];
  int dComp1Supported[32];
  int dBrand, dCacheLineSize, dLogicalProcessorCount, dLocalAPICID;

  if(MaxEAX>=1) {
    _asm {
      MOV     EAX,                    1
      CPUID
      MOV     [REGEAX],               EAX
      MOV     [REGEBX],               EBX
      MOV     [REGECX],               ECX
      MOV     [REGEDX],               EDX
    }
    dFamily=((REGEAX>>8)&0xF);
    dModel=((REGEAX>>4)&0xF);
    dStepping=(REGEAX&0xF);

    dFamilyEx=((REGEAX>>20)&0xFF);
    dModelEx=((REGEAX>>16)&0xF);
    switch(((REGEAX>>12)&0x7)) {
      case 0:
        strcpy(dType, "Original");
        break;
      case 1:
        strcpy(dType, "OverDrive");
        break;
      case 2:
        strcpy(dType, "Dual");
        break;
    }

    for(unsigned long C=1, Q=0;Q<32;C*=2, Q++) {
      dComp1Supported[Q]=(REGEDX&C)!=0?1:0;
    }


    dBrand=REGEBX&0xFF;
    dCacheLineSize=((strcmp(Comp1[19], "CLFSH")==0)&&(dComp1Supported[19]==1))?((REGEBX>>8)&0xFF)*8:-1;
    dLogicalProcessorCount=((strcmp(Comp1[28], "HTT")==0)&&(dComp1Supported[28]==1))?((REGEBX>>16)&0xFF):-1;
    dLocalAPICID=((REGEBX>>24)&0xFF); //This only works on P4 or later, must check for that in the future

  }
  
  printf("%s\n", dType);
  printf("Family %i, Model %i, Stepping %i\n", dFamily, dModel, dStepping);
  printf("Extended Family %i, Extended Model %i\n", dFamilyEx, dModelEx);
  printf("Supported flags: ");
  for(unsigned long Q=0;Q<27;Q++) {
    if(dComp1Supported[Q]) {
      printf("%s ", Comp1[Q]);
    }
  }
  printf("\n");
  printf("CacheLineSize: %i\n", dCacheLineSize);
  printf("Logical processor count: %i\n", dLogicalProcessorCount);
  printf("Local APIC ID: %i\n", dLocalAPICID);


} 
