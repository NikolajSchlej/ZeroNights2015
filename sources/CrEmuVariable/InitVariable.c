/** @file

  Implment all four UEFI runtime variable services and 
  install variable architeture protocol.
  
Copyright (c) 2006 - 2010, Intel Corporation. All rights reserved.<BR>
Copyright (c) 2010 - 2015, dmazar. All rights reserved.<BR>
Copyright (c) 2015, Nikolaj Schlej. All rights reserved.<BR>

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Variable.h"

EFI_EVENT   gVirtualAddressChangeEvent = NULL;
EFI_EVENT   gExitBootServicesEvent = NULL;
BOOLEAN     gAtRuntime = FALSE;

/** Original runtime services. **/
EFI_RUNTIME_SERVICES gOrgRT;

/**

  This code finds variable in storage blocks (Volatile or Non-Volatile).

  @param VariableName               Name of Variable to be found.
  @param VendorGuid                 Variable vendor GUID.
  @param Attributes                 Attribute value of the variable found.
  @param DataSize                   Size of Data found. If size is less than the
                                    data, this value contains the required size.
  @param Data                       Data pointer.
                      
  @return EFI_INVALID_PARAMETER     Invalid parameter
  @return EFI_SUCCESS               Find the specified variable
  @return EFI_NOT_FOUND             Not found
  @return EFI_BUFFER_TO_SMALL       DataSize is too small for the result
  
**/
EFI_STATUS
EFIAPI
RuntimeServiceGetVariable (
  IN CHAR16        *VariableName,
  IN EFI_GUID      *VendorGuid,
  OUT UINT32       *Attributes OPTIONAL,
  IN OUT UINTN     *DataSize,
  OUT VOID         *Data
  )
{
  return EmuGetVariable (
          VariableName,
          VendorGuid,
          Attributes OPTIONAL,
          DataSize,
          Data,
          &gVariableModuleGlobal->VariableGlobal[Physical]
          );
}

/**

  This code Finds the Next available variable.

  @param VariableNameSize           Size of the variable name
  @param VariableName               Pointer to variable name
  @param VendorGuid                 Variable Vendor Guid

  @return EFI_INVALID_PARAMETER     Invalid parameter
  @return EFI_SUCCESS               Find the specified variable
  @return EFI_NOT_FOUND             Not found
  @return EFI_BUFFER_TO_SMALL       DataSize is too small for the result

**/
EFI_STATUS
EFIAPI
RuntimeServiceGetNextVariableName (
  IN OUT UINTN     *VariableNameSize,
  IN OUT CHAR16    *VariableName,
  IN OUT EFI_GUID  *VendorGuid
  )
{
  return EmuGetNextVariableName (
          VariableNameSize,
          VariableName,
          VendorGuid,
          &gVariableModuleGlobal->VariableGlobal[Physical]
          );
}

/**

  This code sets variable in storage blocks (Volatile or Non-Volatile).

  @param VariableName                     Name of Variable to be found
  @param VendorGuid                       Variable vendor GUID
  @param Attributes                       Attribute value of the variable found
  @param DataSize                         Size of Data found. If size is less than the
                                          data, this value contains the required size.
  @param Data                             Data pointer

  @return EFI_INVALID_PARAMETER           Invalid parameter
  @return EFI_SUCCESS                     Set successfully
  @return EFI_OUT_OF_RESOURCES            Resource not enough to set variable
  @return EFI_NOT_FOUND                   Not found
  @return EFI_WRITE_PROTECTED             Variable is read-only

**/
EFI_STATUS
EFIAPI
RuntimeServiceSetVariable (
  IN CHAR16        *VariableName,
  IN EFI_GUID      *VendorGuid,
  IN UINT32        Attributes,
  IN UINTN         DataSize,
  IN VOID          *Data
  )
{
  return EmuSetVariable (
          VariableName,
          VendorGuid,
          Attributes,
          DataSize,
          Data,
          &gVariableModuleGlobal->VariableGlobal[Physical],
          &gVariableModuleGlobal->VolatileLastVariableOffset,
          &gVariableModuleGlobal->NonVolatileLastVariableOffset
          );
}

