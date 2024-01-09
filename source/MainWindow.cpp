#include "MainWindow.h"

MainWindow::MainWindow(QWidget* parent)
   : QMainWindow(parent)
{
   ui.setupUi(this);

   connect(this->ui.startButton, &QPushButton::clicked, this, [this] ()
      {
         StartTest();
      });

   this->FillTestTree();
}

MainWindow::~MainWindow()
{
}

void MainWindow::FillTestTree()
{
   this->ManageTest();

   this->ui.testTree->setColumnCount(2);
   this->ui.testTree->setHeaderLabels({ "Name","Result" });

   std::vector < std::string> testNames{"Password","Account","Firewall","Events","Services","Update","Access"};

   for (int i = 0; i < this->allTests.size(); i++)
   {
      QTreeWidgetItem* mainItem = new QTreeWidgetItem();
      mainItem->setText(0,testNames.at(i).c_str());

      for (Test* test : this->allTests.at(i))
      {
         QTreeWidgetItem* item = new QTreeWidgetItem();
         item->setText(0, test->GetName().c_str());
         item->setText(1, "-");
         mainItem->addChild(item);
      }

      this->ui.testTree->addTopLevelItem(mainItem);
   }
}

void MainWindow::ManageTest()
{
   // Password
   Test* test1 = new Test("Haslo", "Przykladowy test");
   this->passwordTests.push_back(test1);

   // Accounts
   Test* test2 = new Test("Accounts", "2222Przykladowy test");
   this->accountTests.push_back(test2);

   // Firewall
   Test* test3 = new Test("Firewall", "3333Przykladowy test");
   this->firewallTests.push_back(test3);

   // Events
   Test* test4 = new Test("Events", "3444444333Przykladowy test");
   this->eventsTests.push_back(test4);

   // Services
   Test* test5 = new Test("Services", "2222Przykladowy test");
   this->servicesTests.push_back(test5);

   // Update
   Test* test6 = new Test("Update", "2222Przykladowy test");
   this->updateTests.push_back(test6);

   // Access
   Test* test7 = new Test("Access", "2222Przykladowy test");
   this->accessTests.push_back(test7);

   this->allTests.push_back(passwordTests);
   this->allTests.push_back(accountTests);
   this->allTests.push_back(firewallTests);
   this->allTests.push_back(eventsTests);
   this->allTests.push_back(servicesTests);
   this->allTests.push_back(updateTests);
   this->allTests.push_back(accessTests);
}

void MainWindow::ResetTests()
{
   for (std::vector<Test*> tests : this->allTests)
   {
      for (Test* test : tests)
      {
         test->Clear();
      }
   }
   this->allTests.clear();
}

void MainWindow::StartTest()
{
   this->ResetTests();
}
