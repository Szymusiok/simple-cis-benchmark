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

   if (!GetUserName(username, &usernameLen))
   {
      return false;
   }

   USER_INFO_3* userInfo = nullptr;
   NET_API_STATUS status;

   status = NetUserGetInfo(nullptr, username, 3, reinterpret_cast<LPBYTE*>(&userInfo));
   if (status != NERR_Success)
   {
      return false;
   }

   if (userInfo->usri3_password_expired)
   {
      return false;
   }

   DWORD passwordAge = userInfo->usri3_password_age;

   DWORD currentTime = GetTickCount(); // Aktualny czas w milisekundach.
   if (passwordAge <= currentTime)
   {
      return false;
   }

   return true;
}

bool TestSuite::CheckFirewall()
{
   bool rV = true;
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

   HKEY hKey;
   LONG result = RegCreateKeyEx(HKEY_LOCAL_MACHINE, registryKey, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr);

   if (result == ERROR_SUCCESS)
   {
      result = RegSetValueEx(hKey, registryValue, 0, REG_EXPAND_SZ, (const BYTE*)registryValueData, (wcslen(registryValueData) + 1) * sizeof(wchar_t));

      if (result == ERROR_SUCCESS)
      {
         rV = false;
      }
      else
      {
         rV = true;
      }

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
      return false;
   }

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
         continue;
      }

      SERVICE_STATUS status;
      if (QueryServiceStatus(service, &status))
      {
         if (status.dwControlsAccepted != 0)
         {
            hasPermission = true;
         }
         else
         {
            hasPermission = false;
         }
      }
      else
      {
         return false;
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
   const wchar_t* serviceName = L"wuauserv"; // Us³uga Windows Update

   SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
   if (!scm)
   {
      return false;
   }

   SC_HANDLE service = OpenService(scm, serviceName, SERVICE_CHANGE_CONFIG);
   if (!service)
   {
      CloseServiceHandle(scm);
      return false;
   }

   SERVICE_DESCRIPTION serviceDescription;
   if (ChangeServiceConfig2(service, SERVICE_CONFIG_DESCRIPTION, &serviceDescription))
   {
      CloseServiceHandle(service);
      CloseServiceHandle(scm);
      return true;
   }
   else
   {
      return false;
   }

   CloseServiceHandle(service);
   CloseServiceHandle(scm);
   return false;
}

bool TestSuite::CheckIfUpdateIsAutomat()
{
   const wchar_t* serviceName = L"wuauserv"; // Us³uga Windows Update

   SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
   if (!scm)
   {
      return false;
   }

   SC_HANDLE service = OpenService(scm, serviceName, SERVICE_QUERY_CONFIG);
   if (!service)
   {
      CloseServiceHandle(scm);
      return false;
   }

   DWORD bytesNeeded;
   QUERY_SERVICE_CONFIG* serviceConfig = nullptr;
   if (!QueryServiceConfig(service, nullptr, 0, &bytesNeeded) && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
   {
      serviceConfig = static_cast<QUERY_SERVICE_CONFIG*>(malloc(bytesNeeded));
      if (QueryServiceConfig(service, serviceConfig, bytesNeeded, &bytesNeeded))
      {
         if (serviceConfig->dwStartType == SERVICE_AUTO_START)
         {
            free(serviceConfig);
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            return true;
         }
         else
         {
            return false;
         }
      }
      else
      {
         return false;
      }
   }
   else
   {
      return false;
   }

   free(serviceConfig);
   CloseServiceHandle(service);
   CloseServiceHandle(scm);
   return false;
}

bool TestSuite::CheckFolderAccess()
{
   const wchar_t* folderPath = L"C:\\Windows";

   if (GetFileAttributes(folderPath) != INVALID_FILE_ATTRIBUTES)
   {
      return true;
   }
   else
   {
      return false;
   }
}

bool TestSuite::CheckFolderPermissions()
{
   const wchar_t* folderPath = L"C:\\Windows";

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
            return true;
         }
      }
   }

   if (pNewDACL) LocalFree(pNewDACL);
   if (pSD) LocalFree(pSD);

   return false;
}

