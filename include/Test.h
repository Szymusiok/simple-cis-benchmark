#pragma once

#include <string>

class Test
{
public:
   Test(std::string name, std::string description);
   ~Test();

   bool IsPassed();
   std::string GetName();
   void Clear();


private:
   std::string name;
   std::string description;
   bool passed;
};

