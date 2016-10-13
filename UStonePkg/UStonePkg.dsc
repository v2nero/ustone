# Copyright (c) 2016 by Nero Liu .  ALL RIGHTS RESERVED.
# Consult your license regarding permissions and restrictions.

[Defines]
  PLATFORM_NAME                  = UStone
  PLATFORM_GUID                  = 79C5303E-C269-4731-B750-F305DC083B58
  PLATFORM_VERSION               = 1.06
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/UStone
  SUPPORTED_ARCHITECTURES        = IA32|X64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT

[Components]
  UStonePkg/Library/CppBaseLib/CppBaseLib.inf
  UStonePkg/Library/MemLeakCheckLib/MemLeakCheckLib.inf
  UStonePkg/Library/CrtSupportLib/CrtSupportLib.inf
  
  UStonePkg/Test/utils/LightRefBase/TestLightRefBase.inf
  #stl test
  UStonePkg/Test/stl/test_stl.inf
  
  #lua
  UStonePkg/Lua/Lua_5.3.3/Lua533Lib.inf
  UStonePkg/Lua/Lua_5.3.3/Lua.inf
  

[LibraryClasses]
  #
  # Entry Point Libraries
  #
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  ShellCEntryLib|ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  #
  # Common Libraries
  #
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  !if $(DEBUG_ENABLE_OUTPUT)
    DebugLib|MdePkg/Library/UefiDebugLibConOut/UefiDebugLibConOut.inf
    DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  !else   ## DEBUG_ENABLE_OUTPUT
    DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  !endif  ## DEBUG_ENABLE_OUTPUT

  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  PciLib|MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
  PciCf8Lib|MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  PerformanceLib|MdeModulePkg/Library/DxePerformanceLib/DxePerformanceLib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  FileHandleLib|MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
  SortLib|MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf

  ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf
  CrtSupportLib|UStonePkg/Library/CrtSupportLib/CrtSupportLib.inf
  MemLeakCheckLib|UStonePkg/Library/MemLeakCheckLib/MemLeakCheckLib.inf

############
  CppBaseLib|UStonePkg/Library/CppBaseLib/CppBaseLib.inf
  
  #lua
  Lua533Lib|UStonePkg/Lua/Lua_5.3.3/Lua533Lib.inf

[BuildOptions]


!include StdLib/StdLib.inc
!include AppPkg/Applications/Sockets/Sockets.inc