#include "include/mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    ui->plainTextEdit->setFont(font);
    this->readSettings();
    QGuiApplication::setWindowIcon(QIcon(QString(" ")));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "closeEvent()";
    this->on_actionExit_triggered();

    if (event != NULL) {
        event->accept();
        //QMainWindow::closeEvent(event);
    }
}

void MainWindow::saveAllState()
{
    qDebug() << "saveAllState()";
    QSettings settings("Editor", "EditorSimples");
    settings.setValue("mainwindow/geometry", saveGeometry());
    settings.setValue("mainwindow/windowState", saveState());

    QFont font = ui->plainTextEdit->font();
    settings.setValue("mainwindow/fontFamily", font.family());
    settings.setValue("mainwindow/fontSize", font.pointSize());
}

void MainWindow::readSettings()
{
    QSettings settings("Editor", "EditorSimples");
    restoreGeometry(settings.value("mainwindow/geometry").toByteArray());
    restoreState(settings.value("mainwindow/windowState").toByteArray());

    if (settings.contains("mainwindow/fontFamily")) {
        QFont font = QFont(settings.value("mainwindow/fontFamily").toString(),
                           settings.value("mainwindow/fontSize").toInt());
        ui->plainTextEdit->setFont(font);
    }

}

/**
 * @brief MainWindow::reset
 * Resets the window (clear text, reset scroll, properties, etc.)
 */
void MainWindow::reset()
{
    ui->plainTextEdit->clear();
    ui->plainTextEdit->scroll(0, 0);
    ui->plainTextEdit->setProperty("filename", QVariant());
    ui->plainTextEdit->setProperty("modified", QVariant(false));

    this->connect(ui->plainTextEdit, SIGNAL(textChanged()), this, SLOT(on_plainTextEdit_textChanged()), Qt::UniqueConnection);
    this->setWindowTitle(QString(PROGRAM_NAME));
    ui->actionSave->setEnabled(false);
}

/**
 * @brief MainWindow::on_actionNew_triggered
 * File -- New button clicked.
 */
void MainWindow::on_actionNew_triggered()
{
    qDebug() << "on_actionNew_triggered";
    if (this->is_file_modified()) {
        QString filename;
        switch(this->ask_to_save()) {
            case QMessageBox::Yes:
                // Yes, we want to save it.
                filename = this->get_filename();
                if (filename.isNull()) {
                    filename = this->ask_for_filename();
                }
                this->do_save();
                this->reset();
                break;

            case QMessageBox::No:
                this->reset();
                break;

            case QMessageBox::Cancel:
                // Return from here, abort the 'new' operation.
                break;
        }
    } else {
        // OK to just reset, since the file hasn't been modified.
        this->reset();
    }
}

/**
 * @brief MainWindow::is_file_modified
 * Has the currently open file been modified?
 * @return true iff it has been modified.
 */
bool MainWindow::is_file_modified()
{
    QVariant is_modified = ui->plainTextEdit->property("modified");
    return (!is_modified.isNull() && is_modified.toBool() == true);
}

/**
 * @brief MainWindow::getFilename
 * Gets the name of the open file.
 * @return Name of the file, or QString() if not available.
 */
QString MainWindow::get_filename()
{
    QVariant result = ui->plainTextEdit->property("filename");
    QString filename;

    if (result.isNull()) {
        filename = QString();
    } else {
        filename = result.toString();
    }
    return filename;
}

/**
 * @brief MainWindow::ask_to_save
 * Ask the user if they want to save the file.
 * @return
 */
int MainWindow::ask_to_save()
{
    QMessageBox msgBox;
    msgBox.setText(tr("Este documento foi modificado."));
    msgBox.setInformativeText(tr("Você deseja salvar as mudanças?"));

    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Yes);
    return msgBox.exec();
}

QString MainWindow::ask_for_filename()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString filename = QFileDialog::getSaveFileName(this,
                                tr("Salvar Arquivo"),
                                NULL,
                                tr("Todos os arquivos (*);;Arquivo de texto (*.txt)"),
                                &selectedFilter,
                                options);
    ui->plainTextEdit->setProperty("filename", filename);
    return filename;
}

/**
 * @brief MainWindow::do_save
 * Perform the actual save operation (file system), using the filename
 * associated with the object.
 *
 * @return true iff the operation is successful.
 */
