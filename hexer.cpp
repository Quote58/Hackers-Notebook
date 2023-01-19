// Hexer
#include "hexer.h"
#include "rom.h"

IMPLEMENT_APP(Hexer)

// App methods
bool Hexer::OnInit()
{
	// We will be using .png images for the program, so we need a handler for them
	wxImage::AddHandler(new wxPNGHandler);

	// We need to load preferences early so we can use the right starting size

	// Which is by default 800x600
	int sizeX = 850;
	int sizeY = 600;

	wxTextFile _prefFile(wxString("preferences"));
	if (!_prefFile.Open()) {
		wxLogError(wxString("Could not load preferences file, using default size"));

	} else {
		sizeX = wxAtoi(_prefFile.GetFirstLine().SubString(5,7));
		sizeY = wxAtoi(_prefFile.GetFirstLine().SubString(9,11));
	}

	// Create the frame that will contain the program
	HexerFrame *frame = new HexerFrame(wxSize(sizeX, sizeY));
	frame->Show(true);

	// Hexer needs to be resizable, but we don't want it going smaller than the default
	frame->SetMinSize(wxSize(sizeX, sizeY));

	return true;
}

// ------------------------------------------------------------------
/* ****               ****
 * **** The App Frame ****
 * ****               ****
 */
void HexerFrame::debug(wxString s) {
	std::cout << s << std::endl;
}

void HexerFrame::debug(int i) {
	std::cout << i << std::endl;
}

HexerFrame::HexerFrame(wxSize s) : wxFrame(NULL, wxID_ANY, "Hackers Notebook", wxDefaultPosition, s) {
	// The program has a logo, so we should set it up at the start
	// Although this doesn't seem to work for making the program icon use the image...
	_logo = new wxBitmap(wxT("icons/logo.png"), wxBITMAP_TYPE_PNG);

	/* This program is structured as:
	 * /--- Menu ---\
	 * |-- Toolbar -|
	 * |--- Panel --|
	 * \-- StatBar -/
	 */

	//formatstuff();
	//return;

	// So first we give it a menu
	wxMenuBar *menuBar = new wxMenuBar;				// Create a menu bar for our menus
	populateMenuBar(menuBar);
	SetMenuBar(menuBar);

	// Next we give it a toolbar
	wxToolBar *toolBar = new wxToolBar(this, ID_ToolBar);
	populateToolBar(toolBar);
	SetToolBar(toolBar);
	toolBar->Realize();

	// After the toolbar is the main program area, which needs a general sizer to correctly size the panel when resizing
	_mainSizer = new wxBoxSizer(wxVERTICAL);

	/* We will have three panels active at any given time, but the hex tweaks and hex editor
	 * only get populated when a rom is loaded
	 */

	// First is the Documents View
	_view = kViewDocs;
	_docsView = new wxPanel(this, ID_DocsView, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, "Documents Panel");
	populateDocsView();								// The document view does not need a rom loaded, so it can be populated right away
	_docsView->Show(false);
	_views[0] = _docsView;
	_mainSizer->Add(_docsView, 1, wxGROW);

	// Next is the Rom Editor View
	_view = kViewHex;
	_hexView = new wxPanel(this, ID_HexView, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, "Hex Editor Panel");
	populateHexViewDefault();
	_hexView->Show(false);
	_views[1] = _hexView;
	_mainSizer->Add(_hexView, 1, wxGROW);
	
	// Last is the Edit View
	_view = kViewEdit;
	_editView = new wxPanel(this, ID_EditView, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, "Edit Panel");
	populateEditViewDefault();
	_editView->Show(false);
	_views[2] = _editView;
	_mainSizer->Add(_editView, 1, wxGROW);

	// The program defaults to the docs view first (maybe this should be a preference?)
	_view = kViewDocs;
	_docsView->Show(true);

	// And now we need to apply the sizer to the frame
	_mainSizer->Layout();
	SetSizer(_mainSizer);

	// Finally, we also want a status bar for showing the last action performed (ie. 'Rom saved successfull')
	//CreateStatusBar();

	// By default, the status bar just tells the user they should load a rom
	//SetStatusText("To get started, load a rom");
}
// ------------------------------------------------------------------

// ------------------------------------------------------------------
/* ****              ****
 * **** The Menu Bar ****
 * ****              ****
 */
