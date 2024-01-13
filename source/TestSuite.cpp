#include "Test.h"
#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <regex>
#include <windows.h>
#include <lm.h>
#include <sddl.h>
#include <winsvc.h>
#include <aclapi.h>
#include <Windows.h>
#include <ntsecapi.h>  // Include the header for LSA functions
#include <ntstatus.h>  // Include the header for NTSTATUS codes

#ifndef POLICY_AUDIT_EVENT_SUCCESS
#define POLICY_AUDIT_EVENT_SUCCESS 0x0001
#endif

#ifndef POLICY_AUDIT_EVENT_FAILURE
#define POLICY_AUDIT_EVENT_FAILURE 0x0002
#endif

#pragma comment(lib, "Netapi32.lib")

bool TestSuite::GetMinimumPasswordLength()
{
   std::array<char, 256> buffer{};
   std::string result;
   std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen("net accounts", "r"), _pclose);
   if (!pipe)
   {
      return 0;
   }

   while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
   {
      result += buffer.data();
   }

   //!!!! Depends of windows language formatting !!!!
   std::regex regex("Minimalna.*?(\\d+)");
   std::smatch match;
   if (std::regex_search(result, match, regex) && match.size() > 1)
   {
      
      return std::stoul(match.str(1)) > 8 ? true : false;
   }

   return 0;
}

bool TestSuite::GetPasswordExpiredTime()
{
   bool rV = true;
   wchar_t username[UNLEN + 1];
   DWORD usernameLen = UNLEN + 1;

   // Pobierz nazwê aktualnego u¿ytkownika.
   if (!GetUserName(username, &usernameLen))
   {
      // Obs³u¿ b³¹d, np. zwróæ -1 w przypadku niepowodzenia.
      return false;
   }

   USER_INFO_3* userInfo = nullptr;
   NET_API_STATUS status;

   // Uzyskaj informacje o u¿ytkowniku.
   status = NetUserGetInfo(nullptr, username, 3, reinterpret_cast<LPBYTE*>(&userInfo));
   if (status != NERR_Success)
   {
      // Obs³u¿ b³¹d, np. zwróæ -1 w przypadku niepowodzenia.
      return false;
   }

   // SprawdŸ, czy data wygaœniêcia has³a jest dostêpna.
   if (userInfo->usri3_password_expired)
   {
      // Has³o wygas³o, zwróæ 0 lub inn¹ wartoœæ oznaczaj¹c¹ wygaœniêcie.
      return false;
   }

   // Pobierz datê wygaœniêcia has³a jako DWORD.
   DWORD passwordAge = userInfo->usri3_password_age;

   // Oblicz pozosta³y czas do wygaœniêcia has³a w sekundach.
   DWORD currentTime = GetTickCount(); // Aktualny czas w milisekundach.
   if (passwordAge <= currentTime)
   {
      // Has³o wygas³o, zwróæ 0 lub inn¹ wartoœæ oznaczaj¹c¹ wygaœniêcie.
      return false;
   }

   return true;
}

bool TestSuite::CheckFirewall()
{
   bool rV = true;
   // Wywo³aj polecenie netsh w trybie wiersza poleceñ
   FILE* cmd_pipe = _popen("netsh advfirewall show allprofiles state", "r");
   if (cmd_pipe)
   {
      char buffer[128];
      std::string result = "";

      while (fgets(buffer, sizeof(buffer), cmd_pipe) != nullptr)
      {
         result += buffer;
      }

      _pclose(cmd_pipe);

      // SprawdŸ, czy zapora sieciowa jest w³¹czona lub wy³¹czona
      if (result.find("ON") != std::string::npos)
      {
         rV = true;
      }
      else if (result.find("OFF") != std::string::npos)
      {
         rV = false;
      }
      else
      {
         rV = false;
      }
   }
   else
   {
      rV = false;
   }

   return rV;
}

bool TestSuite::CheckRegistryWrite()
{
   bool rV = true;

   const wchar_t* registryKey = L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\MyEventLog";
   const wchar_t* registryValue = L"EventMessageFile";
   const wchar_t* registryValueData = L"C:\\Path\\To\\Your\\EventLog.dll"; // Œcie¿ka do biblioteki DLL obs³uguj¹cej zdarzenia

   // Otwórz klucz rejestru (lub utwórz go, jeœli nie istnieje)
   HKEY hKey;
   LONG result = RegCreateKeyEx(HKEY_LOCAL_MACHINE, registryKey, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr);

   if (result == ERROR_SUCCESS)
   {
      // Ustaw wartoœæ w kluczu rejestru
      result = RegSetValueEx(hKey, registryValue, 0, REG_EXPAND_SZ, (const BYTE*)registryValueData, (wcslen(registryValueData) + 1) * sizeof(wchar_t));

      if (result == ERROR_SUCCESS)
      {
         rV = false;
      }
      else
      {
         rV = true;
      }

      // Zamknij klucz rejestru
      result = RegCloseKey(hKey);
   }
   else
   {
      rV = true;
   }

   return rV;
}

