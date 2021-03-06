#include "gui/MainWindow.h"
#include "gui/PreferencesDialog.h"
#include "utils/utils.h"
#include "utils/PreferencesManager.h"
#include <QMenuBar>
#include <QTextCodec>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSqlQuery>

using std::string;
using std::vector;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setCodecs("UTF-8");
    setWindowTitle(tr("Papers Light"));

    createMenus();
    createPanels();

    openDefaultDatabase();
}

MainWindow::~MainWindow()
{

}

void MainWindow::openDatabase()
{
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Open a Database File"),
                                                    QDir::homePath());

    if (!filePath.isEmpty()) {
        databaseHelper_.init(filePath);
        refreshAllPanels();
    }
}

void MainWindow::openDefaultDatabase()
{
    QString filePath = PreferencesManager::instance().getDatabaseFilePath();

    if (!filePath.isEmpty()) {
        databaseHelper_.init(filePath);
        refreshAllPanels();
    }
}

void MainWindow::newPaper()
{
    Paper paper;
    paper.setId(0);
    paper.setTitle("Untitled");

    papers_.push_back(paper);
    paperList_->addItem(paper);

    paperInfoTable_->setPaper(paper);
}

void MainWindow::editPreferences()
{
    PreferencesDialog preferencesDialog;
    preferencesDialog.exec();
}

void MainWindow::yearSelectedOnly(int index)
{
    searchHelper_.clear();

    SearchHelper::Filter filter;
    filter.first = SearchHelper::Year;
    filter.second = yearStats_[index].getName();

    searchHelper_.addFilter(filter);
    refreshPaperList();
}

void MainWindow::bookTitleSelectedOnly(int index)
{
    searchHelper_.clear();

    SearchHelper::Filter filter;
    filter.first = SearchHelper::BookTitle;
    filter.second = bookTitleStats_[index].getName();

    searchHelper_.addFilter(filter);
    refreshPaperList();
}

void MainWindow::authorSelectedOnly(int index)
{
    searchHelper_.clear();

    SearchHelper::Filter filter;
    filter.first = SearchHelper::Author;
    filter.second = authorStats_[index].getName();

    searchHelper_.addFilter(filter);
    refreshPaperList();
}

void MainWindow::tagSelectedOnly(int index)
{
    searchHelper_.clear();

    SearchHelper::Filter filter;
    filter.first = SearchHelper::Tag;
    filter.second = tagStats_[index].getName();

    searchHelper_.addFilter(filter);
    refreshPaperList();
}

void MainWindow::paperSelectedOnly(int index)
{
    paperInfoTable_->setPaper(papers_[index]);
}

void MainWindow::savePaper()
{
    Paper paper = paperInfoTable_->getPaper();
    int paperId = databaseHelper_.updatePaper(paper);
    if (paper.getId() <= 0) {
        paper.setId(paperId);
        paperInfoTable_->setPaper(paper);
    }

    refreshAllPanels();
}

void MainWindow::removePaper(Paper paper)
{
    databaseHelper_.removePaper(paper);

    refreshAllPanels();
}

void MainWindow::setCodecs(const char* codec)
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName(codec));
}

void MainWindow::createMenus()
{
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    openDatabaseAction_ = fileMenu->addAction(tr("Open Database File"));
    connect(openDatabaseAction_, &QAction::triggered, this, &MainWindow::openDatabase);

    fileMenu->addSeparator();
    newPaperAction_ = fileMenu->addAction(tr("New Paper"));
    newPaperAction_->setShortcut(QKeySequence::New);
    connect(newPaperAction_, &QAction::triggered, this, &MainWindow::newPaper);

    QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));
    editPreferencesAction_ = editMenu->addAction(tr("Edit Preference"));
    connect(editPreferencesAction_, &QAction::triggered, this, &MainWindow::editPreferences);
}

void MainWindow::createPanels()
{
    yearList_ = new CategoryList(tr("Year"));
    bookTitleList_ = new CategoryList(tr("Book Title"));
    authorList_ = new CategoryList(tr("Author"));
    tagList_ = new CategoryList(tr("Tag"));
    searchBar_ = new SearchBar;
    paperList_ = new PaperList;
    paperInfoTable_ = new PaperInfoTable;

    connect(yearList_, &CategoryList::itemSelectedOnly, this, &MainWindow::yearSelectedOnly);
    connect(bookTitleList_, &CategoryList::itemSelectedOnly, this, &MainWindow::bookTitleSelectedOnly);
    connect(authorList_, &CategoryList::itemSelectedOnly, this, &MainWindow::authorSelectedOnly);
    connect(tagList_, &CategoryList::itemSelectedOnly, this, &MainWindow::tagSelectedOnly);
    connect(paperList_, &PaperList::itemSelectedOnly, this, &MainWindow::paperSelectedOnly);
    connect(paperInfoTable_, &PaperInfoTable::paperSaved, this, &MainWindow::savePaper);
    connect(paperInfoTable_, &PaperInfoTable::paperRemoved, this, &MainWindow::removePaper);

    QVBoxLayout* leftPanelLayout = new QVBoxLayout;
    leftPanelLayout->addWidget(yearList_);
    leftPanelLayout->addWidget(bookTitleList_);
    leftPanelLayout->addWidget(authorList_);
    leftPanelLayout->addWidget(tagList_);

    QVBoxLayout* middlePanelLayout = new QVBoxLayout;
    middlePanelLayout->addWidget(searchBar_);
    middlePanelLayout->addWidget(paperList_);
    searchBar_->setMaximumWidth(500);
    paperList_->setMaximumWidth(500);

    QVBoxLayout* rightPanelLayout = new QVBoxLayout;
    rightPanelLayout->addWidget(paperInfoTable_);

    QHBoxLayout* frameLayout = new QHBoxLayout;
    frameLayout->addLayout(leftPanelLayout);
    frameLayout->addLayout(middlePanelLayout);
    frameLayout->addLayout(rightPanelLayout);

    QWidget* frame = new QWidget;
    frame->setLayout(frameLayout);

    setCentralWidget(frame);

    showMaximized();
}

void MainWindow::refreshAllPanels()
{
    yearStats_ = databaseHelper_.getYearStats(DatabaseHelper::SortByName);
    bookTitleStats_ = databaseHelper_.getBookTitleStats(DatabaseHelper::SortByName);
    authorStats_ = databaseHelper_.getAuthorStats(DatabaseHelper::SortByName);
    tagStats_ = databaseHelper_.getTagStats(DatabaseHelper::SortByCount);

    yearList_->clear();
    yearList_->addItems(yearStats_);

    bookTitleList_->clear();
    bookTitleList_->addItems(bookTitleStats_);

    authorList_->clear();
    authorList_->addItems(authorStats_);

    tagList_->clear();
    tagList_->addItems(tagStats_);

    searchHelper_.clear();

    refreshPaperList();
}

void MainWindow::refreshPaperList()
{
    string queryString = searchHelper_.getSqlQueryString();
    QSqlQuery query = databaseHelper_.exec(queryString.c_str());

    papers_.clear();
    while (query.next()) {
        int paperId = query.value(0).toInt();
        papers_.push_back(databaseHelper_.getPaper(paperId));
    }

    paperList_->clear();
    paperList_->addItems(papers_);
}
