#include "Test.h"

Test::Test(std::string name, std::string description, std::function<bool()> test)
   : name{name}, description{description}, test(test)
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
   this->result = Result::Unknown;
}

void Test::Run()
{
   this->test() ? this->result = Result::Success : this->result = Result::Failed;
}