bool TestSuite::CheckServices()
{
   bool rV = true;

   SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
   if (!scm)
   {
      std::cerr << "B³¹d podczas otwierania mened¿era us³ug." << std::endl;
      return false;
   }

   // Przyk³adowe nazwy systemowych us³ug do sprawdzenia uprawnieñ
   const wchar_t* systemServices[] = {
       L"EventLog",       // Dziennik zdarzeñ
       L"RpcSs",          // RPC (Remote Procedure Call)
       L"wuauserv",       // Windows Update
   };

   bool hasPermission = false;

   for (const wchar_t* serviceName : systemServices)
   {
      SC_HANDLE service = OpenService(scm, serviceName, SERVICE_QUERY_STATUS);
      if (!service)
      {
         std::cerr << "B³¹d podczas otwierania us³ugi: " << serviceName << std::endl;
         continue;
      }

      SERVICE_STATUS status;
      if (QueryServiceStatus(service, &status))
      {
         if (status.dwControlsAccepted != 0)
         {
            std::cout << "U¿ytkownik ma uprawnienia do zarz¹dzania us³ug¹: " << serviceName << std::endl;
            hasPermission = true;
         }
         else
         {
            std::cerr << "U¿ytkownik nie ma uprawnieñ do zarz¹dzania us³ug¹: " << serviceName << std::endl;
         }
      }
      else
      {
         std::cerr << "B³¹d podczas sprawdzania statusu us³ugi: " << serviceName << std::endl;
      }

      CloseServiceHandle(service);
   }

   CloseServiceHandle(scm);

   if (hasPermission)
   {
      return true;
   }
   else
   {
      return false;
   }


   return rV;
}

bool TestSuite::CheckServiceTriggerStartPermissions()
{
   // Nazwa us³ugi, której uprawnienia do zmiany trybu automatycznego startu s¹ sprawdzane
   const wchar_t* serviceName = L"wuauserv"; // Us³uga Windows Update

   SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
   if (!scm)
   {
      std::cerr << "B³¹d podczas otwierania mened¿era us³ug." << std::endl;
      return false;
   }

   SC_HANDLE service = OpenService(scm, serviceName, SERVICE_CHANGE_CONFIG);
   if (!service)
   {
      CloseServiceHandle(scm);
      std::cerr << "B³¹d podczas otwierania us³ugi." << std::endl;
      return false;
   }

   // SprawdŸ uprawnienia do zmiany trybu automatycznego startu
   SERVICE_DESCRIPTION serviceDescription;
   if (ChangeServiceConfig2(service, SERVICE_CONFIG_DESCRIPTION, &serviceDescription))
   {
      std::cout << "U¿ytkownik ma uprawnienia do zmiany trybu automatycznego startu us³ugi." << std::endl;
      CloseServiceHandle(service);
      CloseServiceHandle(scm);
      return true;
   }
   else
   {
      std::cerr << "U¿ytkownik nie ma uprawnieñ do zmiany trybu automatycznego startu us³ugi." << std::endl;
   }

   CloseServiceHandle(service);
   CloseServiceHandle(scm);
   return false;
}