/**

  This code returns information about the EFI variables.

  @param Attributes                     Attributes bitmask to specify the type of variables
                                        on which to return information.
  @param MaximumVariableStorageSize     Pointer to the maximum size of the storage space available
                                        for the EFI variables associated with the attributes specified.
  @param RemainingVariableStorageSize   Pointer to the remaining size of the storage space available
                                        for EFI variables associated with the attributes specified.
  @param MaximumVariableSize            Pointer to the maximum size of an individual EFI variables
                                        associated with the attributes specified.

  @return EFI_INVALID_PARAMETER         An invalid combination of attribute bits was supplied.
  @return EFI_SUCCESS                   Query successfully.
  @return EFI_UNSUPPORTED               The attribute is not supported on this platform.

**/
EFI_STATUS
EFIAPI
RuntimeServiceQueryVariableInfo (
  IN  UINT32                 Attributes,
  OUT UINT64                 *MaximumVariableStorageSize,
  OUT UINT64                 *RemainingVariableStorageSize,
  OUT UINT64                 *MaximumVariableSize
  )
{
  return EmuQueryVariableInfo (
          Attributes,
          MaximumVariableStorageSize,
          RemainingVariableStorageSize,
          MaximumVariableSize,
          &gVariableModuleGlobal->VariableGlobal[Physical]
          );
}

/**

  Notification function of EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE.

  This is a notification function registered on EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event.
  It convers pointer to new virtual address.

  @param  Event        Event whose notification function is being invoked.
  @param  Context      Pointer to the notification function's context.

**/
VOID
EFIAPI
VariableClassAddressChangeEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  gRT->ConvertPointer (0x0, (VOID **) &gVariableModuleGlobal->PlatformLangCodes);
  gRT->ConvertPointer (0x0, (VOID **) &gVariableModuleGlobal->LangCodes);
  gRT->ConvertPointer (0x0, (VOID **) &gVariableModuleGlobal->PlatformLang);
  gRT->ConvertPointer (0x0, (VOID **) &gVariableModuleGlobal->VariableGlobal[Physical].NonVolatileVariableBase);
  gRT->ConvertPointer (0x0, (VOID **) &gVariableModuleGlobal->VariableGlobal[Physical].VolatileVariableBase);
  gRT->ConvertPointer (0x0, (VOID **) &gVariableModuleGlobal);
}



/**

  This function copies all RT variables present in original NVRAM into the emulated one.

  @param[in]  SrcRT   Source RT.
  @param[in]  DestRT  Destination RT.

**/
VOID
EFIAPI
CopyRTVariables (
  EFI_RUNTIME_SERVICES    *SrcRT,
  EFI_RUNTIME_SERVICES    *DestRT
)
{
  EFI_STATUS              Status;
  CHAR16                  *Name;
  EFI_GUID                Guid;
  UINTN                   NameSize;
  UINTN                   NewNameSize;
  UINT32                  Attributes;
  VOID                    *Data;
  UINTN                   DataSize;
  UINTN                   NewDataSize;
  
  //
  // First call to GetNextVariableName is with L"\0"
  //
  NameSize    = 512;
  Name        = AllocateZeroPool(NameSize);
  
  //
  // Initial Data buffer
  //
  DataSize    = 512;
  Data        = AllocateZeroPool(DataSize);
  
  while (TRUE) {
    //
    // Get next variable name from SrcRT
    //
    NewNameSize = NameSize;
    Status = SrcRT->GetNextVariableName(&NewNameSize, Name, &Guid);
    if (Status == EFI_BUFFER_TOO_SMALL) {
      Name = ReallocatePool(NameSize, NewNameSize, Name);
      if (!Name) {
        IoWrite8(0x80, 0xE8);
        while(1);
      }
      NameSize = NewNameSize;
      Status = SrcRT->GetNextVariableName(&NewNameSize, Name, &Guid);
    }
    if (EFI_ERROR(Status)) {
        break;
    }
    
    //
    // Read variable from SrcRT
    //
    NewDataSize = DataSize;
    Status = SrcRT->GetVariable(Name, &Guid, &Attributes, &NewDataSize, Data);
    if (Status == EFI_BUFFER_TOO_SMALL) {
      Data = ReallocatePool(DataSize, NewDataSize, Data);
      if (!Data) {
        IoWrite8(0x80, 0xE9);
        while(1);
      }
      DataSize = NewDataSize;
      Status = SrcRT->GetVariable(Name, &Guid, &Attributes, &NewDataSize, Data);
    }
    if (EFI_ERROR(Status)) {
      break;
    }
    
    //
    // Delete variable if exists in DestRT
    //
    Status = DestRT->SetVariable(Name, &Guid, 0, 0, NULL);

    //
    // Write variable to DestRT
    //
    Status = DestRT->SetVariable(Name, &Guid, Attributes, NewDataSize, Data);
    if (Status != EFI_SECURITY_VIOLATION && EFI_ERROR(Status)) {
      IoWrite8(0x80, (UINT8)Status);
    }
  }
  
  FreePool (Name);
  FreePool (Data);
}

