#include "MainWindow.h"
#include "DataManager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
}

MainWindow::~MainWindow()
{}


void MainWindow::on_btn_Open_clicked()
{
}

void MainWindow::on_btn_New_clicked()
{
    std::string DeviceId = ui.lin_Open->text().toStdString();
    

	Device* device = DataManager::Get().AddDevice(DeviceId, "", "");

    // load impedances


    // graph EIS

	// graph CV

    // graph CIL
}