void HexerFrame::populateMenuBar(wxMenuBar *menuBar) {

	/* ---- File ----
	 * -Save Changes
	 * -Load Rom
	 * -Load Tweaks
	 * -Load Docs
	 * -Refresh
	 */
	wxMenu *menuFile = new wxMenu;
	menuFile->Append(ID_MenuOpen, 		"&Open...\tCtrl-O", "Load a new Rom");
	menuFile->Append(ID_MenuSave,		"&Save\tCtrl-S", "Save recent changes made to rom");
	menuFile->AppendSeparator();
	menuFile->Append(ID_MenuLoadTweaks, "&Load New Patches", "Load a new Patches file");
	menuFile->Append(ID_MenuLoadDocs, 	"&Load New Documents", "Load a new Documents file");
	menuFile->AppendSeparator();
	menuFile->Append(ID_MenuRefresh, 	"&Refresh\tCtrl-R", "Refresh the primary Hex Tweaks and Documents with the source");
	/* -------------- */

	/* ---- Edit ----
	 * -Preferences
	 * -Undo
	 * -Add
	 */
	wxMenu *menuEdit = new wxMenu;
	menuEdit->Append(ID_MenuUndo, "&Undo\tCtrl-Z", "Undo the last action");
	/* -------------- */

	/* ---- Help ----
	 * -Credits
	 * -Contact
	 */
	wxMenu *menuHelp = new wxMenu;
	menuHelp->AppendSeparator();
	menuHelp->Append(ID_MenuContact, "&Contact", "Show contact information");
	menuHelp->Append(ID_MenuCredits, "&Credits", "Show credits");
	/* -------------- */

	/* ---- Hexer ----
	 * (special cases)
	 * Quit
	 * About
	 * Preferences
	 */
	menuFile->Append(wxID_EXIT);
	menuFile->Append(wxID_PREFERENCES);
	menuHelp->Append(wxID_ABOUT);
	/* --------------- */
 
	// Append the menu items to the menu bar
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuEdit, "&Edit");
	menuBar->Append(menuHelp, "&Help");

	// And finally, we bind all the events to their respective functions
	Bind(wxEVT_MENU, &HexerFrame::onOpen,    	 this, ID_MenuOpen);
	Bind(wxEVT_MENU, &HexerFrame::onSave,    	 this, ID_MenuSave);
	Bind(wxEVT_MENU, &HexerFrame::onLoadTweaks,  this, ID_MenuLoadTweaks);
	Bind(wxEVT_MENU, &HexerFrame::onLoadDocs,    this, ID_MenuLoadDocs);
	Bind(wxEVT_MENU, &HexerFrame::onRefresh,     this, ID_MenuRefresh);
	Bind(wxEVT_MENU, &HexerFrame::onContact,	 this, ID_MenuContact);
	Bind(wxEVT_MENU, &HexerFrame::onCredits, 	 this, ID_MenuCredits);
	Bind(wxEVT_MENU, &HexerFrame::onAbout,   	 this, wxID_ABOUT);
	Bind(wxEVT_MENU, &HexerFrame::onExit,    	 this, wxID_EXIT);
	Bind(wxEVT_MENU, &HexerFrame::onPreferences, this, wxID_PREFERENCES);
}

void HexerFrame::onLoadTweaks(wxCommandEvent& event) {}
void HexerFrame::onLoadDocs(wxCommandEvent& event) {}
void HexerFrame::onRefresh(wxCommandEvent& event) {}
void HexerFrame::onMenuEdit(wxCommandEvent& event) {}
void HexerFrame::onMenuDocs(wxCommandEvent& event) {}
void HexerFrame::onMenuHex(wxCommandEvent& event) {}
void HexerFrame::onPreferences(wxCommandEvent& event) {}

void HexerFrame::onContact(wxCommandEvent& event) {
	wxMessageBox("Discord: Quote58#6249\nTwitter: @Quote_58", "Please report issues to:", wxOK | wxICON_INFORMATION);
}
 
void HexerFrame::onCredits(wxCommandEvent& event) {
	wxMessageBox("MetroidConstruction.com","Special thanks to:", wxOK | wxICON_INFORMATION);
}

void HexerFrame::onAbout(wxCommandEvent& event) {
	wxMessageBox("The Hackers Notebook is designed to be a\n'Hackers' Digital Assistant'", "About Hexer", wxOK | wxICON_INFORMATION);
}

void HexerFrame::onExit(wxCommandEvent& event) {
	// This true ensures that this close button has vito over all windows
	Close(true);
}
// ------------------------------------------------------------------

// ------------------------------------------------------------------
/* ****              ****
 * **** The Tool Bar ****
 * ****              ****
 */