/**
  
  This function starts NVRAM emulation.

**/
EFI_STATUS
EmuVariableInstallEmulation(VOID)
{
  EFI_STATUS            Status;
  EFI_RUNTIME_SERVICES  DestRT;
  EFI_GUID FlagVariableGuid = {0xd3c29c3, 0x28d6, 0x455b, {0x94, 0x5e, 0xd4, 0x3c, 0x2c, 0x9f, 0x47, 0x75}};
  UINT8 Flag = 1;
  
  if (gRT->GetVariable == RuntimeServiceGetVariable) {
    return EFI_ALREADY_STARTED;
  }
  
  //
  // Copy RT variables to emulation store
  // Use temporary DestRT structure to pass SetVariable pointer
  //
  DestRT.SetVariable = RuntimeServiceSetVariable;
  CopyRTVariables (gRT, &DestRT);
  
  //
  // Install emulation services
  //
  gRT->GetVariable         = RuntimeServiceGetVariable;
  gRT->GetNextVariableName = RuntimeServiceGetNextVariableName;
  gRT->SetVariable         = RuntimeServiceSetVariable;
  gRT->QueryVariableInfo   = RuntimeServiceQueryVariableInfo;
  
  gRT->Hdr.CRC32 = 0;
  gBS->CalculateCrc32(gRT, gRT->Hdr.HeaderSize, &gRT->Hdr.CRC32);

  //
  // Add a flag variable to allow detection of enabled emulation
  //
  Status = gRT->SetVariable(L"CrEmuVariablePresent",
                            &FlagVariableGuid,
                            EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                            sizeof(Flag),
                            &Flag);
  if (EFI_ERROR(Status)) {
    IoWrite8(0x80, 0xEB);
    while(1);
  }
  //
  // Set PR0 to protect the whole BIOS region and set FLOCKDN bit to prevent it's removal
  // All magic values below are platform-specific, use chipset datasheet to obtain them
  //
  {
    #pragma warning( disable : 4306) // Disable the warning about converting 32-bit values to 64-bit pointers and back
    UINT32 RcbaLocationAddress = 0xE00F80F0; // Memory-mapped PCI device B0:D31:F0, register offset 0xF0;
    UINT32 RootComplexBaseAddress = (*(UINT32*)RcbaLocationAddress) & 0xFFFFFC00; // Reset the first byte to get the real address
    UINT32 SpiBaseAddress = RootComplexBaseAddress + 0x3800; // SPIBAR for all registers below
    UINT32 HsfsRegisterAddress = SpiBaseAddress + 0x04;      // HSFS register for FLOCKDN
    UINT32 BfprRegisterAddress = SpiBaseAddress + 0x00;      // BFPR register to get BIOS region base and limit
    UINT32 Pr0RegisterAddress = SpiBaseAddress + 0x74;       // PR0 register to enable BIOS region write protection
    UINT32 Pr1RegisterAddress = SpiBaseAddress + 0x78;       // PR1 register to enable BIOS password read protection
    
    UINT16 HsfsRegisterValue = *(UINT16*)HsfsRegisterAddress;
    UINT32 BfprRegisterValue = *(UINT32*)BfprRegisterAddress;
    UINT32 Pr0RegisterValue = BfprRegisterValue | BIT31;     // Set PR0 to cover the whole BIOS region, enable WP
    UINT32 Pr1RegisterValue = 0x825D825C;                    // Set PR1 to cover 25C000 - 25D000 flash region, 
                                                             // where the BIOS password is stored as clear text,
                                                             // enable both RP and WP                                                             
    //
    // Check FLOCKDN bit to be 0
    //
    if ((HsfsRegisterValue & BIT15) == 0) {
      //
      // Set new PR0 and PR1 values
      //
      *(UINT32*)Pr0RegisterAddress = Pr0RegisterValue;
      *(UINT32*)Pr1RegisterAddress = Pr1RegisterValue;
      
      //
      // Set FLOCKDN bit
      //
      HsfsRegisterValue |= BIT15;
      *(UINT16*)HsfsRegisterAddress = HsfsRegisterValue;
    }    
  }
  
  return EFI_SUCCESS;
}

