#include "mainwindow.hpp"

enum {
    ID_OPEN_RECENT_1 = 1,
    ID_OPEN_RECENT_2,
    ID_OPEN_RECENT_3,
    ID_OPEN_RECENT_4,
};


#if defined(WAREHOUSE_USE_MDI) && WAREHOUSE_USE_MDI != 0

wxBEGIN_EVENT_TABLE(ChildWindow, wxMDIChildFrame)
    EVT_CLOSE(ChildWindow::OnClose)
wxEND_EVENT_TABLE()

size_t ChildWindow::s_uCount = 0;

ChildWindow::ChildWindow(MainWindow* parent, const wxString& title) : wxMDIChildFrame(parent, wxID_ANY, title), m_bModified(false) {
    m_pView = new StockpileView(this);

    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(m_pView, 1, wxEXPAND);
    SetSizerAndFit(sizer);

    s_uCount++;
}

void ChildWindow::OnClose(wxCloseEvent& event) {
    if (m_bModified) {
        int r = wxMessageBox("Would you like to save changes before closing the window?", "Unsaved changes", wxICON_QUESTION | wxYES | wxNO | wxCANCEL);
        if (r == wxYES) {
            // TODO: Save
        } else if (r == wxCANCEL) {
            event.Veto();
            return;
        }
    }

    size_t prev_count = s_uCount;
    if (s_uCount > 0) {
        s_uCount--;
    }

    if (prev_count > 0 && s_uCount == 0) {
        if (MainWindow* win = wxDynamicCast(GetParent(), MainWindow); win != nullptr) {
            win->EnableWindowDependentMenu(false);
        }
    }

    event.Skip();
}

#endif

MainWindow::MainWindow() : MainWindowParent(nullptr, wxID_ANY, "Warehouse") {
    SetClientSize(1024, 768);

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
#if defined(WAREHOUSE_USE_MDI) && WAREHOUSE_USE_MDI != 0
    m_pMenuView->AppendSeparator();
    m_pMenuView->Append(wxID_MDI_WINDOW_TILE_HORZ, "&Tile child windows horizontally\tCtrl-Shift-H");
    m_pMenuView->Append(wxID_MDI_WINDOW_TILE_VERT, "&Tile child windows vertically\tCtrl-Shift-V");
    m_pMenuView->Append(wxID_MDI_WINDOW_CASCADE, "&Cascade child windows\tCtrl-Shift-C");
    m_pMenuView->AppendSeparator();
    m_pMenuView->Append(wxID_CLOSE_ALL, "&Close all child windows\tCtrl-Shift-W");
#endif

    m_pMenuBar = new wxMenuBar;
    m_pMenuBar->Append(m_pMenuFile, "&File");
    m_pMenuBar->Append(m_pMenuEdit, "&Edit");
    m_pMenuBar->Append(m_pMenuView, "&View");

    SetMenuBar(m_pMenuBar);
    SetWindowMenu(nullptr);

#if !defined(WAREHOUSE_USE_MDI) || WAREHOUSE_USE_MDI == 0
    wxPanel* panel = new wxPanel(this, wxID_ANY);
    m_pTabControl = new wxAuiNotebook(panel, wxID_ANY);

    wxBoxSizer* panelSizer = new wxBoxSizer(wxHORIZONTAL);
    panelSizer->Add(m_pTabControl, 1, wxEXPAND);
    panel->SetSizer(panelSizer);

    wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
    topSizer->SetMinSize(250, 200);
    topSizer->Add(panel, 1, wxEXPAND);
    SetSizerAndFit(topSizer);
#endif

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
    Bind(wxEVT_MENU, &MainWindow::OnCloseAll, this, wxID_CLOSE_ALL);
}

void MainWindow::OnExit(wxCommandEvent& event) {
    Close();
}

void MainWindow::OnFileNew(wxCommandEvent& event) {
#if defined(WAREHOUSE_USE_MDI) && WAREHOUSE_USE_MDI != 0
    size_t prev_count = ChildWindow::children_count();
    ChildWindow* child = new ChildWindow(this, "Test " + std::to_string(ChildWindow::children_count()));
    child->Show(true);

    if (prev_count == 0 && ChildWindow::children_count() > 0) {
        EnableWindowDependentMenu(true);
    }
#else
    wxPanel* a = new wxPanel(m_pTabControl, wxID_ANY);
    wxPanel* b = new wxPanel(m_pTabControl, wxID_ANY);
    m_pTabControl->AddPage(a, "Tab A", true, wxID_ANY);
    m_pTabControl->AddPage(b, "Tab B", true, wxID_ANY);
#endif
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
    wxMDIChildFrame* child = GetActiveChild();
    if (child != nullptr) {
        child->Close();
    }
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

void MainWindow::OnCloseAll(wxCommandEvent& event) {
    for (auto child : GetChildren()) {
        if (wxDynamicCast(child, wxMDIChildFrame)) {
            child->Close();
        }
    }
}

void MainWindow::EnableWindowDependentMenu(bool enable) {
    m_pMenuItemFileSave->Enable(enable);
    m_pMenuItemFileSaveAs->Enable(enable);
    m_pMenuItemFileClose->Enable(enable);
    m_pMenuItemEditImport->Enable(enable);
}

StockpileView::StockpileView(wxWindow* parent) : wxPanel(parent) {
    m_pTextPath = new wxTextCtrl(this, wxID_ANY, "/");
    m_pListView = new wxListView(this);

    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(m_pTextPath, 1, wxEXPAND);
    sizer->AddSpacer(5);
    sizer->Add(m_pListView, 1, wxEXPAND);
    SetSizerAndFit(sizer);
}