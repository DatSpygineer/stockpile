#pragma once

#include <wx/wx.h>
#include <wx/mdi.h>
#include <wx/aui/aui.h>

class MainWindow : public wxFrame {
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

    wxAuiNotebook* m_pTabControl;
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
};