void HexerFrame::populateToolBar(wxToolBar *toolBar) {
	/* Despite sizers normally not seeming to work in toolbars, this
	 * *requires* a staticboxsizer to work as it does currently. Why
	 * that is? Unclear.
	 */
	wxStaticBoxSizer *viewBox = new wxStaticBoxSizer(wxHORIZONTAL, toolBar, wxEmptyString);

	// These are just | characters, but to get them aligned nicely they need to be outside of the buttons
	wxStaticText *separator  = new wxStaticText(toolBar, wxID_ANY, "|", wxDefaultPosition, wxDefaultSize, 0, wxEmptyString);
	wxStaticText *separator2 = new wxStaticText(toolBar, wxID_ANY, "|", wxDefaultPosition, wxDefaultSize, 0, wxEmptyString);

	// We have a button for each view, and since only one is shown at a time, they are toggle buttons
	wxToggleButton *toggleEdit = new wxToggleButton(toolBar, ID_ToolEdit, "Documents", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Edit");
	_toggleViews[0] = toggleEdit;
	
	wxToggleButton *toggleDocs = new wxToggleButton(toolBar, ID_ToolDocs, "Rom Editor", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Docs");
	_toggleViews[1] = toggleDocs;
	
	wxToggleButton *toggleHex  = new wxToggleButton(toolBar, ID_ToolHex,  "Patches", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Hex");
	_toggleViews[2] = toggleHex;

	// We also have a search bar, and buttons for undo/save/open rom
	wxSearchCtrl *searchBar = new wxSearchCtrl(toolBar, ID_ToolSearch, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Search");

	wxButton *buttonUndo = new wxButton(toolBar, ID_ToolUndo, "Undo", wxDefaultPosition, wxDefaultSize, wxBU_NOTEXT | wxBORDER_NONE | wxBU_EXACTFIT, wxDefaultValidator, "Undo");
	buttonUndo->SetBitmap(wxArtProvider::GetIcon(wxART_UNDO, wxART_FRAME_ICON));
	
	wxButton *buttonSave = new wxButton(toolBar, ID_ToolSave, "Save", wxDefaultPosition, wxDefaultSize, wxBU_NOTEXT | wxBORDER_NONE | wxBU_EXACTFIT, wxDefaultValidator, "Save");
	buttonSave->SetBitmap(wxArtProvider::GetIcon(wxART_NORMAL_FILE, wxART_FRAME_ICON));

	wxButton *buttonOpen = new wxButton(toolBar, ID_ToolOpen, "Open", wxDefaultPosition, wxDefaultSize, wxBU_NOTEXT | wxBORDER_NONE | wxBU_EXACTFIT, wxDefaultValidator, "Open");
	buttonOpen->SetBitmap(wxArtProvider::GetIcon(wxART_FOLDER, wxART_FRAME_ICON));

	// Now we can add the controls to the toolbar
	toolBar->AddControl(toggleEdit);
	//toolBar->AddControl(separator);
	toolBar->AddControl(toggleDocs);
	//toolBar->AddControl(separator2);
	toolBar->AddControl(toggleHex);
	toolBar->AddStretchableSpace();
	toolBar->AddControl(searchBar);
	//toolBar->AddControl(buttonUndo);
	toolBar->AddControl(buttonSave);
	toolBar->AddControl(buttonOpen);

	// And set up the sizer that arranges them (despite this box sizer not actually showing a box around them??)
	viewBox->Add(toggleEdit, 0, wxALIGN_CENTER);
	viewBox->Add(separator,  0, wxALIGN_CENTER);
	viewBox->Add(toggleDocs, 0, wxALIGN_CENTER);
	viewBox->Add(separator2, 0, wxALIGN_CENTER);
	viewBox->Add(toggleHex,  0, wxALIGN_CENTER);

	viewBox->Layout();
	toolBar->SetSizer(viewBox);

	// The default view is Edit, and since the default value for a toggle is false, we need to set this one to true
	toggleEdit->SetValue(true);

	Bind(wxEVT_TOGGLEBUTTON, &HexerFrame::onToggle, this, ID_ToolEdit);
	Bind(wxEVT_TOGGLEBUTTON, &HexerFrame::onToggle, this, ID_ToolDocs);
	Bind(wxEVT_TOGGLEBUTTON, &HexerFrame::onToggle, this, ID_ToolHex);
	Bind(wxEVT_BUTTON, 		 &HexerFrame::onUndo,	this, ID_ToolUndo);
	Bind(wxEVT_BUTTON, 		 &HexerFrame::onSave,	this, ID_ToolSave);
	Bind(wxEVT_BUTTON, 		 &HexerFrame::onOpen,	this, ID_ToolOpen);
	Bind(wxEVT_SEARCH,		 &HexerFrame::onSearch, this, ID_ToolSearch);
}

void HexerFrame::onToggle(wxCommandEvent &event) {
	/* This function does three things when a toggle view button is pressed:
	 * 1. Shows the current view panel and hides the others
	 * 2. Deselects the other buttons
	 * 3. Adjusts the label of each button to show a • next to the active one
	 */

	// The eventID will be toolEdit + 1 or 2, so subtracing the lower IDs gives us a range of 0-2
	int evtID = (event.GetId() - ID_ToolEdit);
	wxString _names[3] = {"  Patches", "  Documents", "  Hex\t"};

	// This loop will deselect everything except the index which matches the button that triggered it
	// Could still be used if more views are added
	for (int i = 0; i < 3; i++) {
		if (i == evtID) {
			_views[i]->Show(true);
			_toggleViews[i]->SetValue(true);
			// The dot is larger than a normal char, so we can't just do s[0] = dot
			//wxString s = _toggleViews[i]->GetLabel();
			//_toggleViews[i]->SetLabel("» " + s.substr(2));
			_view = i;
		} else {
			_views[i]->Show(false);
			_toggleViews[i]->SetValue(false);
			//_toggleViews[i]->SetLabel(names[i]);
		}
		_prevRow = -1;
	}
	_mainSizer->Layout();
}

void HexerFrame::onSearch(wxCommandEvent &event) {
	wxString s = event.GetString();
	int cat = -1;
	int entry = -1;
	for (int i = 0; i < _editPatches.size(); i++) {
		for (int j = 0; j < _editPatches[i].size(); j++) {
			if (_editPatches[i][j]._name.CmpNoCase(s) == 0) {
				debug("yes?");
				cat = i;
				entry = j;
				_editData->_noteBook->SetSelection(cat);
				return;
			}
		}
	}
	debug("couldn't find the search term");
}

void HexerFrame::onUndo(wxCommandEvent &event) {
}

// ------------------------------------------------------------------

// ------------------------------------------------------------------
/* ****              		   	  ****
 * **** General purpose Functions ****
 * ****              		   	  ****
 */

ViewData *HexerFrame::getViewData() {
	switch (_view) {
		case kViewEdit:
			return _editData;
		case kViewDocs:
			return _docsData;
		case kViewHex:
		default:
			debug("_view not corrolating a view data");
			return nullptr;
	}
}

void HexerFrame::saveLocalEdit() {
	wxTextFile _editFile(wxString("local/editLocal"));
	if (!_editFile.Open()) {
		wxLogError(wxString("Could not load local edit file"));
		debug("couldn't load the local edit file");
		return;
	}

	_editFile.Clear();
	wxString catLine = "";
	for (int i = 0; i < _editData->_catNames.size(); i++) {
		catLine << _editData->_catNames[i];
		if (i != _editData->_catNames.size() - 1) {
			catLine << "|";
		}
	}	
	_editFile.AddLine(catLine);
	for (int i = 0; i < _editPatches.size(); i++) {
		for (int j = 0; j < _editPatches[i].size(); j++) {
			wxString editLine = "";
			editLine << i << "|";
			editLine << _editPatches[i][j]._name << "|";

			wxString sOffsets = "";
			wxString sOldBytes = "";
			wxString sNewBytes = "";
			debug(_editPatches[i][j]._bytes.size());
			for (int k = 0; k < _editPatches[i][j]._bytes.size(); k++) {
				debug("for every offset");
				// For every offset, we add an offset to the offset string
				sOffsets << wxString::Format("%X", _editPatches[i][j]._bytes[k]._offset);
				for (int l = 0; l < _editPatches[i][j]._bytes[k]._oldBytes.size(); l++) {
					debug("for every byte string");
					// And for every byte string, we add the byte string to old/newbytes
					sOldBytes << wxString::Format("%02X", _editPatches[i][j]._bytes[k]._oldBytes[l]);
					sNewBytes << wxString::Format("%02X", _editPatches[i][j]._bytes[k]._newBytes[l]);
				}
				if (k != _editPatches[i][j]._bytes.size() - 1) {
					// Separated by commas
					sOffsets << ",";
					sOldBytes << ",";
					sNewBytes << ",";
				}
			}
			editLine << sOffsets << "|";
			editLine << sNewBytes << "|";
			editLine << sOldBytes << "|";
			editLine << _editPatches[i][j]._desc;

			_editFile.AddLine(editLine);
		}
	}
	_editFile.Write();
}

void HexerFrame::saveLocalDocs() {
	wxTextFile _docsFile(wxString("local/docsLocal"));
	if (!_docsFile.Open()) {
		wxLogError(wxString("Could not load local docs file"));
		debug("couldn't load the local docs file");
		return;
	}

	// First we clear and rewrite the docs file
	_docsFile.Clear();
	wxString catLine = "";
	for (int i = 0; i < _docsData->_catNames.size(); i++) {
		catLine << _docsData->_catNames[i];
		if (i != _docsData->_catNames.size() - 1) {
			catLine << "|";
		}
	}
	_docsFile.AddLine(catLine);
	for (int i = 0; i < _docsEntries.size(); i++) {
		for (int j = 0; j < _docsEntries[i].size(); j++) {
			wxString docsLine = "";
			docsLine << i << "|";
			docsLine << _docsEntries[i][j]._name << "|";
			docsLine << _docsEntries[i][j]._addr << "|";
			docsLine << _docsEntries[i][j]._size << "|";
			docsLine << _docsEntries[i][j]._type << "|";
			docsLine << _docsEntries[i][j]._desc;
			_docsFile.AddLine(docsLine);
			debug(docsLine);
		}
	}
	_docsFile.Write();
}

int HexerFrame::YToRowGood(wxGrid *grid, int y) {
	/* For whatever reason, the actual YToRow() seems
	 * to be a little broken. So instead, we have a very
	 * simple and straightforward implementation of
	 * the same thing.
	 */
	int xUnit;
	int yUnit;
	grid->GetScrollPixelsPerUnit(&xUnit, &yUnit);		// Why is this done with pointers instead of returning a position tuple?? Gross	
	int yOff = grid->GetScrollPos(wxVERTICAL) * yUnit;
	int yPos;
	int nextY;

	// We literally just iterate over each row until the mouse position and the row position match
	y += yOff;
	yPos = 0;
	for (int i = 0; i < grid->GetNumberRows(); i++) {
		nextY = yPos + grid->GetRowSize(i);
		if ((yPos <= y) && (y <= nextY)) {
			return i;
		}
		yPos = nextY;
	}

	// And if they don't, we return -1 instead
	return -1;
}

/* ****              		       ****
 * **** Notebook related Functions ****
 * ****              		       ****
 */

void HexerFrame::onAddEntry(wxCommandEvent &event) {
	ViewData *data = getViewData();

	int cat = data->_noteBook->GetSelection();
	if (_view == kViewEdit) {
		addEntry(true, "", "", "", "", "", "", "");
	} else {
		if (data->_catNames[cat] != "Bitflags") {
			addEntry(true, "", "", "", "", "", "", "");		
		}
	}
}

void HexerFrame::onNotebookChange(wxBookCtrlEvent &event) {
	if (_view == kViewEdit) {
		_editData->_catName->SetLabel(_editData->_catNames[event.GetSelection()]);
	} else {
		_docsData->_catName->SetLabel(_docsData->_catNames[event.GetSelection()]);
	}
	SendSizeEvent();
}

void HexerFrame::onTextEnter(wxCommandEvent& event) {
	ViewData *data = getViewData();
	if (data->_catName->GetValue() != "Bitflags") {
		data->_catNames[data->_noteBook->GetSelection()] = data->_catName->GetValue();
		data->_noteBook->SetPageText(data->_noteBook->GetSelection(), data->_catName->GetValue());
		saveLocalDocs();
	}
}

void HexerFrame::onShowSidePanel(wxCommandEvent &event) {
	ViewData *data = getViewData();

	// Simple toggle to hide/show the panel
	if (data->_sidePanel->GetStaticBox()->IsShown()) {
		data->_sidePanel->GetStaticBox()->Hide();
		data->_showSidePanel->SetLabel("«");
	} else {
		data->_sidePanel->GetStaticBox()->Show(true);
		data->_showSidePanel->SetLabel("»");
	}
	SendSizeEvent();
}

void HexerFrame::createSidePanel(ViewData *data, wxPanel *view) {
	// We need a staticBoxSizer for the side panel
	data->_sidePanel = new wxStaticBoxSizer(wxVERTICAL, view, "Options");
	data->_sidePanel->GetStaticBox()->Hide();

	wxStaticBoxSizer *catBoxSizer = new wxStaticBoxSizer(wxVERTICAL, data->_sidePanel->GetStaticBox(), "Category");

	// And then a sizer for the category options
	wxBoxSizer *catSizer = new wxBoxSizer(wxHORIZONTAL);

	// And on the side panel, we have several things
	wxStaticText *strCat = new wxStaticText(catBoxSizer->GetStaticBox(), wxID_ANY, "Category ");
	data->_catName = new wxTextCtrl(catBoxSizer->GetStaticBox(), wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxDefaultValidator, wxEmptyString);
	data->_catName->Bind(wxEVT_TEXT, &HexerFrame::onTextEnter, this);
	wxButton *deleteCat = new wxButton(catBoxSizer->GetStaticBox(), wxID_ANY, "Delete Category");

	catSizer->Add(strCat, 0, wxALIGN_CENTER_VERTICAL);
	catSizer->Add(data->_catName, 1, wxGROW);

	catBoxSizer->Add(catSizer, 0, wxGROW | wxBOTTOM, 15);
	catBoxSizer->Add(deleteCat, 0, wxGROW | wxBOTTOM, 10);

	data->_sidePanel->Add(catBoxSizer, 0, wxGROW | wxBOTTOM, 15);

	data->_showSidePanel = new wxButton(view, wxID_ANY, "«", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxBU_EXACTFIT, wxDefaultValidator, wxEmptyString);
	data->_showSidePanel->Bind(wxEVT_BUTTON, &HexerFrame::onShowSidePanel, this);

}

void HexerFrame::onGridMouseExit(wxMouseEvent& event) {
	wxWindow *gridWindow = (wxWindow *) event.GetEventObject();
	wxGrid *grid = (wxGrid *) gridWindow->GetParent();

	if (_prevRow != -1) {
		grid->SetCellValue(_prevRow, 0, "");
		grid->SetCellValue(_prevRow, 1, "");
		_prevRow = -1;
	}
}

void HexerFrame::createPage(ViewData *data, wxString pageName, int row, int col) {
	/* A 'page' of the notebook consists of:
	 * - panel
	 * - grid
	 */

	// First step is to save the pagename in to the struct
	data->_catNames.push_back(pageName);

	// Next we set up all of the sizers and objects we need

	// Starting with the page panel itself
	wxPanel *page = new wxPanel(data->_noteBook);

	// And the sizers for the page
	wxBoxSizer *pageSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *buttonsSizer = new wxBoxSizer(wxHORIZONTAL);

	// And now the grid
	wxGrid *grid = new wxGrid(page, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, wxEmptyString);
	
	// And buttons
	wxCheckBox *editModeButton = new wxCheckBox(page, ID_EditMode, "Edit", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, wxEmptyString);
	  wxButton *addEntry = new wxButton(page, wxID_ANY, "Add", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxBU_EXACTFIT, wxDefaultValidator, "New Entry");
	  addEntry->SetBitmap(wxArtProvider::GetIcon(wxART_PLUS, wxART_BUTTON));

	// We need to be able to reference these things later
	data->_pages.push_back(page);
	data->_grids.push_back(grid);

	// We start with the button sizer so that it can be added cleanly into the pagesizer
	buttonsSizer->AddStretchSpacer();
	buttonsSizer->Add(addEntry, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 15);
	buttonsSizer->Add(editModeButton, 0, wxALIGN_CENTER_VERTICAL);

	/* Before creating the grid, we need to set the properties
	 * so that it looks like it isn't a grid (but we still get the
	 * performance benefit of the virtual gridtable backend)
	 */
	grid->EnableGridLines(false);

	// The headers are obviously not relevant in this one
	grid->SetRowLabelSize(0);
	grid->SetColLabelSize(0);

	// We don't want the user changing any of the row/col sizes
	grid->DisableDragColMove();
	grid->DisableDragRowMove();
	grid->DisableDragColSize();
	grid->DisableDragGridSize();
	grid->DisableDragRowSize();
	grid->DisableCellEditControl();
	grid->EnableEditing(false);

	// We don't want a highlight box around the entry
	grid->SetCellHighlightPenWidth(0);
	grid->SetCellHighlightROPenWidth(0);

	// We also want the alignment of everything to be centered
	grid->SetDefaultCellAlignment(wxALIGN_LEFT, wxALIGN_CENTER);

	// And lastly, we want the entries to be drawn directly on the panel, so we use a transparent background
	grid->SetDefaultCellBackgroundColour(wxColour(0,0,0,0));

	// Now we can actually create the grid
	grid->CreateGrid(row, col, wxGrid::wxGridSelectionModes::wxGridSelectNone);

	/* MoreInfo: Light grey question mark
	 * Edit: Image
	 * Delete: Red X (or maybe light grey?)
	 */
	wxGridCellAttr *moreInfoAttr = new wxGridCellAttr();
	moreInfoAttr->SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
	moreInfoAttr->SetTextColour(wxColour(*wxLIGHT_GREY));

	wxGridCellAttr *editButtonAttr = new wxGridCellAttr();
	editButtonAttr->SetAlignment(wxALIGN_RIGHT, wxALIGN_CENTER);
	editButtonAttr->SetRenderer(new ImageCellRenderer());

	wxGridCellAttr *deleteButtonAttr = new wxGridCellAttr();
	deleteButtonAttr->SetAlignment(wxALIGN_RIGHT, wxALIGN_CENTER);
	//deleteButtonAttr->SetTextColour(wxColour(*wxRED));
	deleteButtonAttr->SetTextColour(wxColour(*wxLIGHT_GREY));

	grid->SetColAttr(0, deleteButtonAttr);
	grid->SetColAttr(1, editButtonAttr);
	grid->SetColAttr(2, moreInfoAttr);

	// For most pages, we want a small border
	pageSizer->Add(grid, 1, wxGROW | wxALL, 4);
	pageSizer->Add(buttonsSizer, 0, wxGROW | wxALL, 10);
	page->SetSizer(pageSizer);

	// Now add the panel to the notebook
	data->_noteBook->AddPage(page, pageName);

	// Events:
	addEntry->Bind(wxEVT_BUTTON, &HexerFrame::onAddEntry, this);
	editModeButton->Bind(wxEVT_CHECKBOX, &HexerFrame::onEditModeButton, this);

	// The gridWindow is the actual window the grid is drawn onto, and is what captures mouse events
	wxWindow *gridWindow = grid->GetGridWindow();
	gridWindow->Bind(wxEVT_LEAVE_WINDOW, &HexerFrame::onGridMouseExit, this);

	if (_view == kViewEdit) {
		grid->Bind(wxEVT_GRID_CELL_LEFT_CLICK,  &HexerFrame::onEditGridLeftClick, this);
		grid->Bind(wxEVT_GRID_CELL_LEFT_DCLICK, &HexerFrame::onEditGridLeftClick, this);
		gridWindow->Bind(wxEVT_MOTION, 		 	&HexerFrame::onMouseInEditGrid,   this);

	} else {
		grid->Bind(wxEVT_GRID_CELL_LEFT_CLICK,  &HexerFrame::onDocsGridLeftClick,   this);
		grid->Bind(wxEVT_GRID_CELL_LEFT_DCLICK, &HexerFrame::onDocsGridDoubleClick, this);
		gridWindow->Bind(wxEVT_MOTION, 		 	&HexerFrame::onMouseInDocsGrid,     this);
	}
}

/* ****              		      ****
 * **** Button response Functions ****
 * ****              		      ****
 */
void HexerFrame::onEditModeButton(wxCommandEvent &event) {
	bool value = event.IsChecked();

	_editMode = !_editMode;

	for (int i = 0; i < _docsData->_grids.size(); i++) {
		wxPanel *dP = (wxPanel *) _docsData->_grids[i]->GetParent();
		wxCheckBox *dC = (wxCheckBox *) dP->FindWindow(ID_EditMode);
		dC->SetValue(value);
	}

	if (_rom != nullptr) {
		for (int i = 0; i < _editData->_grids.size(); i++) {
			wxPanel *eP = (wxPanel *) _editData->_grids[i]->GetParent();
			wxCheckBox *eC = (wxCheckBox *) eP->FindWindow(ID_EditMode);
			eC->SetValue(value);
		}
	}
}

void HexerFrame::onOpen(wxCommandEvent &event) {
	// Prompt the user to open a rom file
	wxFileDialog open(this, _("Open ROM file"), "", "", "", wxFD_OPEN|wxFD_FILE_MUST_EXIST);

	// If they decide to cancel, just return
	if (open.ShowModal() == wxID_CANCEL) {
		return;
	}

	wxString path = open.GetPath();

	if (!wxFile::Exists(path)) {
		wxLogError("Cannot open file");
		return;
	}

	// Get the rom loaded in
	_rom = new Rom(path);

	int temp = _view;

	// With a rom chosen, we can now populate the Edit view
	_view = kViewEdit;
	populateEditView();
	
	// And the Hex view
	_view = kViewHex;
	populateHexView();

	_view = temp;

	for (int i = 0; i < _docsData->_grids.size(); i++) {
		wxPanel *dP = (wxPanel *) _docsData->_grids[i]->GetParent();
		wxCheckBox *dC = (wxCheckBox *) dP->FindWindow(ID_EditMode);
		dC->SetValue(false);
	}

	for (int i = 0; i < _editData->_grids.size(); i++) {
		wxPanel *eP = (wxPanel *) _editData->_grids[i]->GetParent();
		wxCheckBox *eC = (wxCheckBox *) eP->FindWindow(ID_EditMode);
		eC->SetValue(false);
	}

	_mainSizer->Layout();
}

void HexerFrame::onSave(wxCommandEvent& event) {
	if (_rom != nullptr) {
		_rom->saveToRom();
	} else {
		debug("pressed save button but no rom was loaded");
	}
}
// ------------------------------------------------------------------

// ------------------------------------------------------------------
/* Other class overrides */

/* Image cell renderer code
 * This is just so that the edit button can exist in the grid.
 * To do so, we need to create a special grid cell renderer
 * that will draw an image instead of the string
 */
void ImageCellRenderer::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) {
	wxGridCellStringRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);
	if (grid.GetCellValue(row, col) != "") {
		wxIcon icon = wxArtProvider::GetIcon(wxART_FIND_AND_REPLACE, wxART_BUTTON);
		dc.DrawIcon(icon, rect.x + ((rect.width / 2) - (icon.GetLogicalWidth() / 2)), rect.y + ((rect.height / 2) - (icon.GetLogicalHeight() / 2)));
	}
}

/* Three-state bool renderer code
 * By default, the bool renderer only creates a two-state checkbox.
 * This code just adjusts it to make a three-state instead.
 */
void ThreeStateBoolEditor::Create(wxWindow* parent, wxWindowID id, wxEvtHandler* evtHandler) {
	m_control = new wxCheckBox(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxCHK_3STATE);
	wxGridCellEditor::Create(parent, id, evtHandler);
}

void ThreeStateBoolRenderer::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) {
	wxGridCellBoolRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);

    int hAlign = wxALIGN_LEFT;
    int vAlign = wxALIGN_CENTRE_VERTICAL;
    attr.GetNonDefaultAlignment(&hAlign, &vAlign);

	wxSize contentSize = GetBestSize(grid, attr, dc, row, col);
	const wxRect cellRect = rect;

    /* Here we have to reimplement a function because it is otherwise only available in the private generic header for grid */

	// the space between the cell edge and the checkbox mark
	const int GRID_CELL_CHECKBOX_MARGIN = 2;

    // Keep square aspect ratio for the checkbox, but ensure that it fits into
    // the available space, even if it's smaller than the standard size.
    const wxCoord minSize = wxMin(cellRect.width, cellRect.height);
    if ( contentSize.x >= minSize || contentSize.y >= minSize )
    {
        // It must still have positive size, however.
        const int fittingSize = wxMax(1, minSize - 2*GRID_CELL_CHECKBOX_MARGIN);

        contentSize.x =
        contentSize.y = fittingSize;
    }

    wxRect contentRect(contentSize);

    if ( hAlign & wxALIGN_CENTER_HORIZONTAL )
    {
        contentRect = contentRect.CentreIn(cellRect, wxHORIZONTAL);
    }
    else if ( hAlign & wxALIGN_RIGHT )
    {
        contentRect.SetX(cellRect.x + cellRect.width
                          - contentSize.x - GRID_CELL_CHECKBOX_MARGIN);
    }
    else // ( hAlign == wxALIGN_LEFT ) and invalid alignment value
    {
        contentRect.SetX(cellRect.x + GRID_CELL_CHECKBOX_MARGIN);
    }

    if ( vAlign & wxALIGN_CENTER_VERTICAL )
    {
        contentRect = contentRect.CentreIn(cellRect, wxVERTICAL);
    }
    else if ( vAlign & wxALIGN_BOTTOM )
    {
        contentRect.SetY(cellRect.y + cellRect.height
                          - contentSize.y - GRID_CELL_CHECKBOX_MARGIN);
    }
    else // wxALIGN_TOP
    {
        contentRect.SetY(cellRect.y + GRID_CELL_CHECKBOX_MARGIN);
    }

    const wxRect checkBoxRect = contentRect;
    int value;
    if ( grid.GetTable()->CanGetValueAs(row, col, wxGRID_VALUE_BOOL) )
    {
        value = (int) grid.GetTable()->GetValueAsBool(row, col);
    }
    else
    {
        wxString cellval( grid.GetTable()->GetValue(row, col) );
        if (cellval == "2") {
        	value = 2;
        } else {
			value = (int) wxGridCellBoolEditor::IsTrueValue(cellval);
        }
    }

    // Finally we can adjust this so that if the value is 2, it shows up as the undetermined option
    int flags = wxCONTROL_CELL;
    if (value == 1) {
		flags |= wxCONTROL_CHECKED;
    } else if (value == 2) {
    	flags |= wxCONTROL_UNDETERMINED;
    }

    wxRendererNative::Get().DrawCheckBox( &grid, dc, checkBoxRect, flags );
}

// ------------------------------------------------------------------

































