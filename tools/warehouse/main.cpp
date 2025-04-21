#include <wx/wxprec.h>

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "forms/mainwindow.hpp"

class WareHouseApp : public wxApp {
    MainWindow* m_pWindow;
public:
    bool OnInit() override {
        m_pWindow = new MainWindow;
        m_pWindow->Show(true);
        return true;
    }
};
wxIMPLEMENT_APP(WareHouseApp);