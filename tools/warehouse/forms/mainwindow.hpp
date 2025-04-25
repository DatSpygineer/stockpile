#pragma once

#include <wx/wx.h>
#include <wx/mdi.h>
#include <wx/listctrl.h>
#include <wx/aui/aui.h>

class MainWindow;
class StockpileView;

#if defined(WAREHOUSE_USE_MDI) && WAREHOUSE_USE_MDI != 0
    using MainWindowParent = wxMDIParentFrame;
#else
    using MainWindowParent = wxFrame;
#endif

#if defined(WAREHOUSE_USE_MDI) && WAREHOUSE_USE_MDI != 0
class ChildWindow : public wxMDIChildFrame {
    StockpileView* m_pView;
    bool m_bModified;

    static size_t s_uCount;
public:
    ChildWindow(MainWindow* parent, const wxString& title);

    void OnClose(wxCloseEvent& event);

    static size_t children_count() { return s_uCount; }

    wxDECLARE_EVENT_TABLE();
};
#endif

class MainWindow : public MainWindowParent {
    wxMenuBar* m_pMenuBar;
    wxMenu* m_pMenuFile;
    wxMenu* m_pMenuEdit;
    wxMenu* m_pMenuView;

    wxMenu* m_pSubMenuFileOpenRecent;
    wxMenuItem* m_pMenuItemFileOpenRecent[4];

    wxMenuItem* m_pMenuItemFileSave;
    wxMenuItem* m_pMenuItemFileSaveAs;
    wxMenuItem* m_pMenuItemFileClose;

    wxMenuItem* m_pMenuItemEditImport;
    wxMenuItem* m_pMenuItemEditExport;
    wxMenuItem* m_pMenuItemEditRemove;
    wxMenuItem* m_pMenuItemEditEdit;

    wxMenuItem* m_pMenuItemViewPreview;
#if !defined(WAREHOUSE_USE_MDI) || WAREHOUSE_USE_MDI == 0
    wxAuiNotebook* m_pTabControl;
#endif
public:
    MainWindow();

private:
    void OnExit(wxCommandEvent& event);
    void OnFileNew(wxCommandEvent& event);
    void OnFileOpen(wxCommandEvent& event);
    void OnFileOpenRecent(wxCommandEvent& event);
    void OnFileSave(wxCommandEvent& event);
    void OnFileSaveAs(wxCommandEvent& event);
    void OnFileClose(wxCommandEvent& event);
    void OnEditImport(wxCommandEvent& event);
    void OnEditExport(wxCommandEvent& event);
    void OnEditRemove(wxCommandEvent& event);
    void OnEditEdit(wxCommandEvent& event);
    void OnViewPreview(wxCommandEvent& event);
    void OnCloseAll(wxCommandEvent& event);
    void EnableWindowDependentMenu(bool enable);

#if defined(WAREHOUSE_USE_MDI) && WAREHOUSE_USE_MDI != 0
    friend class ChildWindow;
#endif
};

class StockpileView : public wxPanel {
    wxTextCtrl* m_pTextPath;
    wxListView* m_pListView;
public:
    StockpileView(wxWindow* parent);
};