#include "Mainwindow.h"
#include "ui_Mainwindow.h"

static int currentIndex = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(HOMEPAGE);

    setupDirectoryExplorer();

    /*
    cisc320(":/dummyDirs/ExampleCourses/CISC 320");
    elec370(":/dummyDirs/ExampleCourses/ELEC 370");

    ui->listWidget->addItem(cisc320.dirName());
    ui->listWidget->addItem(elec370.dirName());


    ui->listWidget->addItem("CISC 220");
    ui->listWidget->addItem("CISC 320");
    ui->listWidget->addItem("ELECT 370");

    ui->listWidget_2->addItem("Lectures");
    ui->listWidget_2->addItem("Assignments");

    ui->listWidget_3->addItem("Week 01");
    ui->listWidget_3->addItem("Week 02");
    ui->listWidget_3->addItem("Week 03");
    ui->listWidget_3->addItem("Week 04");
    ui->listWidget_3->addItem("Week 05");
    ui->listWidget_3->addItem("Week 06");
    */
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_login_clicked()
{
    currentIndex = ui->stackedWidget->currentIndex();
  if( currentIndex < ui->stackedWidget->count())
  {
      ui->stackedWidget->setCurrentIndex(YOURCLASSESPAGE);
  }
}

void MainWindow::on_pushButton_doneSubscribe_clicked()
{
    currentIndex = ui->stackedWidget->currentIndex();
  if( currentIndex < ui->stackedWidget->count())
  {
      ui->stackedWidget->setCurrentIndex(YOURCLASSESPAGE);
  }
}

void MainWindow::on_pushButton_doneSignUp_clicked()
{
    currentIndex = ui->stackedWidget->currentIndex();
    if (ui->lineEdit_password->text().isEmpty() || ui->lineEdit_fileDirectory->text().isEmpty() ||
                   ui->lineEdit_confirmPassword->text().isEmpty() || ui->lineEdit_username->text().isEmpty()) {
            QMessageBox::critical(this, "Empty Fields",
                          "Please ensure all fields are filled in.");
    }
    else if (ui->lineEdit_password->text() != ui->lineEdit_confirmPassword->text()){
        QMessageBox::critical(this, "Passwords Not Matched",
                     "'Password' and 'Confirm Password' do not match. "
                     "Please ensure the correct password is entered to continue.");
    }

    else if (ui->checkBox_cisc320->isChecked() == false && ui->checkBox_elec451->isChecked() == false
           && ui->checkBox_cisc221->isChecked() == false){
        QMessageBox::critical(this, "Must Select Class",
                      "Please select at least one class to subscribe to.");
    }
    else {

        QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Account Details",
                       "Are you sure you would like to proceed with:\nUsername: " + ui->lineEdit_username->text()
                      ,QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes){
            ui->label_myAccountUsername->setText("Hello, " + ui->lineEdit_username->text());

            /*
             * Hash password, get file path from GUI, and convert GUI update interval to number of hours.
             */
            int passwordHashed = hashPassword(ui->lineEdit_password->text().toStdString());
            string filepath = ui->lineEdit_fileDirectory->text().toStdString();

            int scaledInterval = 0;
            int interval = ui->updateIntervalComboBox->currentIndex();
            if (interval == 0)
                scaledInterval = 24;
            else if (interval == 1)
                scaledInterval = 24*7;
            else if (interval == 2)
                scaledInterval = 24*7*2;

            /* Course Object Creation specific for user selection.
             * This is then used in createUser() method to link courses to user object.
             */
            // TODO: This call may need to be moved once categoryPreferences is involved
            vector<Course> userCourses = createUserCourseObjects();

            // make user object function which checks if the username is unique or not
            // if unique, create user object, if not unique, tell user that already exists
            if(!(createUser(ui->lineEdit_username->text().toStdString(), passwordHashed,filepath,scaledInterval,userCourses))){
                QMessageBox::critical(this, "Exists",
                              "An account with this username already exists.");
            } else {
                if(currentIndex < ui->stackedWidget->count())
                {
                    ui->stackedWidget->setCurrentIndex(EDITSUBSCRIPTIONSPAGE);
                }
            }
        }
    }
}

/*
 * Function to hash a user's password before being passed to User object.
 */
int MainWindow::hashPassword(string password) {
    std::size_t str_password_hash = std::hash<std::string>{}(password);
    return str_password_hash;
}

/*
 * Function to create the User object and any associated DB calls. (including adding which courses the user is subscribed to)
 */
