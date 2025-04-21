#include "mainwindow.hpp"

enum {
    ID_OPEN_RECENT_1 = 1,
    ID_OPEN_RECENT_2,
    ID_OPEN_RECENT_3,
    ID_OPEN_RECENT_4,
};

MainWindow::MainWindow() : wxFrame(nullptr, wxID_ANY, "Warehouse") {
    m_pMenuFile = new wxMenu;
    m_pMenuFile->Append(wxID_NEW, "New Stockpile\tCtrl-N");
    m_pMenuFile->Append(wxID_OPEN, "Open Stockpile...\tCtrl-O");

    m_pSubMenuFileOpenRecent = new wxMenu;
    for (int i = 0; i < 4; i++) {
        m_pMenuItemFileOpenRecent[i] = m_pSubMenuFileOpenRecent->Append(ID_OPEN_RECENT_1 + i, "None");
        m_pMenuItemFileOpenRecent[i]->Enable(false);
    }

    m_pMenuFile->AppendSubMenu(m_pSubMenuFileOpenRecent, "Open Recent");

    m_pMenuItemFileSave = m_pMenuFile->Append(wxID_SAVE, "Save Stockpile\tCtrl-S");
    m_pMenuItemFileSave->Enable(false);
    m_pMenuItemFileSaveAs = m_pMenuFile->Append(wxID_SAVEAS, "Save Stockpile as...\tCtrl-Shift-S");
    m_pMenuItemFileSaveAs->Enable(false);
    m_pMenuItemFileClose = m_pMenuFile->Append(wxID_CLOSE, "Close Stockpile\tCtrl-W");
    m_pMenuItemFileClose->Enable(false);
    m_pMenuFile->Append(wxID_EXIT, "Exit Application\tAlt-F4");

    m_pMenuEdit = new wxMenu;
    m_pMenuItemEditImport = m_pMenuEdit->Append(wxID_ADD, "Import file...\tCtrl-I");
    m_pMenuItemEditImport->Enable(false);
    m_pMenuItemEditExport = m_pMenuEdit->Append(wxID_FORWARD, "Export selected file(s)...\tCtrl-E");
    m_pMenuItemEditExport->Enable(false);
    m_pMenuItemEditRemove = m_pMenuEdit->Append(wxID_DELETE, "Remove selected file(s)...\tDel");
    m_pMenuItemEditRemove->Enable(false);
    m_pMenuItemEditEdit = m_pMenuEdit->Append(wxID_EDIT, "Edit selected file");
    m_pMenuItemEditEdit->Enable(false);

    m_pMenuView = new wxMenu;
    m_pMenuItemViewPreview = m_pMenuView->Append(wxID_PREVIEW, "Preview selected file\tCtrl-P");
    m_pMenuItemViewPreview->Enable(false);

    m_pMenuBar = new wxMenuBar;
    m_pMenuBar->Append(m_pMenuFile, "&File");
    m_pMenuBar->Append(m_pMenuEdit, "&Edit");
    m_pMenuBar->Append(m_pMenuView, "&View");

    SetMenuBar(m_pMenuBar);

    wxPanel* tabControlPanel = new wxPanel(this, wxID_ANY);
    m_pTabControl = new wxAuiNotebook(tabControlPanel, wxID_ANY);

    wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
    topSizer->SetMinSize(250, 200);
    topSizer->Add(tabControlPanel, 1, wxEXPAND);
    SetSizerAndFit(topSizer);

    Bind(wxEVT_MENU, &MainWindow::OnExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, &MainWindow::OnFileNew, this, wxID_NEW);
    Bind(wxEVT_MENU, &MainWindow::OnFileOpen, this, wxID_OPEN);
    Bind(wxEVT_MENU, &MainWindow::OnFileSave, this, wxID_SAVE);
    Bind(wxEVT_MENU, &MainWindow::OnFileSaveAs, this, wxID_SAVEAS);
    Bind(wxEVT_MENU, &MainWindow::OnFileClose, this, wxID_CLOSE);
    Bind(wxEVT_MENU, &MainWindow::OnEditImport, this, wxID_ADD);
    Bind(wxEVT_MENU, &MainWindow::OnEditExport, this, wxID_FORWARD);
    Bind(wxEVT_MENU, &MainWindow::OnEditRemove, this, wxID_DELETE);
    Bind(wxEVT_MENU, &MainWindow::OnEditEdit, this, wxID_EDIT);
    Bind(wxEVT_MENU, &MainWindow::OnViewPreview, this, wxID_PREVIEW);
}

void MainWindow::OnExit(wxCommandEvent& event) {
    Close();
}

void MainWindow::OnFileNew(wxCommandEvent& event) {
    wxPanel* a = new wxPanel(m_pTabControl, wxID_ANY);
    wxPanel* b = new wxPanel(m_pTabControl, wxID_ANY);
    m_pTabControl->AddPage(a, "Tab A", true, wxID_ANY);
    m_pTabControl->AddPage(b, "Tab B", true, wxID_ANY);
}
void MainWindow::OnFileOpen(wxCommandEvent& event) {
}
void MainWindow::OnFileOpenRecent(wxCommandEvent& event) {
}
void MainWindow::OnFileSave(wxCommandEvent& event) {
}
void MainWindow::OnFileSaveAs(wxCommandEvent& event) {
}
void MainWindow::OnFileClose(wxCommandEvent& event) {
}

void MainWindow::OnEditImport(wxCommandEvent& event) {
}
void MainWindow::OnEditExport(wxCommandEvent& event) {
}
void MainWindow::OnEditRemove(wxCommandEvent& event) {
}
void MainWindow::OnEditEdit(wxCommandEvent& event) {
}

void MainWindow::OnViewPreview(wxCommandEvent& event) {
}