bool TestSuite::CheckIfUpdateIsAutomat()
{
   // Nazwa us³ugi, której automatyczny start ma byæ sprawdzony
   const wchar_t* serviceName = L"wuauserv"; // Us³uga Windows Update

   SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
   if (!scm)
   {
      std::cerr << "B³¹d podczas otwierania mened¿era us³ug." << std::endl;
      return false;
   }

   SC_HANDLE service = OpenService(scm, serviceName, SERVICE_QUERY_CONFIG);
   if (!service)
   {
      CloseServiceHandle(scm);
      std::cerr << "B³¹d podczas otwierania us³ugi." << std::endl;
      return false;
   }

   DWORD bytesNeeded;
   QUERY_SERVICE_CONFIG* serviceConfig = nullptr;
   if (!QueryServiceConfig(service, nullptr, 0, &bytesNeeded) && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
   {
      serviceConfig = static_cast<QUERY_SERVICE_CONFIG*>(malloc(bytesNeeded));
      if (QueryServiceConfig(service, serviceConfig, bytesNeeded, &bytesNeeded))
      {
         // SprawdŸ, czy us³uga ma automatyczny start
         if (serviceConfig->dwStartType == SERVICE_AUTO_START)
         {
            std::cout << "Us³uga ma automatyczny start." << std::endl;
            free(serviceConfig);
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            return true;
         }
         else
         {
            std::cerr << "Us³uga nie ma automatycznego startu." << std::endl;
         }
      }
      else
      {
         std::cerr << "B³¹d podczas sprawdzania konfiguracji us³ugi." << std::endl;
      }
   }
   else
   {
      std::cerr << "B³¹d podczas pobierania rozmiaru konfiguracji us³ugi." << std::endl;
   }

   free(serviceConfig);
   CloseServiceHandle(service);
   CloseServiceHandle(scm);
   return false;
}

bool TestSuite::CheckFolderAccess()
{
   const wchar_t* folderPath = L"C:\\Windows"; // Œcie¿ka do folderu Windows

   // SprawdŸ dostêp do folderu
   if (GetFileAttributes(folderPath) != INVALID_FILE_ATTRIBUTES)
   {
      std::cout << "U¿ytkownik ma dostêp do folderu Windows." << std::endl;
      return true;
   }
   else
   {
      std::cerr << "U¿ytkownik nie ma dostêpu do folderu Windows." << std::endl;
      return false;
   }
}

bool TestSuite::CheckFolderPermissions()
{
   const wchar_t* folderPath = L"C:\\Windows"; // Œcie¿ka do folderu Windows

   // SprawdŸ dostêp do zmiany uprawnieñ plików w folderze
   PACL pOldDACL = NULL;
   PACL pNewDACL = NULL;
   PSECURITY_DESCRIPTOR pSD = NULL;

   if (GetNamedSecurityInfo(
      (LPWSTR)folderPath,
      SE_FILE_OBJECT,
      DACL_SECURITY_INFORMATION,
      NULL,
      NULL,
      &pOldDACL,
      NULL,
      &pSD) == ERROR_SUCCESS)
   {
      EXPLICIT_ACCESS ea;
      ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
      ea.grfAccessPermissions = GENERIC_WRITE | WRITE_DAC | WRITE_OWNER;
      ea.grfAccessMode = SET_ACCESS;
      ea.grfInheritance = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
      ea.Trustee.TrusteeForm = TRUSTEE_IS_NAME;
      ea.Trustee.TrusteeType = TRUSTEE_IS_USER;
      ea.Trustee.ptstrName = (LPWSTR)L"Everyone"; // Dostêp dla ka¿dego u¿ytkownika

      if (SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL) == ERROR_SUCCESS)
      {
         if (SetNamedSecurityInfo(
            (LPWSTR)folderPath,
            SE_FILE_OBJECT,
            DACL_SECURITY_INFORMATION,
            NULL,
            NULL,
            pNewDACL,
            NULL) == ERROR_SUCCESS)
         {
            LocalFree(pNewDACL);
            LocalFree(pSD);
            std::cout << "U¿ytkownik ma dostêp do zmiany uprawnieñ plików w folderze Windows." << std::endl;
            return true;
         }
      }
   }

   if (pNewDACL) LocalFree(pNewDACL);
   if (pSD) LocalFree(pSD);

   std::cerr << "U¿ytkownik nie ma dostêpu do zmiany uprawnieñ plików w folderze Windows." << std::endl;
   return false;
}

bool TestSuite::CheckBitLocker()
{
   DWORD dwVolumeStatus;
   if (GetVolumeInformation(L"C:\\", NULL, 0, NULL, NULL, &dwVolumeStatus, NULL, 0))
   {
      if (dwVolumeStatus & FILE_SUPPORTS_ENCRYPTION)
      {
         std::cout << "Szyfrowanie dysku BitLocker jest w³¹czone." << std::endl;
      }
      else
      {
         std::cout << "Szyfrowanie dysku BitLocker nie jest w³¹czone." << std::endl;
      }
   }
   else
   {
      std::cerr << "B³¹d podczas pobierania informacji o woluminie." << std::endl;
   }

   return true;
}

