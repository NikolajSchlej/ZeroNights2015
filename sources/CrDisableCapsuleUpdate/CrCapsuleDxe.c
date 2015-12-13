#include <Uefi.h>
#include <Protocol/Capsule.h>
#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

EFI_STATUS
EFIAPI
UpdateCapsule (
  IN EFI_CAPSULE_HEADER      **CapsuleHeaderArray,
  IN UINTN                   CapsuleCount,
  IN EFI_PHYSICAL_ADDRESS    ScatterGatherList OPTIONAL
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
QueryCapsuleCapabilities (
  IN  EFI_CAPSULE_HEADER   **CapsuleHeaderArray,
  IN  UINTN                CapsuleCount,
  OUT UINT64               *MaxiumCapsuleSize,
  OUT EFI_RESET_TYPE       *ResetType
  )
{
  return EFI_UNSUPPORTED;
}
  
EFI_STATUS
EFIAPI
CrCapsuleDxeEntry (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
    EFI_STATUS  Status;
    EFI_HANDLE  NewHandle = NULL;
  
    //
    // Install capsule runtime services into UEFI runtime services table.
    //
    gRT->UpdateCapsule                    = UpdateCapsule;
    gRT->QueryCapsuleCapabilities         = QueryCapsuleCapabilities;

    //
    // Install the Capsule Architectural Protocol on a new handle
    // to signify the capsule runtime services are ready.
    //
    Status = gBS->InstallMultipleProtocolInterfaces(
                      &NewHandle,
                      &gEfiCapsuleArchProtocolGuid,
                      NULL,
                      NULL);
    ASSERT_EFI_ERROR(Status);

    return Status;
}
