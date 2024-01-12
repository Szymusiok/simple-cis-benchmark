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


private:
   std::string name;
   std::string description;
   Result result;
};