bool MainWindow::do_save()
{
    qDebug() << "do_save()";

    // Gather the filename
    QVariant filename = ui->plainTextEdit->property("filename");
    Q_ASSERT_X(!filename.isNull(), "do_save", "filename was null");

    // Open the file
    QFile file(filename.toString());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    // Write to the file
    QTextStream out(&file);
    out << ui->plainTextEdit->document()->toPlainText();
    out.flush();
    file.close();

    this->connect(ui->plainTextEdit, SIGNAL(textChanged()), this, SLOT(on_plainTextEdit_textChanged()), Qt::UniqueConnection);
    ui->plainTextEdit->setProperty("modified", QVariant(false));

    return true;
}

/**
 * @brief MainWindow::on_plainTextEdit_textChanged
 * Sets the modified property of the editor, so we'll know we need to save.
 */
void MainWindow::on_plainTextEdit_textChanged()
{
    qDebug() << "on_plainTextEdit_textChanged()";
    ui->plainTextEdit->setProperty("modified", QVariant(true));
    this->disconnect(ui->plainTextEdit, SIGNAL(textChanged()), this, SLOT(on_plainTextEdit_textChanged()));
}

/**
 * @brief MainWindow::on_actionExit_triggered
 * Exit Menu
 */
void MainWindow::on_actionExit_triggered()
{
    if (this->is_file_modified()) {
        QString filename;
        switch(this->ask_to_save()) {
            case (QMessageBox::Yes):
                filename = this->get_filename();
                if (filename.isNull()) {
                    filename = this->ask_for_filename();
                }
                this->do_save();
                this->saveAllState();
                qApp->quit();
                break;

            case (QMessageBox::No):
                this->saveAllState();
                qApp->quit();
                break;

            case (QMessageBox::Cancel):
                // Return from here, aborting the operation.
                break;
        }
    } else {
        this->saveAllState();
        qApp->quit();
    }
}

void MainWindow::do_open(QString filename)
{
    // Open the file
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    // Read from the file
    QTextStream in(&file);
    QTextDocument* document = ui->plainTextEdit->document();
    this->reset();
    document->setPlainText(in.readAll());
    file.close();

    // Set properties
    ui->plainTextEdit->setProperty("filename", filename);
    QString truncFileName = QString(filename);
    if (filename.length() > 40) {
        truncFileName = QString("...") + truncFileName.right(40);
    }
    this->setWindowTitle(QString(PROGRAM_NAME) + QString(" - ") + truncFileName);
    ui->actionSave->setEnabled(true);

    // Scroll to top
    ui->plainTextEdit->scroll(0, 0);
}

void MainWindow::on_actionOpen_triggered()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("QFileDialog::getOpenFileName"),
                                                    NULL,
                                                    tr("Todos os arquivos(*);;Arquivo de texto (*.txt)"),
                                                    &selectedFilter,
                                                    options);
    if (!filename.isEmpty()) {
        this->do_open(filename);
    }
}

/**
 * @brief MainWindow::on_actionChange_Font_triggered
 * Chooses a different font.
 */
void MainWindow::on_actionChange_Font_triggered()
{
    ui->plainTextEdit->setFont(QFontDialog::getFont(0, ui->plainTextEdit->font()));
}

/**
 * @brief MainWindow::on_actionSave_triggered
 * File -- Save button clicked
 */
void MainWindow::on_actionSave_triggered()
{
    if (this->is_file_modified()) {
        QString filename = this->get_filename();
        if (filename.isNull()) {
            // There is no filename yet
            this->on_actionSave_As_triggered();
            return;
        } else {
            this->do_save();
        }
    }
    // File hasn't been modified, no reason to save
}

void MainWindow::on_actionSave_As_triggered()
{
    QString filename = this->ask_for_filename();
    if (!filename.isEmpty()) {
        QString truncFileName = QString(filename);
        if (filename.length() > 40) {
            truncFileName = QString("...") + truncFileName.right(40);
        }

        this->setWindowTitle(QString(PROGRAM_NAME) + QString(" - ") + truncFileName);
        ui->actionSave->setEnabled(true);
        this->do_save();
    }
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox msgBox;
    msgBox.setText(QString(PROGRAM_NAME) + QString(" v") + QString(PROGRAM_VERSION));

    msgBox.addButton(tr("Fechar"), QMessageBox::NoRole);
    QPushButton* pButtonHomePage = msgBox.addButton(tr("Home Page"), QMessageBox::NoRole);
    msgBox.exec();

    if ((QPushButton*)(msgBox.clickedButton()) == pButtonHomePage) {
        QDesktopServices::openUrl(QUrl(" "));
    } else {
        // don't do anything, just close
    }
}
