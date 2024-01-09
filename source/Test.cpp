#include "Test.h"

Test::Test(std::string name, std::string description)
   : name{name}, description{description}
{
   result = Result::Unknown;
}

Test::~Test()
{
   this->Clear();
}

Test::Result Test::GetResult()
{
   return this->result;
}

std::string Test::GetName()
{
   return this->name;
}

std::string Test::GetDescription()
{
   return this->description;
}

void Test::Clear()
{  
   this->result = Result::Failed;
}
