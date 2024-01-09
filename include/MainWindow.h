#pragma once

#include <QMainWindow>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QList>
#include "ui_MainWindow.h"
#include "Test.h"
#include <vector>

class MainWindow : public QMainWindow
{
   Q_OBJECT

public:
   explicit MainWindow(QWidget* parent = nullptr);
   ~MainWindow();

   //dzialanie
   void FillTestTree();
   void ManageTest();
   void ResetTests();

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

