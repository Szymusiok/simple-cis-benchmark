#pragma once

#include <string>

class Test
{
public:
   enum class Result
   {
      Failed = 0,
      Success,
      Unknown
   };

   Test(std::string name, std::string description);
   ~Test();

   Result GetResult();
   std::string GetName();
   std::string GetDescription();
   void Clear();


   // TEST METHODS

   // PASSWORD
   size_t GetMinimumPasswordLength(); // GIT
   size_t GetPasswordExpiredTime();

   bool CheckFirewall();

   bool CheckRegistryWrite();
   bool CheckServices();

   bool CheckServiceTriggerStartPermissions();

   bool CheckIfUpdateIsAutomat();

   bool CheckFolderAccess();

   bool CheckFolderPermissions();

   bool CheckBitLocker();

private:
   std::string name;
   std::string description;
   Result result;
};