bool TestSuite::CheckBitLocker()
{
   DWORD dwVolumeStatus;
   bool result = false;

   if (GetVolumeInformation(L"C:\\", NULL, 0, NULL, NULL, &dwVolumeStatus, NULL, 0))
   {
      if (dwVolumeStatus & FILE_SUPPORTS_ENCRYPTION)
      {
         result = true;
      }
      else
      {
         result = false;
      }
   }
   else
   {
      result = false;
   }

   return result;
}

bool TestSuite::IsAuditConfigured()
{
   bool result = true;

   DWORD expectedAuditPolicy = POLICY_AUDIT_EVENT_SUCCESS | POLICY_AUDIT_EVENT_FAILURE;

   LSA_OBJECT_ATTRIBUTES ObjectAttributes;
   LSA_HANDLE PolicyHandle;

   ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

   NTSTATUS status = LsaOpenPolicy(NULL, &ObjectAttributes, POLICY_VIEW_AUDIT_INFORMATION, &PolicyHandle);
   if (status != STATUS_SUCCESS)
   {
      DWORD winError = LsaNtStatusToWinError(status);
      return false;
   }

   POLICY_AUDIT_EVENTS_INFO* auditPolicyInfo = NULL;
   status = LsaQueryInformationPolicy(PolicyHandle, PolicyAuditEventsInformation, (PVOID*)&auditPolicyInfo);
   if (status != STATUS_SUCCESS)
   {
      result = false;
   }
   else
   {
      if (auditPolicyInfo->AuditingMode != expectedAuditPolicy)
      {
         result = false;
      }
      else
      {
         result = true;
      }

      LsaFreeMemory(auditPolicyInfo);
   }

   LsaClose(PolicyHandle);

   return result;
}

bool TestSuite::IsAuditingEnabledForCriticalEvents()
{
   bool result = false;

   DWORD expectedAuditPolicy = POLICY_AUDIT_EVENT_SUCCESS | POLICY_AUDIT_EVENT_FAILURE;

   LSA_OBJECT_ATTRIBUTES ObjectAttributes;
   LSA_HANDLE PolicyHandle = NULL;
   PPOLICY_AUDIT_EVENTS_INFO auditPolicyInfo = NULL;

   ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

   NTSTATUS status = LsaOpenPolicy(NULL, &ObjectAttributes, POLICY_VIEW_AUDIT_INFORMATION, &PolicyHandle);
   if (status != STATUS_SUCCESS)
   {
      return false;
   }

   status = LsaQueryInformationPolicy(PolicyHandle, PolicyAuditEventsInformation, (PVOID*)&auditPolicyInfo);
   if (status != STATUS_SUCCESS)
   {
      return false;
   }
   else
   {
      if ((auditPolicyInfo->AuditingMode & expectedAuditPolicy) == expectedAuditPolicy)
      {
         result = true;
      }
      else
      {
         result = false;
      }

      LsaFreeMemory(auditPolicyInfo);
   }

   LsaClose(PolicyHandle);

   return result;
}

bool TestSuite::IsBackupAndRestoreConfigured()
{
   bool backupExists = false;
   bool restorationSuccessful = false;

   std::string backupDirectory = "C:\\BackupDirectory";

   DWORD attributes = GetFileAttributesA(backupDirectory.c_str());
   if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY))
   {
      backupExists = true;
   }
   else
   {
      backupExists = false;
   }
   std::string filePathToRestore = backupDirectory + "\\restore_test.txt";

   if (CopyFileA(filePathToRestore.c_str(), "C:\\RestoredFile.txt", FALSE))
   {
      restorationSuccessful = true;
   }
   else
   {
      backupExists = false;
   }

   if (backupExists && restorationSuccessful)
   {
      return true;
   }
   else
   {
      return false;
   }
}