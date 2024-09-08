// QT includes
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QStandardItemModel>

#include <iostream>
#include <fstream>

// Header files
#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Set the fixed size to the default size
    this->setFixedSize(this->size());
    // Disable the size grip
    this->statusBar()->setSizeGripEnabled(false);

    // Set the table view to expand
    ui->readingList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Load data into the table view
    loadData();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadData()
{
    QFile reading_list("reading_list.csv");
    if (!reading_list.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open file for reading:" << reading_list.errorString();
        return;
    }

    QStandardItemModel *model = new QStandardItemModel(this);
    QTextStream in(&reading_list);

    // Read the header line
    if (!in.atEnd()) {
        QString headerLine = in.readLine();
        QStringList headers = headerLine.split(",");
        model->setHorizontalHeaderLabels(headers);
    }

    // Read the data lines
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(",");
        QList<QStandardItem *> items;
        for (const QString &field : fields) {
            QStandardItem *item = new QStandardItem(field);
            items.append(new QStandardItem(field));
        }
        model->appendRow(items);
    }

    ui->readingList->setModel(model);
    reading_list.close();
}


void MainWindow::on_addBookButton_clicked()
{
    QString title = ui->titleField->text();
    QString author = ui->authorField->text();
    QString publisher = ui->publisherField->text();
    QString isbn = ui->isbnField->text();

    if (title.isEmpty() || author.isEmpty() || publisher.isEmpty() || isbn.isEmpty()) {
        QMessageBox::information(nullptr, "Error", "Please fill in all fields!");
    } else {
        QFile reading_list("reading_list.csv");

        qDebug() << "Writing to file";

        bool fileExists = reading_list.exists();

        if (reading_list.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&reading_list);

            if (!fileExists) {
                out << "Title,Author,Publisher,ISBN" << Qt::endl;
            }

            out << title << "," << author << "," << publisher << "," << isbn << Qt::endl;
            reading_list.close();

            ui->titleField->clear();
            ui->authorField->clear();
            ui->publisherField->clear();
            ui->isbnField->clear();

            QMessageBox::information(nullptr, "Success", "Book added successfully!");

            loadData();
        } else {
            qWarning() << "Could not open file for writing:" << reading_list.errorString();
        }
    }
}

void MainWindow::on_deleteButton_clicked()
{
    QItemSelectionModel *selectionModel = ui->readingList->selectionModel();
    QModelIndexList selectedRows = selectionModel->selectedRows();

    if (selectedRows.isEmpty()) {
        QMessageBox::information(this, "Error", "Please select a book to remove!");
        return;
    }

    // Assuming single selection mode
    QModelIndex selectedIndex = selectedRows.first();
    int row = selectedIndex.row();

    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(ui->readingList->model());
    if (model) {
        model->removeRow(row);
        updateCSVFile(model);
    }
}

void MainWindow::updateCSVFile(QStandardItemModel *model)
{
    QFile reading_list("reading_list.csv");
    if (!reading_list.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Could not open file for writing:" << reading_list.errorString();
        return;
    }

    QTextStream out(&reading_list);
    QStringList headers;
    for (int column = 0; column < model->columnCount(); ++column) {
        headers << model->horizontalHeaderItem(column)->text();
    }
    out << headers.join(",") << Qt::endl;

    for (int row = 0; row < model->rowCount(); ++row) {
        QStringList rowData;
        for (int column = 0; column < model->columnCount(); ++column) {
            rowData << model->item(row, column)->text();
        }
        out << rowData.join(",") << Qt::endl;
    }

    reading_list.close();
}
