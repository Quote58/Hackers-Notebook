// Hexer

#ifndef HEXER_H
#define HEXER_H

//If I end up needing to scale the text, this is how
//editBook->SetFont(editBook->GetFont().Scale(1.25));

// The notebook changing pages affects the buttons a little bit
//Bind(wxEVT_BOOKCTRL_PAGE_CHANGING, &HexerFrame::onNotebookChange, this, ID_DocsViewNB);

// For compilers that don't support precompilation, include "wx/wx.h"
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include <wx/notebook.h>
#include "wx/colordlg.h"
#include "wx/fontdlg.h"
#include "wx/numdlg.h"
#include "wx/aboutdlg.h"

#include "wx/grid.h"
#include "wx/headerctrl.h"
#include "wx/generic/gridctrl.h"
#include "wx/generic/grideditors.h"
#include <wx/tglbtn.h>
#include <wx/toolbar.h>
#include <wx/filedlg.h>
#include <wx/wfstream.h>
#include <wx/artprov.h>
#include <wx/vector.h>
#include <wx/checkbox.h>
#include <wx/log.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>
#include <wx/srchctrl.h>
#include <wx/statbox.h>
#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/scrolwin.h>
#include <wx/clrpicker.h>
#include <wx/windowptr.h>
#include <wx/stc/stc.h>
#include "wx/numformatter.h"
#include "wx/renderer.h"
#include "wx/uilocale.h"
#include <wx/spinctrl.h>

#include "rom.h"
#include "romEditor.h"

// For some reason this isn't a default template?
template<class T> using wxVector2D = wxVector< wxVector<T> >;

enum CommonValues {
	kMacMargins = 19,
};

enum ID {
	// You can not add a menu ID of 0 on macOS (it's reserved for 'help' I think?)
	ID_Default,

	// Main components
	ID_ToolBar,
	ID_EditView,
	ID_DocsView,
	ID_HexView,
	ID_Statbar,
	
	// MenuBar
	ID_MenuSave,
	ID_MenuOpen,
	ID_MenuLoadTweaks,
	ID_MenuLoadDocs,
	ID_MenuRefresh,
	ID_MenuUndo,
	ID_MenuAdd,
	ID_MenuPreferences,
	ID_MenuCredits,
	ID_MenuContact,

	// ToolBar
	ID_ToolPrefs,
	ID_ToolEdit,
	ID_ToolDocs,
	ID_ToolHex,
	ID_ToolUndo,
	ID_ToolSearch,
	ID_ToolSave,
	ID_ToolOpen,

	// Grids

	// Misc
	ID_EditMode,

	// Dialog
	ID_DTitle,
	ID_DCat,
	ID_DDesc,
	ID_DSize,
	ID_DType,
	ID_DAddress,
	ID_DOldBytes,
	ID_DNewBytes

};

enum Views {
	kViewDocs,
	kViewHex,
	kViewEdit
};

struct PatchBytes {
	int _offset = 0;
	wxVector<wxByte> _newBytes;
	wxVector<wxByte> _oldBytes;
};

struct Entry {
	int _cat = 0;
	wxString _name = "";
	wxString _desc = "";
	wxString _addr = "";
	wxString _size = "";
	wxString _type = "";
	wxVector<PatchBytes> _bytes;
};

struct ViewData {
	wxNotebook  *_noteBook;
	 wxTextCtrl *_catName;
	wxStaticBoxSizer *_sidePanel;
	wxButton *_showSidePanel;
	wxVector<wxString>  _catNames;
	wxVector<wxPanel *> _pages;
	wxVector<wxGrid *>  _grids;
};

/* Hexer Application
 * This class controls the entire program
 */
class Hexer : public wxApp {
public:
	// Preferences needs to be part of the app, not the frame
	wxTextFile _prefFile;

	virtual bool OnInit();
};

/* Image cell renderer
 * A simple class overriding the string cell renderer.
 * This one just draws an edit icon instead of a string.
 */
class ImageCellRenderer : public wxGridCellStringRenderer {
public:
    virtual void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) wxOVERRIDE;
};

/* Three-state bool editor
 * A simple class overriding the bool cell renderer.
 * This one just creates a 3-state checkbox instead of default.
 */
class ThreeStateBoolEditor : public wxGridCellBoolEditor {
public:
    virtual void Create(wxWindow* parent,wxWindowID id, wxEvtHandler* evtHandler) wxOVERRIDE;
};

/* Renderer for three-state bool editor
 * This class adjusts the renderer, specifically the flags when calling the draw checkbox
 * function of the renderer, to show a third state if the value is 2
 */
class ThreeStateBoolRenderer : public wxGridCellBoolRenderer {
public:
	  virtual void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) wxOVERRIDE;
};

/* Hexer Main Frame (ha)
 * This class is the frame within which all
 * panels and controls get placed
 */
class HexerFrame : public wxFrame {
public:
	HexerFrame(wxSize s);

protected:
	// At some point I'm sure the logo will be used
	wxBitmap *_logo;

	// The rom is a member so that it can be read/written from anywhere
	Rom *_rom = nullptr;

	// We also need the files to be accessable as members
	wxTextFile _editFile;
	wxTextFile _docsFile;

	/* For the toolbar */
	wxToggleButton *_toggleViews[3];
		   wxPanel *_views[3];

	/* Top level aspects of the program */
	 int _view 		= kViewEdit;
	 int _prevRow   = -1;
	 int _curRow    = -1;
	bool _editMode  = false;
	bool _addOrEdit = false;
	bool _editOrDoc = false;

