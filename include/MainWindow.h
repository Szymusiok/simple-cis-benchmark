#pragma once

#include <QMainWindow>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QList>
#include <QFont>
#include "ui_MainWindow.h"
#include "Test.h"
#include <vector>
#include <windows.h>

class MainWindow : public QMainWindow
{
   Q_OBJECT

public:
   explicit MainWindow(QWidget* parent = nullptr);
   ~MainWindow();

   //dzialanie
   void BuildTestTree();
   void ResetTestTree();
   void SetResultText();
   void ManageTest();

   void StartTest();
private:
   Ui::MainWindow ui;

   std::vector<Test*> passwordTests;
   std::vector<Test*> accountTests;
   std::vector<Test*> firewallTests;
   std::vector<Test*> eventsTests;
   std::vector<Test*> servicesTests;
   std::vector<Test*> updateTests;
   std::vector<Test*> accessTests;

   std::vector<std::vector<Test* >> allTests;


};

