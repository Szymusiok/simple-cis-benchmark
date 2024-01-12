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