#pragma once

#include "TestSuite.h"
#include <string>
#include <functional>

class Test
{
public:
   enum class Result
   {
      Failed = 0,
      Success,
      Unknown
   };

   Test(std::string name, std::string description, std::function<bool()> test);
   ~Test();

   Result GetResult();
   std::string GetName();
   std::string GetDescription();
   void Clear();
   void Run();


private:
   std::string name;
   std::string description;
   Result result;
   std::function<bool()> test;
};