bool MainWindow::createUser(string username, int password, string path, int interval, vector<Course> userCourses){
    int result = Database::dbGetPasswordForUsername(username);
    if(result == -1){
        User userAccount = User(username,password,path,interval,userCourses);
        displayApplicableCourseTabs(userAccount);
        Database::dbCreateUserRow(userAccount.getUsername(),userAccount.getPassword(),userAccount.getFileDirectory(),userAccount.getUpdateInterval());

        // Create entry in usercourses DB table for each course the user has subscribed to.
        for(auto userCourse: userCourses) {
            Database::dbCreateUserCoursesRow(username, userCourse);
        }

        return true;
    }
    else {
        return false;
    }
}


/*
 * Function to create user specific Course object.
 * Returns the course object.
 */
Course MainWindow::createCourse(string courseName, string rootUrl, vector<CourseCategory> categories){
    Course userCourse = Course(courseName,rootUrl,categories);

    return userCourse;
}

/*
 * Function to create user specific Course objects depending on which courses they chose in the GUI.
 * Returns vector of course objects.
 */
vector<Course> MainWindow::createUserCourseObjects(){
    vector<Course> courseVector;
    string coursePath;

    // Since the course categories subscription phase is later on in the GUI, for now set as an empty categories vector.
    vector<CourseCategory> emptyCategoriesVector;

    // Note, The GUI ensures atleast one of these items is checked.
    // TODO: CISC221 won't work as a course (OnQ based), so we need a new course.
    if (ui->checkBox_cisc320->isChecked()) {
        coursePath = Database::dbGetCoursePath("CISC320");
        if (coursePath != "") {
            courseVector.push_back(createCourse("CISC320", coursePath, emptyCategoriesVector));
        } else {
           cout << "ERROR occured, course path could not be found for CISC320" << endl;
        }
    }
    if (ui->checkBox_cisc221->isChecked()) {
        coursePath = Database::dbGetCoursePath("CISC221");
        if (coursePath != "") {
            courseVector.push_back(createCourse("CISC221", coursePath, emptyCategoriesVector));
        } else {
           cout << "ERROR occured, course path could not be found for CISC221" << endl;
        }
    }
    if (ui->checkBox_elec451->isChecked()) {
        coursePath = Database::dbGetCoursePath("ELEC451");
        if (coursePath != "") {
            courseVector.push_back(createCourse("ELEC451", coursePath, emptyCategoriesVector));
        } else {
           cout << "ERROR occured, course path could not be found for ELEC451" << endl;
        }
    }
    return courseVector;
}

void MainWindow::on_pushButton_createAccount_clicked()
{
    currentIndex = ui->stackedWidget->currentIndex();
  if( currentIndex < ui->stackedWidget->count())
  {
      ui->stackedWidget->setCurrentIndex(SIGNUPPAGE);
  }
}

void MainWindow::on_pushButton_editSubs_clicked()
{
    if (currentIndex < ui->stackedWidget->count()){
        ui->stackedWidget->setCurrentIndex(EDITSUBSCRIPTIONSPAGE);
    }
}

void MainWindow::on_pushButton_Back_clicked()
{
    if (currentIndex < ui->stackedWidget->count()){
        ui->stackedWidget->setCurrentIndex(HOMEPAGE);
    }
}

void MainWindow::on_treeView_courseDirectories_clicked(const QModelIndex &index)
{
    QString path = dirModel->fileInfo(index).absoluteFilePath();
    ui->listView_courseFiles->setRootIndex(fileModel->setRootPath(path));
}

void MainWindow::on_listView_courseFiles_doubleClicked(const QModelIndex &index)
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileModel->fileInfo(index).absoluteFilePath()));
}

void MainWindow::displayApplicableCourseTabs(User userObj){
    vector<Course> subscription = userObj.getSubscribedCourses();
    bool cisc320 = false;
    bool cisc221 = false;
    bool elec451 = false;
    for (unsigned long i = 0; i < subscription.size(); i++){
        if(subscription[i].getCourseName() == "CISC320"){
            cisc320 = true;
        }
        else if(subscription[i].getCourseName() == "CISC221") {
            cisc221 = true;
        }
        else {
            elec451 = true;
        }
    }
      ui->tabWidget->setTabEnabled(1,cisc320);
      ui->tabWidget->setTabEnabled(2,elec451);
      ui->tabWidget->setTabEnabled(3,cisc221);
}


