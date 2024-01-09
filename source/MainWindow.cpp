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
   font.setPointSize(12);
   this->ui.testTree->header()->setFont(font);

   std::vector < std::string> testNames{"Haslo","Konto","Firewall","Rejestr","Serwisy","Aktualizacja","Dostep"};
   int passed = 0;

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
            passed++;
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

         item->setToolTip(0,test->GetDescription().c_str());

         font.setPointSize(10);
         item->setFont(0, font);
         item->setFont(1, font);
         
         mainItem->addChild(item);
      }
      mainItem->setText(1, std::string(std::to_string(passed)+"/3").c_str());
      passed = 0;
      this->ui.testTree->addTopLevelItem(mainItem);
   }
}

void MainWindow::ManageTest()
{
   // Password
   Test* password1 = new Test("Minimalna dlugosc hasla", "Sprawdzenie minimalnej dlugosci hasla w systemie");
   this->passwordTests.push_back(password1);
   Test* password2 = new Test("Typy znakow", "Weryfikacja, czy wymagane sa okreslone typy znakow (np. duze litery, cyfry, znaki specjalne)");
   this->passwordTests.push_back(password2);
   Test* password3 = new Test("Wygasniecie hasla", "Sprawdzenie czasu wygasniecia hasla");
   this->passwordTests.push_back(password3);

   // Accounts
   Test* accounts1 = new Test("Konta uzytkownika", "Pobranie listy istniejacych kont uzytkownikow i ich typow (administrator, uzytkownik standardowy).");
   this->accountTests.push_back(accounts1);
   Test* accounts2 = new Test("Uprawnienia", "Zmiana uprawnien dla okreslonych kont uzytkownikow.");
   this->accountTests.push_back(accounts2);
   Test* accounts3 = new Test("Data logowania", "Sprawdzenie daty ostatniego logowania dla kont uzytkownikow.");
   this->accountTests.push_back(accounts3);

   // Firewall
   Test* firewall1 = new Test("Porty", "Sprawdzenie aktualnych regul zapory sieciowej, aby sprawdzic, czy okreslone porty sa zablokowane lub otwarte.");
   this->firewallTests.push_back(firewall1);
   Test* firewall2 = new Test("Reguly", "Dodanie lub usuwanie regul w zaporze sieciowej w zaleznosci od konfiguracji.");
   this->firewallTests.push_back(firewall2);
   Test* firewall3 = new Test("Stan", "Pobranie informacji o aktualnym stanie zapory sieciowej (wlaczona/wylaczona).");
   this->firewallTests.push_back(firewall3);

   // Events
   Test* events1 = new Test("Zapis do rejestru", "Zapisywanie okreslonych zdarzen do rejestru systemowego, np. logowanie do systemu, proby dostepu do chronionych plikow itp.");
   this->eventsTests.push_back(events1);
   Test* events2 = new Test("Odczyt z rejestru", "Odczytanie i analiza zdarzen w rejestrze systemowym w celu wykrycia okreslonych aktywnosci.");
   this->eventsTests.push_back(events2);
   Test* events3 = new Test("Reakcje", "Ustawienie powiadomien lub reakcji na okreslone zdarzenia systemowe.");
   this->eventsTests.push_back(events3);

   // Services
   Test* services1 = new Test("Zarzadzanie uslugami", "Startowanie, zatrzymywanie lub restartowanie okreslonych uslug systemowych.");
   this->servicesTests.push_back(services1);
   Test* services2 = new Test("Dzialanie uslug", "Sprawdzenie stanu dzialania wybranych uslug systemowych.");
   this->servicesTests.push_back(services2);
   Test* services3 = new Test("Autostart", "Ustawienie automatycznego startu/uslugi na zadanie.");
   this->servicesTests.push_back(services3);

   // Update
   Test* update1 = new Test("Ostatnie aktualizacje", "Pobranie informacji o statusie ostatnich aktualizacji systemowych.");
   this->updateTests.push_back(update1);
   Test* update2 = new Test("Konfiguracja", "Sprawdzenie, czy system jest skonfigurowany do automatycznego pobierania i instalowania aktualizacji.");
   this->updateTests.push_back(update2);
   Test* update3 = new Test("Plan", "Zmiana konfiguracji planu aktualizacji (np. harmonogram instalacji aktualizacji).");
   this->updateTests.push_back(update3);

   // Access
   Test* access1 = new Test("Dostep", "Sprawdzenie listy uzytkownikow lub grup, ktore maja dostep do okreslonych plikow lub folderow.");
   this->accessTests.push_back(access1);
   Test* access2 = new Test("Zmiana uprawnien", "Zmiana uprawnien dostepu do wybranych plikow/folderow dla okreslonych uzytkownikow.");
   this->accessTests.push_back(access2);
   Test* access3 = new Test("ACL", "Sprawdzenie konfiguracji ACL (Access Control List) dla wybranych zasobow systemowych.");
   this->accessTests.push_back(access3);

   this->allTests.push_back(passwordTests);
   this->allTests.push_back(accountTests);
   this->allTests.push_back(firewallTests);
   this->allTests.push_back(eventsTests);
   this->allTests.push_back(servicesTests);
   this->allTests.push_back(updateTests);
   this->allTests.push_back(accessTests);
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

void MainWindow::StartTest()
{
   this->ResetTestTree();
   this->BuildTestTree();
}