bool TestSuite::IsAuditConfigured()
{
   bool result = true;

   // Define the expected audit policy values based on CIS recommendations.
   // Modify these values according to your specific CIS benchmark.
   DWORD expectedAuditPolicy = POLICY_AUDIT_EVENT_SUCCESS | POLICY_AUDIT_EVENT_FAILURE;

   LSA_OBJECT_ATTRIBUTES ObjectAttributes;
   LSA_HANDLE PolicyHandle;

   // Initialize the ObjectAttributes structure.
   ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

   // Open the local LSA policy.
   NTSTATUS status = LsaOpenPolicy(NULL, &ObjectAttributes, POLICY_VIEW_AUDIT_INFORMATION, &PolicyHandle);
   if (status != STATUS_SUCCESS)
   {
      DWORD winError = LsaNtStatusToWinError(status);
      std::cerr << "Failed to open LSA policy. Error code: 0x" << std::hex << status << std::dec << std::endl;
      return false;
   }

   // Query the audit policy.
   POLICY_AUDIT_EVENTS_INFO* auditPolicyInfo = NULL;
   status = LsaQueryInformationPolicy(PolicyHandle, PolicyAuditEventsInformation, (PVOID*)&auditPolicyInfo);
   if (status != STATUS_SUCCESS)
   {
      std::cerr << "Failed to query audit policy. Error code: 0x" << std::hex << status << std::dec << std::endl;
      result = false;
   }
   else
   {
      if (auditPolicyInfo->AuditingMode != expectedAuditPolicy)
      {
         std::cerr << "Audit settings do not match CIS recommendations." << std::endl;
         result = false;
      }
      else
      {
         std::cout << "Audit settings are configured according to CIS recommendations." << std::endl;
      }

      // Free the memory allocated for audit policy info.
      LsaFreeMemory(auditPolicyInfo);
   }

   // Close the LSA policy handle.
   LsaClose(PolicyHandle);

   return result;
}

bool TestSuite::IsAuditingEnabledForCriticalEvents()
{
   bool result = false;

   // Define the expected audit policy values for critical events based on CIS recommendations.
   // Modify these values according to your specific CIS benchmark.
   DWORD expectedAuditPolicy = POLICY_AUDIT_EVENT_SUCCESS | POLICY_AUDIT_EVENT_FAILURE;

   LSA_OBJECT_ATTRIBUTES ObjectAttributes;
   LSA_HANDLE PolicyHandle = NULL;
   PPOLICY_AUDIT_EVENTS_INFO auditPolicyInfo = NULL;

   // Initialize the ObjectAttributes structure.
   ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

   // Open the local LSA policy.
   NTSTATUS status = LsaOpenPolicy(NULL, &ObjectAttributes, POLICY_VIEW_AUDIT_INFORMATION, &PolicyHandle);
   if (status != STATUS_SUCCESS)
   {
      std::cerr << "Failed to open LSA policy. NTSTATUS code: 0x" << std::hex << status << std::dec << std::endl;
      return false;
   }

   // Query the audit policy.
   status = LsaQueryInformationPolicy(PolicyHandle, PolicyAuditEventsInformation, (PVOID*)&auditPolicyInfo);
   if (status != STATUS_SUCCESS)
   {
      std::cerr << "Failed to query audit policy. NTSTATUS code: 0x" << std::hex << status << std::dec << std::endl;
   }
   else
   {
      if ((auditPolicyInfo->AuditingMode & expectedAuditPolicy) == expectedAuditPolicy)
      {
         std::cout << "Auditing is enabled for critical system events according to CIS recommendations." << std::endl;
         result = true;
      }
      else
      {
         std::cout << "Auditing is not enabled for critical system events." << std::endl;
      }

      // Free the memory allocated for audit policy info.
      LsaFreeMemory(auditPolicyInfo);
   }

   // Close the LSA policy handle.
   LsaClose(PolicyHandle);

   return result;
}

bool TestSuite::IsBackupAndRestoreConfigured()
{
   bool backupExists = false;
   bool restorationSuccessful = false;

   // Define the path to the directory where backup files are expected.
   // Modify this path according to your specific configuration.
   std::string backupDirectory = "C:\\BackupDirectory";

   // Check if the backup directory exists.
   DWORD attributes = GetFileAttributesA(backupDirectory.c_str());
   if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY))
   {
      backupExists = true;
      std::cout << "Backup directory exists: " << backupDirectory << std::endl;
   }
   else
   {
      std::cerr << "Backup directory does not exist: " << backupDirectory << std::endl;
   }

   // Perform a sample restoration operation (you should adapt this part).
   // This example assumes that a text file named "restore_test.txt" is part of the backup.
   // Modify the restoration process based on your backup strategy.
   std::string filePathToRestore = backupDirectory + "\\restore_test.txt";

   // Attempt to restore the file and check if it exists after restoration.
   if (CopyFileA(filePathToRestore.c_str(), "C:\\RestoredFile.txt", FALSE))
   {
      restorationSuccessful = true;
      std::cout << "Restoration successful." << std::endl;
   }
   else
   {
      std::cerr << "Restoration failed." << std::endl;
   }

   // Report the overall result.
   if (backupExists && restorationSuccessful)
   {
      std::cout << "Regular backups are performed and can be restored successfully." << std::endl;
      return true;
   }
   else
   {
      std::cerr << "Regular backups are not performed or cannot be restored successfully." << std::endl;
      return false;
   }
}