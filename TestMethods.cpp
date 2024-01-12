#include "Test.h"
#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <regex>

size_t Test::GetMinimumPasswordLength()
{
   std::array<char, 256> buffer{};
   std::string result;
   std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen("net accounts", "r"), _pclose);
   if (!pipe)
   {
      std::cerr << "Nie mo¿na uruchomiæ polecenia 'net accounts'." << std::endl;
      return 0;
   }

   while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
   {
      result += buffer.data();
   }

   std::regex regex("Minimalna.*?(\\d+)");
   std::smatch match;
   if (std::regex_search(result, match, regex) && match.size() > 1)
   {
      return std::stoul(match.str(1));
   }

   return 0;
}