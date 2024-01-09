#include "Test.h"

Test::Test(std::string name, std::string description)
   : name{name}, description{description},passed{false}
{
}

Test::~Test()
{
   this->Clear();
}

bool Test::IsPassed()
{
   return this->passed;
}

std::string Test::GetName()
{
   return this->name;
}

void Test::Clear()
{
   this->name.clear();
   this->passed = false;
}
