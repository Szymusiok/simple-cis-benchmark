#pragma once
class TestSuite
{
public:
   static bool GetMinimumPasswordLength();
   static bool GetPasswordExpiredTime();
   static bool CheckFirewall();
   static bool CheckRegistryWrite();
   static bool CheckServices();
   static bool CheckServiceTriggerStartPermissions();
   static bool CheckIfUpdateIsAutomat();
   static bool CheckFolderAccess();
   static bool CheckFolderPermissions();
   static bool CheckBitLocker();
};