	// _mainSizer is a vertical box sizer containing all 3 views
	wxBoxSizer *_mainSizer;
	   wxPanel *_editView;
	   wxPanel *_docsView;
	   wxPanel *_hexView;

	/* Specific to the EditView */
	ViewData *_editData = nullptr;
	wxVector2D<Entry> _editPatches;

	/* Specific to the DocsView */
	ViewData *_docsData = nullptr;
	wxVector2D<Entry> _docsEntries;

	/* Specific to the HexView */
			  wxGrid *_hexGrid;
			  wxGrid *_header;
		  wxBoxSizer *_headerSizer;
	  RomEditorTable *_hexTable;
		  wxCheckBox *_gridLines;
    wxStaticBoxSizer *_stringPanel;
    wxStaticBoxSizer *_palettePanel;
    wxStaticBoxSizer *_gfxPanel;

    wxStaticBoxSizer *_viewPanels[3];

private:
	// Debug
	void debug(wxString s);
	void debug(int i);

	// Init functions
	void populateMenuBar(wxMenuBar *menuBar);
	void populateToolBar(wxToolBar *toolBar);
	void populateEditViewDefault();
	void populateEditView();
	void populateDocsView();
	void populateHexView();
	void populateHexViewDefault();

	// Menu functions
	void onLoadTweaks(wxCommandEvent& event);
	void onLoadDocs(wxCommandEvent& event);
	void onMenuEdit(wxCommandEvent& event);
	void onMenuDocs(wxCommandEvent& event);
	void onMenuHex(wxCommandEvent& event);
	void onPreferences(wxCommandEvent& event);
	void onContact(wxCommandEvent& event);
	void onCredits(wxCommandEvent& event);
	void onExit(wxCommandEvent& event);
	void onAbout(wxCommandEvent& event);

	// Toolbar functions
	void onToggle(wxCommandEvent &event);
	void onUndo(wxCommandEvent &event);

	// Edit View functions
	void onEditGridLeftClick(wxGridEvent& event);
	void onMouseInEditGrid(wxMouseEvent& event);
	void onAddPatch(wxCommandEvent& event);
	void onLoadNewPatchFile();
	void askDeleteClosedEdit(wxWindowModalDialogEvent &event);

	// Docs View functions
	void onDocsGridLeftClick(wxGridEvent& event);
	void onMouseInDocsGrid(wxMouseEvent& event);
	void prepBitflagDraw(int size, wxString t, wxString a, wxString &addrLine, wxString &zeroLine);
	void drawBitflagDiagram(int size, wxString desc, int cat, int lineNum, wxFont f, wxString addrLine, wxString zeroLine, wxString name);
	void askDeleteClosedDocs(wxWindowModalDialogEvent &event);
	void onAddEntry(wxCommandEvent &event);
	void onNotebookChange(wxBookCtrlEvent &event);
	void onTextEnter(wxCommandEvent& event);
	void onDocsGridDoubleClick(wxGridEvent &event);

	// Hex View functions
	void onColourPickerChanged(wxColourPickerEvent &event);
	void onGoToEnter(wxCommandEvent &event);
	void goToOffset(int offset);
	void createHexEditorHeader();
	void createHexEditor();
	void onViewTypeChoice(wxCommandEvent &event);
	void onPresetChoice(wxCommandEvent &event);
	void onChangeStringTable(wxCommandEvent &event);
	void onGridLinesCheck(wxCommandEvent &event);
	void onStringByteSizeChanged(wxSpinEvent &event);
	void onFormatChanged(wxSpinEvent &event);
	void onGfxBitdepthChanged(wxSpinEvent &event);
	void onArrowUp(wxCommandEvent &event);
	void onArrowDown(wxCommandEvent &event);
	void onArrowLeft(wxCommandEvent &event);
	void onArrowRight(wxCommandEvent &event);
	void onHexViewDClick(wxGridEvent &event);
	 int calcByteSize(int numBits);
	void updateColLabels(int size);
	void updateGridProps(int size, int cellSize, int byteSize, wxString colWidth);
	void loadDefaultStringTable();
	void resetTableSize();
	void refreshEditor();
	void onScrollEnd(wxScrollWinEvent &event);
	void onMouseWheel(wxMouseEvent &event);
	void adjustForScroll();
	void onColourSearch(wxCommandEvent &event);
	void loadDefaultPalettes();
	void loadPalette(wxVector<wxColour> &targetPal);
	void onChangeGfxPalette(wxCommandEvent &event);
	void onLoadIndexedPalette(wxCommandEvent &event);
	void onGfxPalChanged(wxSpinEvent &event);
	void onGfxRefresh(wxCommandEvent &event);

	// General program functions
	void onOpen(wxCommandEvent& event);
	void onSave(wxCommandEvent& event);
	void saveLocalEdit();
	void saveLocalDocs();
	void addEntry(bool addOrEdit, wxString n, wxString d, wxString s, wxString t, wxString a, wxString nb, wxString ob);
	void onAddEntryApply(wxCommandEvent &event);
	void onRefresh(wxCommandEvent& event);
	 int YToRowGood(wxGrid *grid, int y);
	void moreInfo(wxString description, bool big);
	void onGridMouseExit(wxMouseEvent& event);
	void onSearch(wxCommandEvent &event);
	void createPage(ViewData *data, wxString pageName, int row, int col);
	void onEditModeButton(wxCommandEvent& event);
	void createSidePanel(ViewData *data, wxPanel *view);
	void onShowSidePanel(wxCommandEvent &event);
	ViewData *getViewData();
};

DECLARE_APP(Hexer)

#endif

