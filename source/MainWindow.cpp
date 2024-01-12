#include "MainWindow.h"

MainWindow::MainWindow(QWidget* parent)
   : QMainWindow(parent)
{
   ui.setupUi(this);

   connect(this->ui.startButton, &QPushButton::clicked, this, [this] ()
      {
         StartTest();
      });

   this->ManageTest();
   this->BuildTestTree();
}

MainWindow::~MainWindow()
{
   for (std::vector<Test*> tests : this->allTests)
   {
      for (Test* test : tests)
      {
         delete test;
      }
   }
}

void MainWindow::BuildTestTree()
{
   this->ui.testTree->setColumnCount(2);
   this->ui.testTree->setHeaderLabels({ "Nazwa","Wynik" });
   this->ui.testTree->header()->setDefaultSectionSize(400);

   QFont font;
   font.setPointSize(13);
   this->ui.testTree->header()->setFont(font);

   std::vector < std::string> testNames{"Haslo","Firewall","Rejestr","Serwisy","Aktualizacja","Dostep","Dysk"};
   int points{}, maxPoints{};

   for (int i = 0; i < this->allTests.size(); i++)
   {
      QTreeWidgetItem* mainItem = new QTreeWidgetItem();
      mainItem->setText(0,testNames.at(i).c_str());

      font.setPointSize(12);
      mainItem->setFont(0, font);
      mainItem->setFont(1, font);

      for (Test* test : this->allTests.at(i))
      {
         QTreeWidgetItem* item = new QTreeWidgetItem();
         item->setText(0, test->GetName().c_str());
         if (test->GetResult() == Test::Result::Success)
         {
            item->setText(1, "Success");
            item->setTextColor(1, QColor(0, 128, 0));
            points++;
         }
         else if (test->GetResult() == Test::Result::Failed)
         {
            item->setText(1, "Failed");
            item->setTextColor(1, QColor(255, 0, 0));
         }
         else
         {
            item->setText(1, "-");
         }
         maxPoints++;

         item->setToolTip(0,test->GetDescription().c_str());

         font.setPointSize(10);
         item->setFont(0, font);
         item->setFont(1, font);
         
         mainItem->addChild(item);
      }
      mainItem->setText(1, std::string(std::to_string(points)+"/"+std::to_string(maxPoints)).c_str());
      points = 0, maxPoints = 0;;
      this->ui.testTree->addTopLevelItem(mainItem);
   }
}

void MainWindow::ManageTest()
{
   // Password
   Test* password1 = new Test("Minimalna dlugosc hasla", "Sprawdzenie minimalnej dlugosci hasla w systemie");
   this->passwordTests.push_back(password1);
   Test* password3 = new Test("Wygasniecie hasla", "Sprawdzenie czasu wygasniecia hasla");
   this->passwordTests.push_back(password3);

   // Firewall
   Test* firewall3 = new Test("Stan", "Pobranie informacji o aktualnym stanie zapory sieciowej (wlaczona/wylaczona).");
   this->firewallTests.push_back(firewall3);

   // Events
   Test* events1 = new Test("Zapis do rejestru", "Zapisywanie okreslonych zdarzen do rejestru systemowego, np. logowanie do systemu, proby dostepu do chronionych plikow itp.");
   this->eventsTests.push_back(events1);

   // Services
   Test* services1 = new Test("Zarzadzanie uslugami", "Startowanie, zatrzymywanie lub restartowanie okreslonych uslug systemowych.");
   this->servicesTests.push_back(services1);
   Test* services3 = new Test("Autostart", "Ustawienie uprawnien automatycznego startu/uslugi na zadanie.");
   this->servicesTests.push_back(services3);

   // Update
   Test* update2 = new Test("Konfiguracja", "Sprawdzenie, czy system jest skonfigurowany do automatycznego pobierania i instalowania aktualizacji.");
   this->updateTests.push_back(update2);

   // Access
   Test* access1 = new Test("Dostep", "Sprawdzenie czy uzytkownik ma dostep do okreslonych plikow lub folderow. W tym przypadku caly folder Windows");
   this->accessTests.push_back(access1);
   Test* access2 = new Test("Zmiana uprawnien", "Sprawdzenie czy uzytkownik ma dostep do zmiany uprawnien dostepu do wybranych plikow/folderow dla okreslonych uzytkownikow.");
   this->accessTests.push_back(access2);

   // Disc
   Test* disc1 = new Test("Szyfrowanie", "Sprawdzenie czy dysk jest szyfrowany za pomoca BitLocker'a");
   this->discTest.push_back(disc1);

   this->allTests.push_back(passwordTests);
   this->allTests.push_back(firewallTests);
   this->allTests.push_back(eventsTests);
   this->allTests.push_back(servicesTests);
   this->allTests.push_back(updateTests);
   this->allTests.push_back(accessTests);
   this->allTests.push_back(discTest);
}

void MainWindow::ResetTestTree()
{
   for (std::vector<Test*> tests : this->allTests)
   {
      for (Test* test : tests)
      {
         test->Clear();
      }
   }
   this->ui.testTree->clear();
}

void MainWindow::SetResultText()
{
   this->ui.resultText->clear();

   QFont font;
   font.setPointSize(16);
   this->ui.resultText->setFont(font);

   std::string resultText{"Points: "};
   int points{}, maxPoints{};

   for (std::vector<Test*> tests : this->allTests)
   {
      for (Test* test : tests)
      {  
         if (test->GetResult() == Test::Result::Success)
         {
            points++;
         }
         maxPoints++;
      }
   }
   float percent = static_cast<float>(points) / static_cast<float>(maxPoints) * 100.0f;

   if (percent > 80)
   {
      this->ui.resultText->setStyleSheet("background-color: rgb(0, 255, 0);");
   }
   else
   {
      this->ui.resultText->setStyleSheet("background-color: rgb(255, 0, 0);");
   }

   resultText += std::to_string(points) + "/" + std::to_string(maxPoints);
   resultText += "\nPercent: ";

   this->ui.resultText->append(resultText.c_str());

   resultText = std::to_string(percent);
   QString result = QString::number(percent, 'f', 2).arg('%');
   this->ui.resultText->insertPlainText(result);
}

void MainWindow::StartTest()
{
   this->ResetTestTree();

   // TESTY NA SUCHO
   bool overall = true;

   //HASLO 1
   this->allTests[0][0]->GetMinimumPasswordLength();

   //HASLO 2
   this->allTests[0][1]->GetPasswordExpiredTime();

   // FIREWALL 1
   this->allTests[0][0]->CheckFirewall();

   // REGISTER 1
   this->allTests[0][0]->CheckRegistryWrite();

   // SERVICES 1
   this->allTests[0][0]->CheckServices();

   // SERVICES 2
   this->allTests[0][0]->CheckServiceTriggerStartPermissions();

   // UPDATE 1
   this->allTests[0][0]->CheckIfUpdateIsAutomat();

   // ACCESS
   this->allTests[0][0]->CheckFolderAccess();

   // ACCESS 2
   this->allTests[0][0]->CheckFolderPermissions();




   // KONIEC TESTOW NA SUCHO

   this->SetResultText();
   this->BuildTestTree();

   this->ui.testTree->expandAll();
}