void MainWindow::setupDirectoryExplorer(){

    //Due to an apparent bug in Qt 4.7.2, the version I'm using, the QFileSystemModel
    //doesn't work with resource files. I'm not sure if its fixed in version 5.
    //So resourcePath is the path to use if resources work, otherwise, make a path to
    //your desktop. This isn't expected to be a problem with the final version of QNoteGet
    //as the directories won't be Qt resources
    // see here : https://bugreports.qt.io/browse/QTBUG-25007
    //          : https://bugreports.qt.io/browse/QTBUG-7010

    //TO USE DIRECTORY EXPLORER:
    // Either: replace all uses of desktopPath with resourcePath
    //      or
    //          If resource path still not working, edit desktopPath to your desktop,
    //          place resource file in appropriately named folder on desktop (or wherever you'd like I guess)

    QString resourcePath = ":/dummyDirs/ExampleCourses";
    QString desktopPath = "/Users/Marshall/Desktop/ExampleCourses";

    dirModel = new QFileSystemModel(this);
    dirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs); //hide "." and ".." folders, only display directories
    /*
     * QFileSystemModel will not fetch any files or directories until
     *  setRootPath() is called. This will prevent any unnecessary querying
     * on the file system until that point such as listing the drives on Windows.
     */
    dirModel->setReadOnly(true);
    ui->treeView_courseDirectories->setModel(dirModel);
    ui->treeView_courseDirectories->setRootIndex(dirModel->setRootPath(desktopPath));

    fileModel = new QFileSystemModel(this);
    fileModel->setFilter(QDir::NoDotAndDotDot | QDir::Files); //only show files
    fileModel->setRootPath(dirModel->rootPath());
    ui->listView_courseFiles->setModel(fileModel);

    QModelIndex coursesIndex = ui->treeView_courseDirectories->rootIndex();
    ui->treeView_courseDirectories->expand(coursesIndex);
    ui->treeView_courseDirectories->setCurrentIndex(coursesIndex);
    ui->treeView_courseDirectories->resizeColumnToContents(0);
}

void MainWindow::on_saveButton_Cisc320_clicked()
{
    courseCategorySaveButtonClicked(0);
}

// TODO: This function along with MainWindow::on_saveButton_Cisc320_clicked() serves as an EXAMPLE.
// Function/GUI should be changed to dynamically list the available categories etc. Using courseScraper information.
void MainWindow::courseCategorySaveButtonClicked(int courseTabId) {
    //Where courseTabId = 0 (CISC320), 1(ELEC451), 2(CISC221)
    QList<QCheckBox *> allCategoriesSelected;

    //Note use of Qt::FindDirectChildrenOnly to ensure only direct children are listed.
    //Example if lectures, assignments etc. is chosen.
    if (courseTabId == 0) {
        allCategoriesSelected = ui->groupBox_CISC320->findChildren<QCheckBox *>(QString(),Qt::FindDirectChildrenOnly);
    } else if (courseTabId == 1) {
        allCategoriesSelected = ui->groupBox_ELEC451->findChildren<QCheckBox *>(QString(),Qt::FindDirectChildrenOnly);
    } else if (courseTabId == 2) {
        allCategoriesSelected = ui->groupBox_CISC221->findChildren<QCheckBox *>(QString(),Qt::FindDirectChildrenOnly);
    } else {
        cout << "ERROR, Invalid courseTabId." << endl;
        return;
    }

    vector<CourseCategory> courseCategoryObjects;

    //qDebug() << allCategoriesSelected.size();
    for(int i = 0; i < allCategoriesSelected.size(); ++i)
    {
        if(allCategoriesSelected.at(i)->isChecked()) {
            //Debug print for what categories were selected.
            qDebug() << "Using course category: " << allCategoriesSelected.at(i)->text();

            string categoryName = allCategoriesSelected.at(i)->text().toStdString();

            string groupBoxNameForCategory = allCategoriesSelected.at(i)->objectName().toStdString();
            cout << groupBoxNameForCategory << endl;

            QList<QCheckBox *> allExtensionsSelected;
            // TODO: This is ugly code, we need a better way to do this...ASAP
            if (groupBoxNameForCategory == "LECT") {
                allExtensionsSelected = ui->groupBox_LECT->findChildren<QCheckBox *>(QString());
            } else if (groupBoxNameForCategory == "TUT") {
                allExtensionsSelected = ui->groupBox_TUT->findChildren<QCheckBox *>(QString());
            } else if (groupBoxNameForCategory == "ASSGN") {
                allExtensionsSelected = ui->groupBox_ASSGN->findChildren<QCheckBox *>(QString());
            }

            vector<string> choosenExtensions;
            //qDebug() << allExtensionsSelected.size();
            for(int i = 0; i < allExtensionsSelected.size(); ++i)
            {
                if(allExtensionsSelected.at(i)->isChecked()) {
                    qDebug() << "With file extension options: " << allExtensionsSelected.at(i)->text();
                    choosenExtensions.push_back(allExtensionsSelected.at(i)->text().toStdString());
                }
            }

            //Create course category object
            //TODO: Fill in appropriate file path, this should come from course scraper.
            courseCategoryObjects.push_back(CourseCategory(categoryName,"Blah blah some URL",choosenExtensions));
        }
    }

    //TODO: Link the course category objects to the course.


    return;
}