/**

  Notification function of EVT_SIGNAL_EXIT_BOOT_SERVICES.

  This is a notification function registered on EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event.
  It sets AtRuntime flag as TRUE after ExitBootServices.

  @param[in]  Event   The Event that is being processed.
  @param[in]  Context The Event Context.

**/
VOID
EFIAPI
VariableClassExitBootServicesEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  // Install emulation
  EFI_STATUS Status = EmuVariableInstallEmulation();
  if (Status != EFI_ALREADY_STARTED && EFI_ERROR(Status)) {
      IoWrite8(0x80, 0xE1);
      while(1);
  }
  // Set AtRuntime flag
  gAtRuntime = TRUE;
}

/**

  This function allows the caller to determine if UEFI ExitBootServices() has been called.

  This function returns TRUE after all the EVT_SIGNAL_EXIT_BOOT_SERVICES functions have
  executed as a result of the OS calling ExitBootServices().  Prior to this time FALSE
  is returned. This function is used by runtime code to decide it is legal to access
  services that go away after ExitBootServices().

  @retval  TRUE  The system has finished executing the EVT_SIGNAL_EXIT_BOOT_SERVICES event.
  @retval  FALSE The system has not finished executing the EVT_SIGNAL_EXIT_BOOT_SERVICES event.

**/
BOOLEAN
EFIAPI
VariableClassAtRuntime (
  VOID
  )
{
  return gAtRuntime;
}

/**

  EmuVariable Driver main entry point. 
  The Variable driver places the 4 EFI runtime services in the EFI System Table 
  and installs arch protocols for variable read and write services being available. 
  It also registers notification functions for EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE 
  and EVT_SIGNAL_EXIT_BOOT_SERVICES events.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.  
  @param[in] SystemTable    A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS       Variable service successfully initialized.

**/
EFI_STATUS
EFIAPI
VariableServiceInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS  Status;

  Status = VariableCommonInitialize (ImageHandle, SystemTable);
  
  //
  // Store original RT
  //
  CopyMem (&gOrgRT, gRT, sizeof(EFI_RUNTIME_SERVICES));
  
  //
  // Create a Set Virtual Address Change event.
  //
  Status = gBS->CreateEvent (EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE,
                             TPL_NOTIFY,
                             VariableClassAddressChangeEvent,
                             NULL,
                             &gVirtualAddressChangeEvent);
    if (EFI_ERROR(Status)) {
      IoWrite8(0x80, 0xEE);
      while(1);
  }
  
  //
  // Create an Exit Boot Services event.
  //
  Status = gBS->CreateEvent (EVT_SIGNAL_EXIT_BOOT_SERVICES,
                             TPL_NOTIFY,
                             VariableClassExitBootServicesEvent,
                             NULL,
                             &gExitBootServicesEvent);
  if (EFI_ERROR(Status)) {
      IoWrite8(0x80, 0xEF);
      while(1);
  }
    
  return EFI_SUCCESS;
}


