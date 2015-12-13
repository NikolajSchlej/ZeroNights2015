#include <Uefi.h>
#include <Liblrary/PeiServicesLib.h>

EFI_STATUS
EFIAPI
PreventS3PeiEntry (
  IN EFI_PEI_FILE_HANDLE       FileHandle,
  IN CONST EFI_PEI_SERVICES    **PeiServices
  )
{
    EFI_STATUS  Status;
    EFI_BOOT_MODE BootMode = BOOT_WITH_FULL_CONFIGURATION;
    
    // Get current boot mode
    (*PeiServices)->GetBootMode(PeiServices, &BootMode);
    
    // Continue normal boot if it's not S3
    if (BootMode != BOOT_ON_S3_RESUME) 
        return EFI_SUCCESS;

    // Try to reset the system
    (*PeiServices)->ResetSystem(PeiServices);
    
    // Hang if reset attempt failed
    while(1);
    
    // Should never end here
    return EFI_INVALID_PARAMETER;
}