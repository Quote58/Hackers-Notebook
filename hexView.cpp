#include "hexer.h"
#include "rom.h"

// ------------------------------------------------------------------
/* ****               	    ****
 * **** The Hex Editor View ****
 * ****               	    ****
 * -> Frame 				[Frame]
 * -> ToolBar				[ToolBar]
 * -> mainSizer 			[BoxSizer V]
 *  \-> HexView 			[Panel]
 *	  \-> HexViewSizer 		[BoxSizer V]
 *		\-> UpperSizer		[BoxSizer H]
 *		  \-> GoToTxt		[StaticText]
 *		  \-> GoToCtrl		[TextCtrl]
 *		  \-> ViewTxt		[StaticText]
 *		  \-> ViewCtrl		[TextCtrl]
 *		\-> MidSizer		[BoxSizer H]
 *		\-> LowerSizer		[BoxSizer H]
 */

void HexerFrame::populateHexViewDefault() {
	wxBoxSizer *defaultSizer = new wxBoxSizer(wxVERTICAL);
	wxStaticText *loadRom = new wxStaticText(_hexView, wxID_ANY, "Load a rom to use the rom editor");
	defaultSizer->AddStretchSpacer();
	defaultSizer->Add(loadRom, 0, wxALIGN_CENTER_HORIZONTAL);
	defaultSizer->AddStretchSpacer();
	defaultSizer->Layout();
	_hexView->SetSizer(defaultSizer);
}

void HexerFrame::populateHexView() {
	// To allow the user to load a new rom, we want to destroy any existing hexView beforehand
	_hexView->DestroyChildren();

	// First thing needed is the main sizer, a regular vertical box sizer
	wxBoxSizer *hexViewSizer = new wxBoxSizer(wxVERTICAL);

	// Then we need a sizer for each section, as each one has multiple widgets
	wxBoxSizer *upperSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *midSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *lowerSizer = new wxBoxSizer(wxHORIZONTAL);

	/* Top section:
	 * The main GoTo widget
	 * A selector for which view type to use,
	 * A checkbox for gridlines
	 */

	// First the goTo box, which has a label text next to it
	wxStaticText *goToTxt  = new wxStaticText(_hexView, wxID_ANY, "GoTo Address ");
	  wxTextCtrl *goToCrtl = new wxTextCtrl(_hexView, wxID_ANY, "0000000", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxDefaultValidator, wxEmptyString);
				  goToCrtl->Bind(wxEVT_TEXT_ENTER, &HexerFrame::onGoToEnter, this);

	// Next is a choice box with the different view types, and a label text next to it
	wxString viewTypeChoices[4] = {"Bytes", "Strings", "Palettes", "Graphics"};
	wxStaticText *viewTypeTxt = new wxStaticText(_hexView, wxID_ANY, "View Rom As ");
	    wxChoice *viewTypeChoice = new wxChoice(_hexView, wxID_ANY, wxDefaultPosition, wxDefaultSize, 4, viewTypeChoices, 0, wxDefaultValidator, wxEmptyString);
			      viewTypeChoice->Bind(wxEVT_CHOICE, &HexerFrame::onViewTypeChoice, this);

	// Also a checkbox with it's associated label, for controlling the gridlines (default is on)
	_gridLines = new wxCheckBox(_hexView, wxID_ANY, "Gridlines", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, wxDefaultValidator, wxEmptyString);
	_gridLines->SetValue(true);
	_gridLines->Bind(wxEVT_CHECKBOX, &HexerFrame::onGridLinesCheck, this);

	// Last is a choice box with the different presets for different consoles, with a label next to it
	wxString presetChoices[7] = {"None", "Nintendo", "Master System", "Gameboy", "Super Nintendo", "Genesis", "Gameboy Advance"};
	wxStaticText *presetTxt = new wxStaticText(_hexView, wxID_ANY, "Console Preset ");
	    wxChoice *presetChoice = new wxChoice(_hexView, wxID_ANY, wxDefaultPosition, wxDefaultSize, 7, presetChoices, 0, wxDefaultValidator, wxEmptyString);
			      presetChoice->Bind(wxEVT_CHOICE, &HexerFrame::onPresetChoice, this);

	// Now they can all be added to the main upper sizer
	upperSizer->Add(goToTxt,        0, wxALIGN_CENTER_VERTICAL);
	upperSizer->Add(goToCrtl,       0, wxGROW);
	upperSizer->Add(viewTypeTxt,    0, wxLEFT | wxALIGN_CENTER_VERTICAL, 15);
	upperSizer->Add(viewTypeChoice, 0, wxGROW);
	upperSizer->Add(presetTxt,      0, wxLEFT | wxALIGN_CENTER_VERTICAL, 15);
	upperSizer->Add(presetChoice,   0, wxGROW);
	upperSizer->Add(_gridLines,     0, wxLEFT | wxALIGN_CENTER_VERTICAL, 15);

	/*********************/

	/* Mid Section:
	 * The hex editor itself
	 * The panels of properties and search functions for the different views
	 */

	// We need two separate grids because dumb generic/private/grid.h reasons <-- FIX THIS ONE DAY
	_headerSizer = new wxBoxSizer(wxVERTICAL);

	// This table data won't change, but the view of it will, so we need the createHexEditor to use _hexTable
	// We start the table off with the view type as bytes
	_hexTable = new RomEditorTable(_rom->_rom->Length(), _rom, kViewTypeBytes);

	// Now we can create the header (which won't be re-made) and the editor itself (which needs to be able to re-make itself)
	createHexEditorHeader();
	createHexEditor();

	// In order for the palette editor to work, we need to catch the event for a double click on the grid
	_hexGrid->Bind(wxEVT_GRID_CELL_LEFT_DCLICK, &HexerFrame::onHexViewDClick, this);

	// We also want to load up the default ascii string table
	loadDefaultStringTable();

	// And the default palette for both gfx and indexed palette views
	loadDefaultPalettes();

	// With the header and grid created, we can add them both to a sizer
	_headerSizer->Add(_header, 0, wxLEFT, 1);
	_headerSizer->Add(_hexGrid, 0);

	// We need a vertical sizer to contain the different staticboxes
	wxBoxSizer *viewTypeSizer = new wxBoxSizer(wxVERTICAL);

	// And we want a static box for each view type
	_stringPanel = new wxStaticBoxSizer(wxVERTICAL, _hexView, "String View");
	_stringPanel->GetStaticBox()->Hide();
	
	_palettePanel = new wxStaticBoxSizer(wxVERTICAL, _hexView, "Palette View");
	_palettePanel->GetStaticBox()->Hide();
	
	_gfxPanel = new wxStaticBoxSizer(wxVERTICAL, _hexView, "Graphics View");
	_gfxPanel->GetStaticBox()->Hide();
	
	_viewPanels[0] = _stringPanel;
	_viewPanels[1] = _palettePanel;
	_viewPanels[2] = _gfxPanel;

	wxStaticBoxSizer *controlPanel = new wxStaticBoxSizer(wxVERTICAL, _hexView, "Controls");
	wxStaticBoxSizer *selectionPanel = new wxStaticBoxSizer(wxVERTICAL, _hexView, "Selection");

	// String View includes:
	// A button for loading a new string table
	// A spin control for the size of the byte width
	// A search bar for searching by string

	// First we have a regular button
	wxButton *loadStringTable = new wxButton(_stringPanel->GetStaticBox(), wxID_ANY, "Load New Table");
			  loadStringTable->Bind(wxEVT_BUTTON, &HexerFrame::onChangeStringTable, this);
	
	// Next we need a horizontal sizer for the text + spin control
	wxBoxSizer *stringByteSizeSizer = new wxBoxSizer(wxHORIZONTAL);

	// And now the text + spin control
	wxStaticText *stringByteText  = new wxStaticText(_stringPanel->GetStaticBox(), wxID_ANY, "Byte Size ");
	_hexTable->_stringCtrl = new wxSpinCtrl(_stringPanel->GetStaticBox(), wxID_ANY, "1", wxDefaultPosition, wxDefaultSize, 0, 1, 4, 0, wxEmptyString);
	_hexTable->_stringCtrl->Bind(wxEVT_SPINCTRL, &HexerFrame::onStringByteSizeChanged, this);

	stringByteSizeSizer->Add(stringByteText, 0, wxALIGN_CENTER_VERTICAL);
	stringByteSizeSizer->Add(_hexTable->_stringCtrl);

	// And lastly, a search control for searching by string
	wxSearchCtrl *searchBar = new wxSearchCtrl(_stringPanel->GetStaticBox(), wxID_ANY, wxEmptyString);

	_stringPanel->Add(loadStringTable, 0, wxGROW | wxBOTTOM, 10);
	_stringPanel->Add(stringByteSizeSizer, 0, wxGROW | wxBOTTOM, 10);
	_stringPanel->Add(searchBar, 0, wxGROW | wxBOTTOM, 6);

	// Palette View includes:
	// A search by colour control (text + colour picker + search button)
	// A spin control for the bit depth of the colours
	// A checkbox for if the palette is indexed or rgb
	// A button for loading a new palette

	// First we sizers to hold each line of controls
	wxBoxSizer *clrSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *formatSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *indexSizer = new wxBoxSizer(wxHORIZONTAL);

	// Then the colour picker search
	      wxStaticText *clrTxt  = new wxStaticText(_palettePanel->GetStaticBox(), wxID_ANY, "Colour");
	wxColourPickerCtrl *clrCtrl = new wxColourPickerCtrl(_palettePanel->GetStaticBox(), wxID_ANY, wxColour(*wxBLACK), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, wxEmptyString);
					    clrCtrl->Bind(wxEVT_COLOURPICKER_CHANGED, &HexerFrame::onColourPickerChanged, this);

			  wxButton *clrFind = new wxButton(_palettePanel->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxBU_EXACTFIT, wxDefaultValidator, wxEmptyString);
					    clrFind->SetBitmap(wxArtProvider::GetIcon(wxART_FIND, wxART_FRAME_ICON));
					    clrFind->Bind(wxEVT_BUTTON, &HexerFrame::onColourSearch, this);

	// Followed by the format controls
	wxStaticText *formatTxt = new wxStaticText(_palettePanel->GetStaticBox(), wxID_ANY, "Bits per colour ");
	_hexTable->_formatCtrl = new wxSpinCtrl(_palettePanel->GetStaticBox(), wxID_ANY, "1", wxDefaultPosition, wxDefaultSize, 0, 1, 8, 0, wxEmptyString);
	_hexTable->_formatCtrl->Bind(wxEVT_SPINCTRL, &HexerFrame::onFormatChanged, this);

	// And lastly the index related controls
	_hexTable->_clrIndexCheck = new wxCheckBox(_palettePanel->GetStaticBox(), wxID_ANY, "Indexed", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, wxEmptyString);
	   wxButton *clrIndexLoad = new wxButton(_palettePanel->GetStaticBox(), wxID_ANY, "Load Palette", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, wxEmptyString);
	   			 clrIndexLoad->Bind(wxEVT_BUTTON, &HexerFrame::onLoadIndexedPalette, this);

	// Now we can add all of them to the sizer
	clrSizer->Add(clrTxt,  0, wxALIGN_CENTER_VERTICAL);
	clrSizer->Add(clrCtrl, 0, wxLEFT, 5);
	clrSizer->Add(clrFind, 0, wxLEFT, 5);

	formatSizer->Add(formatTxt, 0, wxALIGN_CENTER_VERTICAL);
	formatSizer->Add(_hexTable->_formatCtrl);

	indexSizer->Add(_hexTable->_clrIndexCheck, 0, wxALIGN_CENTER_VERTICAL);
	indexSizer->Add(clrIndexLoad, 0, wxLEFT, 5);

	_palettePanel->Add(clrSizer,    0, wxGROW | wxBOTTOM, 6);
	_palettePanel->Add(formatSizer, 0, wxGROW | wxBOTTOM, 6);
	_palettePanel->Add(indexSizer,  0, wxGROW | wxBOTTOM | wxALIGN_LEFT, 6);

	// Graphics View includes:
	// A spin control for the bit depth of the gfx
	// A checkbox for if the gfx are interwoven
	// Probably a checkbox for if the gfx are planar or not

	// These controls need a sizer
	wxBoxSizer *gfxSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *gfxPalSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *gfxTypeSizer = new wxBoxSizer(wxHORIZONTAL);

	// The controls themselves
	wxStaticText *gfxTxt = new wxStaticText(_gfxPanel->GetStaticBox(), wxID_ANY, "Bits per pixel (bpp) ");
	_hexTable->_gfxCtrl = new wxSpinCtrl(_gfxPanel->GetStaticBox(), wxID_ANY, "1", wxDefaultPosition, wxDefaultSize, 0, 1, 8, 0, wxEmptyString);
	_hexTable->_gfxCtrl->Bind(wxEVT_SPINCTRL, &HexerFrame::onGfxBitdepthChanged, this);

	wxStaticText *gfxPalTxt = new wxStaticText(_gfxPanel->GetStaticBox(), wxID_ANY, "Bits per colour ");
	_hexTable->_gfxPalCtrl = new wxSpinCtrl(_gfxPanel->GetStaticBox(), wxID_ANY, "1", wxDefaultPosition, wxDefaultSize, 0, 1, 8, 0, wxEmptyString);
	_hexTable->_gfxPalCtrl->Bind(wxEVT_SPINCTRL, &HexerFrame::onGfxPalChanged, this);

	wxButton *gfxLoadPal = new wxButton(_gfxPanel->GetStaticBox(), wxID_ANY, "Load Palette", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, wxEmptyString);
			  gfxLoadPal->Bind(wxEVT_BUTTON, &HexerFrame::onChangeGfxPalette, this);

	wxStaticText *gfxTypeTxt = new wxStaticText(_gfxPanel->GetStaticBox(), wxID_ANY, "Type ");
	wxString gfxTypeChoices[5] = {"Planar", "Planar (Composite)", "Linear", "Linear (Reversed)", "RGB"};
	_hexTable->_gfxType = new wxChoice(_gfxPanel->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, 5, gfxTypeChoices, 0, wxDefaultValidator, wxEmptyString);
	_hexTable->_gfxType->Bind(wxEVT_CHOICE, &HexerFrame::onGfxRefresh, this);

	// Add them to the sizers
	gfxSizer->Add(gfxTxt, 0, wxALIGN_CENTER_VERTICAL);
	gfxSizer->Add(_hexTable->_gfxCtrl);

	gfxPalSizer->Add(gfxPalTxt, 0, wxALIGN_CENTER_VERTICAL);
	gfxPalSizer->Add(_hexTable->_gfxPalCtrl);

	gfxTypeSizer->Add(gfxTypeTxt, 0, wxALIGN_CENTER_VERTICAL);
	gfxTypeSizer->Add(_hexTable->_gfxType);
	// And lastly add the sizer to the panel
	_gfxPanel->Add(gfxSizer, 0, wxGROW | wxBOTTOM, 6);
	_gfxPanel->Add(gfxTypeSizer, 0, wxGROW | wxBOTTOM, 6);
	_gfxPanel->Add(gfxPalSizer, 0, wxGROW | wxBOTTOM, 6);
	_gfxPanel->Add(gfxLoadPal, 0, wxGROW | wxBOTTOM, 6);

	// Control box includes:
	// 4 arrow buttons (and maybe other stuff?)
	// Relative jump??

	// We need a horizontal and vertical box to make them a cross
	wxBoxSizer *hBox = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *vBox = new wxBoxSizer(wxVERTICAL);

	// We need a button for each
	wxButton *upArrow = new wxButton(controlPanel->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_NOTEXT | wxBORDER_NONE | wxBU_EXACTFIT, wxDefaultValidator, wxEmptyString);
			  upArrow->SetBitmap(wxArtProvider::GetIcon(wxART_GO_UP, wxART_BUTTON));
			  upArrow->Bind(wxEVT_BUTTON,    &HexerFrame::onArrowUp,    this);

	wxButton *downArrow = new wxButton(controlPanel->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_NOTEXT | wxBORDER_NONE | wxBU_EXACTFIT, wxDefaultValidator, wxEmptyString);
			  downArrow->SetBitmap(wxArtProvider::GetIcon(wxART_GO_DOWN, wxART_BUTTON));
			  downArrow->Bind(wxEVT_BUTTON,  &HexerFrame::onArrowDown,  this);

	wxButton *leftArrow = new wxButton(controlPanel->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_NOTEXT | wxBORDER_NONE | wxBU_EXACTFIT, wxDefaultValidator, wxEmptyString);
			  leftArrow->SetBitmap(wxArtProvider::GetIcon(wxART_GO_BACK, wxART_BUTTON));
			  leftArrow->Bind(wxEVT_BUTTON,  &HexerFrame::onArrowLeft,  this);

	wxButton *rightArrow = new wxButton(controlPanel->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_NOTEXT | wxBORDER_NONE | wxBU_EXACTFIT, wxDefaultValidator, wxEmptyString);
			  rightArrow->SetBitmap(wxArtProvider::GetIcon(wxART_GO_FORWARD, wxART_BUTTON));
			  rightArrow->Bind(wxEVT_BUTTON, &HexerFrame::onArrowRight, this);

	// Now we add them to the sizer
	vBox->Add(upArrow);
	vBox->Add(downArrow);
	hBox->Add(leftArrow, 0, wxALIGN_CENTER_VERTICAL);
	hBox->Add(vBox);
	hBox->Add(rightArrow, 0, wxALIGN_CENTER_VERTICAL);

	// And finally to the box
	controlPanel->Add(hBox, 0, wxGROW | wxBOTTOM, 6);

	// Selection box includes:
	// Button to make document entry
	// Other stuff???
	wxButton *makeEntry = new wxButton(selectionPanel->GetStaticBox(), wxID_ANY, "Add Entry");
	selectionPanel->Add(makeEntry, 0, wxGROW | wxBOTTOM, 6);

	wxButton *exportData = new wxButton(selectionPanel->GetStaticBox(), wxID_ANY, "Export");
	selectionPanel->Add(exportData, 0, wxGROW | wxBOTTOM, 6);

	wxBoxSizer *controlSelect = new wxBoxSizer(wxHORIZONTAL);
	controlSelect->Add(controlPanel);
	controlSelect->Add(selectionPanel, 0, wxLEFT, 10);

	viewTypeSizer->Add(controlSelect, 0, wxTOP, 15);
	viewTypeSizer->Add(_stringPanel, 0, wxTOP, 15);
	viewTypeSizer->Add(_palettePanel, 0, wxTOP, 15);
	viewTypeSizer->Add(_gfxPanel, 0, wxTOP, 15);
	viewTypeSizer->AddStretchSpacer();

	// Now we can add the editor and side controls to the mid sizer
	midSizer->Add(_headerSizer, 0, wxGROW | wxRIGHT, 10);
	midSizer->Add(viewTypeSizer, 0, wxGROW | wxRIGHT, kMacMargins);

	/* Lower Section
	 * Text for the rom name
	 * Buttons
	 * Other stuff??? Address of currently selected cell maybe?
	 */

	// First we make a static text with the rom name
	wxStaticText *romName = new wxStaticText(_hexView, wxID_ANY, "Rom: " + _rom->_name);
	
	// We also have two buttons which need a sizer
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);

	// Now the buttons
	wxButton *showLog = new wxButton(_hexView, wxID_ANY, "Show Log");
	wxButton *createPatch = new wxButton(_hexView, wxID_ANY, "Create Patch");

	buttonSizer->Add(showLog, 0, wxRIGHT, 15);
	buttonSizer->Add(createPatch);

	// And add it to the lower sizer
	lowerSizer->Add(romName, 0, wxALIGN_CENTER_VERTICAL);
	lowerSizer->AddStretchSpacer();
	lowerSizer->Add(buttonSizer, 0, wxALIGN_CENTER_VERTICAL);

	/* All components
	 *
	 */

	// Now finally we can add together all three components of this view
	hexViewSizer->Add(upperSizer, 0, wxGROW | wxALL,  kMacMargins);
	hexViewSizer->Add(midSizer,   1, wxGROW | wxLEFT, kMacMargins);
	hexViewSizer->Add(lowerSizer, 0, wxGROW | wxALL,  kMacMargins + 5);

	// And layout the main sizers
	hexViewSizer->Layout();
	_hexView->SetSizer(hexViewSizer);
}

void HexerFrame::createHexEditorHeader() {
	// The grid begins with a header (because dumb private grid.h stuff HATE)
	_header = new wxGrid(_hexView, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	_header->SetRowLabelSize(0);
	_header->DisableDragColMove();
	_header->DisableDragRowMove();
	_header->DisableDragColSize();
	_header->DisableDragGridSize();
	_header->DisableDragRowSize();
	_header->DisableCellEditControl();
	_header->EnableEditing(false);
	_header->UseNativeColHeader(true);
	_header->ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_NEVER);
	_header->CreateGrid(0, 18, wxGrid::wxGridSelectionModes::wxGridSelectNone);

	// To get the width of the cells right without checking every single cell with autosize,
	// we need to know how long the string is and we give it a little extra space
	_hexTable->_byteSize = _header->GetTextExtent("0000");
	_hexTable->_offsetLabelSize = _header->GetTextExtent("Offset");

	// Now we can set the label size and value for each column of the header
	_header->SetColLabelValue(0, "Offset");
	for (int i = 1; i < 17; i++) {
		_header->SetColLabelValue(i, wxString::Format("%02X", i - 1));
		_header->SetColSize(i, _hexTable->_byteSize.GetWidth());
	}

	// There is one more column so that it matches the position of the scroll bar
	_header->SetColLabelValue(17, "");

	// Magic number /2 +1
	_header->SetColSize(17, (_hexTable->_byteSize.GetWidth() / 2) + 1);
}

void HexerFrame::createHexEditor() {
	// Now the actual grid
	_hexGrid = new wxGrid(_hexView, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_DOUBLE, wxEmptyString);
	_hexGrid->SetTable(_hexTable, false);

	// We need to set up some properties of the grid
	_hexGrid->SetDefaultCellAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
	_hexGrid->SetRowLabelSize(0);
	_hexGrid->SetColLabelSize(0);
	_hexGrid->DisableDragColMove();
	_hexGrid->DisableDragRowMove();
	_hexGrid->DisableDragColSize();
	_hexGrid->DisableDragGridSize();
	_hexGrid->DisableDragRowSize();

	// Scrolling should be based on the size of a row
	_hexGrid->SetScrollLineY(_hexGrid->GetDefaultRowSize());

	// The size of the offset box needs to be big enough to hold the largest offset, ie. the size of the file
	wxSize offsetNumSize = _header->GetTextExtent(wxString::Format("%lld", _rom->_rom->Length() / 16) << "00");
	int offsetSize = (offsetNumSize.GetWidth() >= _hexTable->_offsetLabelSize.GetWidth()) ? offsetNumSize.GetWidth() : _hexTable->_offsetLabelSize.GetWidth();
	_hexGrid->SetColSize(0, offsetSize);
	_header->SetColSize(0, offsetSize);

	// Now we set the column width to be the size of 2 digits
	for (int i = 1; i < 17; i++) {
		_hexGrid->SetColSize(i, _hexTable->_byteSize.GetWidth());
	}

	// The font for the offset needs to be uniform, so we need to set it to a teletype font
	wxFont f = _hexGrid->GetDefaultCellFont();
	f.SetFamily(wxFONTFAMILY_TELETYPE);
	f.SetWeight(wxFONTWEIGHT_EXTRABOLD);
	wxGridCellAttr* cellAttr = new wxGridCellAttr();
	cellAttr->SetFont(f);
	_hexGrid->SetColAttr(0, cellAttr);

	// We need to save the font size and cell size
	_hexTable->_fontSizeBytes = f.GetFractionalPointSize();
	_hexTable->_fontSizeChars = _hexTable->_fontSizeBytes + 2;
	_hexTable->_cellSizeBytes = _hexGrid->GetDefaultRowSize();
	_hexTable->_cellSizeGfx   = _hexGrid->GetColSize(1);

	// HATE HATE HATE HATE HATE
	// The grid does not set a correct minimum size when you change the column widths, so we have to
	// manually calculate the minimum width of the grid based on the number of columns and their sizes, + the scrollbar
	_hexGrid->SetMinSize(wxSize((offsetSize + (_hexTable->_byteSize.GetWidth() * 16) + (_hexTable->_byteSize.GetWidth() / 2) + 3), _hexGrid->GetMinHeight()));

	_hexGrid->Bind(wxEVT_SCROLLWIN_TOP, 		 &HexerFrame::onScrollEnd, this);
	_hexGrid->Bind(wxEVT_SCROLLWIN_BOTTOM, 		 &HexerFrame::onScrollEnd, this);
	_hexGrid->Bind(wxEVT_SCROLLWIN_PAGEUP, 		 &HexerFrame::onScrollEnd, this);
	_hexGrid->Bind(wxEVT_SCROLLWIN_PAGEDOWN, 	 &HexerFrame::onScrollEnd, this);
	_hexGrid->Bind(wxEVT_SCROLLWIN_THUMBRELEASE, &HexerFrame::onScrollEnd, this);
	_hexGrid->Bind(wxEVT_MOUSEWHEEL,			 &HexerFrame::onMouseWheel, this);
}

void HexerFrame::loadDefaultStringTable() {
	// Open the default string table
	wxString stringTableName = "tables/";

	wxTextFile prefFile("preferences");
	if (!prefFile.Open()) {
		wxLogError(wxString("Could not load preferences file"));
	
	} else {
		prefFile.GetFirstLine();
		stringTableName += prefFile.GetNextLine();
	}

	wxTextFile stringTable(stringTableName);
	if (!stringTable.Open()) {
		wxLogError(wxString("Could not load string table file"));

	} else {
		for (wxString line = stringTable.GetFirstLine(); !stringTable.Eof(); line = stringTable.GetNextLine()) {
			wxStringTokenizer lineTokenizer(line, "=");
			
			wxString key = lineTokenizer.GetNextToken();
			wxString value = lineTokenizer.GetNextToken();
			if (value.Cmp("EQU") == 0) {
				value = "=";
			}
			_hexTable->_stringTable[key] = value;
		}
	}
}

/*
 * General use functions
 */

/* This function simply rounds a bit depth to the nearest byte and returns the whole number of bytes
 */
int HexerFrame::calcByteSize(int numBits) {
	float w = 0;
	float n = ((float) (numBits)) / 8;	// 8 bits in a byte
	float f = std::modf(n, &w);			// Separate out the whole number and remainder

	int bytes = (int) w;
	if (f > 0) {
		bytes++;						// 'Round' by saying that if there is a remainder at all, we count that as a byte
	}

	return bytes;
}

/* This function handles anything that wants to make the grid go to a specific offset
 */
void HexerFrame::goToOffset(int offset) {
	if (offset < 0) {
		return;
	}
	
	int size = 16;

	switch (_hexTable->_viewType) {
	case kViewTypeChars:
		size = _hexTable->_stringByteSize * 16;
		break;

	case kViewTypePal:
		size = _hexTable->_palByteSize * 16;
		break;

	case kViewTypeGfx:
		size = _hexTable->_gfxByteSize * 16;

	default:
		break;
	}

	if ((offset + size) < _hexTable->_rom->_rom->Length()) {
		// We unfortunately need both X and Y for the function
		int scrollX;
		int scrollY;
		_hexGrid->GetScrollPixelsPerUnit(&scrollX, &scrollY);

		// Before we divide the offset to get the number of rows, we want to save the number for later use
		_hexTable->_offset = offset;

		// Now we take the int of their hex string, and divide it by the number of bytes per line
		offset = offset / size;

		// And finally we can multiply the line offset by the line size, diving by the scroll increment (which is the same as the row size anyway in this case)
		_hexGrid->Scroll(0, (offset * _hexGrid->GetDefaultRowSize() / scrollY));
		_hexGrid->ForceRefresh();
	}
}

/* Similar to gotoOffset, this is meant to catch up _hexTable->_offset to the currently scrolled position
 */
void HexerFrame::adjustForScroll() {
	int offset = 16;

	switch (_hexTable->_viewType) {
	case kViewTypeChars:
		offset *= _hexTable->_stringByteSize;
		break;

	case kViewTypePal:
		offset *= _hexTable->_palByteSize;
		break;

	case kViewTypeGfx:
		offset *= _hexTable->_gfxByteSize;
		break;

	case kViewTypeBytes:
	default:
		break;
	}

	_hexTable->_offset = _hexTable->_offset + ((_hexGrid->GetFirstFullyVisibleRow() - (_hexTable->_offset / offset)) * offset);
}

void HexerFrame::loadDefaultPalettes() {
	if (!wxFile::Exists("palettes/greyscale.pal")) {
		wxLogError("Missing default palette file");
		return;
	}

	wxTextFile newPalette("palettes/greyscale.pal");
	if (!newPalette.Open()) {
		wxLogError(wxString("Could not load palette file"));
		return;

	} else {
		_hexTable->_indexedPalette.clear();
		_hexTable->_gfxPalette.clear();
		for (wxString line = newPalette.GetFirstLine(); !newPalette.Eof(); line = newPalette.GetNextLine()) {
			wxStringTokenizer colourTokenizer(line, ",");

			int red   = 0;
			int green = 0;
			int blue  = 0;
			sscanf(colourTokenizer.GetNextToken().c_str(), "%x", &red);
			sscanf(colourTokenizer.GetNextToken().c_str(), "%x", &green);
			sscanf(colourTokenizer.GetNextToken().c_str(), "%x", &blue);
			wxColour c(red, green, blue);

			_hexTable->_gfxPalette.push_back(c);
			_hexTable->_indexedPalette.push_back(c);
		}
	}
	_hexGrid->ForceRefresh();
}

void HexerFrame::loadPalette(wxVector<wxColour> &targetPal) {
	// This should check how many colours are in the file, and fill the remaining positions with black up to 8 * 8 colours for 8bpp


	// Prompt the user to open a rom file
	wxFileDialog open(this, _("Open Palette File"), "", "", "", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	// If they decide to cancel, just return
	if (open.ShowModal() == wxID_CANCEL) {
		return;
	}

	wxString path = open.GetPath();

	if (!wxFile::Exists(path)) {
		wxLogError("Cannot open file");
		return;
	}

	wxTextFile newPalette(path);
	if (!newPalette.Open()) {
		wxLogError(wxString("Could not load palette file"));
		return;

	} else {
		_hexTable->_gfxPalette.clear();
		for (wxString line = newPalette.GetFirstLine(); !newPalette.Eof(); line = newPalette.GetNextLine()) {
			wxStringTokenizer colourTokenizer(line, ",");

			int red   = 0;
			int green = 0;
			int blue  = 0;
			sscanf(colourTokenizer.GetNextToken().c_str(), "%x", &red);
			sscanf(colourTokenizer.GetNextToken().c_str(), "%x", &green);
			sscanf(colourTokenizer.GetNextToken().c_str(), "%x", &blue);
			wxColour c(red, green, blue);

			_hexTable->_gfxPalette.push_back(c);
		}

		if (_hexTable->_gfxPalette.size() < 64) {
			for (int i = _hexTable->_gfxPalette.size(); i < 64; i++) {
				_hexTable->_gfxPalette.push_back(wxColour(0,0,0));
			}
		}
	}
	_hexGrid->ForceRefresh();
}

/*
 * Top Level Widget bound functions
 */

/* When enter is pressed on the GoTo widget
 */
void HexerFrame::onGoToEnter(wxCommandEvent &event) {
	// And we need offset to be 0 in case they enter an invalid string
	int offset = 0;
	sscanf(event.GetString().c_str(), "%x", &offset);
	goToOffset(offset);
}

/* This controls the view type of the grid
 */
void HexerFrame::onViewTypeChoice(wxCommandEvent &event) {
	if (_hexTable->_viewType != kViewTypeBytes) {
		_viewPanels[_hexTable->_viewType - 1]->GetStaticBox()->Hide();
	}
	_hexTable->_viewType = event.GetSelection();
	if (_hexTable->_viewType != kViewTypeBytes) {
		_viewPanels[_hexTable->_viewType - 1]->GetStaticBox()->Show(true);
	}
	refreshEditor();
}

/* This applies a preset to the settings
 */
void HexerFrame::onPresetChoice(wxCommandEvent &event) {
	debug(event.GetSelection());
}

/* This checkbox just controls the gridlines of the grid
 */
void HexerFrame::onGridLinesCheck(wxCommandEvent &event) {
	_hexGrid->EnableGridLines(event.GetInt());
}


/*
 * Mid Level widgets bound functions
 */

/* Grid related functions
 */
void HexerFrame::onScrollEnd(wxScrollWinEvent &event) {
	ProcessEvent(event);
	adjustForScroll();
}

void HexerFrame::onMouseWheel(wxMouseEvent &event) {
	ProcessEvent(event);
	adjustForScroll();
}

/* This will destroy and remake the grid, and then update everything based on which view is active
 */
void HexerFrame::refreshEditor() {
	double fontSize = _hexTable->_fontSizeBytes;
	int cellSize = _hexTable->_cellSizeBytes;

	// Destroy and recreate the grid
	_hexGrid->Destroy();
	createHexEditor();

	// Add it back to the sizer
	_headerSizer->Add(_hexGrid, 0);

	// Re-bind the double click
	_hexGrid->Bind(wxEVT_GRID_CELL_LEFT_DCLICK, &HexerFrame::onHexViewDClick, this);

	// Make sure it actually shows up
	_mainSizer->Layout();

	// We need to reset the grid to the last offset it was on
	goToOffset(_hexTable->_offset);

	// And if the gridlines were on/off before, we want them on/off now
	_hexGrid->EnableGridLines(_gridLines->GetValue());

	switch (_hexTable->_viewType) {
	case kViewTypeBytes:
		updateGridProps(fontSize, cellSize, 1, "0000");
		break;

	case kViewTypeChars:
		_hexTable->_palByteSize = _hexTable->_stringCtrl->GetValue();
		updateGridProps(_hexTable->_fontSizeChars, cellSize, _hexTable->_stringByteSize, "00");
		break;

	case kViewTypePal:
		_hexTable->_palByteSize = calcByteSize(_hexTable->_formatCtrl->GetValue() * 3);
		updateGridProps(fontSize, cellSize, _hexTable->_palByteSize, "0000");
		break;

	case kViewTypeGfx:
		// 64 pixels in an 8x8 square
		_hexTable->_gfxByteSize = calcByteSize(_hexTable->_gfxCtrl->GetValue() * 64);
		updateGridProps(fontSize, _hexTable->_cellSizeGfx, _hexTable->_gfxByteSize, "00");

	default:
		break;
	}
}

/* This and updateGridProps are sort of connected to refreshGrid, but they are separate
 * because they can be used separately
 */
void HexerFrame::updateColLabels(int size) {
	for (int i = 1; i < 17; i++) {
		_header->SetColLabelValue(i, wxString::Format("%02X", ((i - 1) * size)));
	}
}

void HexerFrame::updateGridProps(int fontSize, int cellSize, int byteSize, wxString colWidth) {
	for (int i = 1; i < 17; i++) {
		wxGridCellAttr* cellAttr = _hexGrid->GetOrCreateCellAttr(0, i);

		wxFont f = cellAttr->GetFont();
		f.SetFractionalPointSize(fontSize);
		cellAttr->SetFont(f);

		wxGridCellRenderer *renderer;
		switch (_hexTable->_viewType) {
		case kViewTypePal:
			renderer = new RomEditorPalRenderer();
			break;

		case kViewTypeGfx:
			renderer = new RomEditorGfxRenderer();
			break;

		default:
			renderer = _hexGrid->GetDefaultRendererForCell(0, i);
			break;
		}
		cellAttr->SetRenderer(renderer);
		_hexGrid->SetColAttr(i, cellAttr);
	}

	_hexGrid->SetDefaultRowSize(cellSize);
	_hexGrid->SetScrollLineY(cellSize);

	updateColLabels(byteSize);
	_hexGrid->ForceRefresh();
}

/* When you double click a cell in palette view, the grid brings up a colour picker dialog
 * which lets you edit or choose a new colour, converting between different bit depths automatically
 */
void HexerFrame::onHexViewDClick(wxGridEvent& event) {
	// If we're in palette view and not clicking on the offset column
	if ((_hexTable->_viewType == kViewTypePal) && (event.GetCol() != 0)) {
		int row = event.GetRow();
		int col = event.GetCol();
		int bitDepth = _hexTable->_formatCtrl->GetValue();

		// We want to grab the string of the current cell colour
		wxStringTokenizer colourTokenizer(_hexGrid->GetCellValue(row, col), " ");

		// And extract the colour data
		wxString inputRed   = colourTokenizer.GetNextToken();
		wxString inputGreen = colourTokenizer.GetNextToken();
		wxString inputBlue  = colourTokenizer.GetNextToken();

		int red;
		int green;
		int blue;

		sscanf(inputRed.c_str(),   "%2x", &red);
		sscanf(inputGreen.c_str(), "%2x", &green);
		sscanf(inputBlue.c_str(),  "%2x", &blue);

		// This colour data is then converted from whatever bitdepth it currently is, into 24bit for the colour picker
		red   = trunc(float(red)   / float((1 << bitDepth) - 1) * float((1 << 8) - 1) + 0.5f);
		green = trunc(float(green) / float((1 << bitDepth) - 1) * float((1 << 8) - 1) + 0.5f);
		blue  = trunc(float(blue)  / float((1 << bitDepth) - 1) * float((1 << 8) - 1) + 0.5f);

		// To start the dialog with a certain colour, we need to put it into a colourData object
		wxColourData *cellClrData = new wxColourData();
		cellClrData->SetColour(wxColour(red, green, blue));

		// Show the colour dialog with the cell colour
		wxColourDialog *clrDialog = new wxColourDialog(_hexView, cellClrData);
		clrDialog->ShowModal();

		// Get the current colour data from the dialog
		wxColourData clrData = clrDialog->GetColourData();
		wxColour newClr = clrData.GetColour();

		// Now that we have our colour, we can delete the dialog
		clrDialog->Destroy();

		// If the current colours have changed at all from what the colour picker was given, we want to write the new ones into the cell
		if ((red != newClr.Red()) || (green != newClr.Green()) || (blue != newClr.Blue())) {
			// Convert it back down to whatever bitdepth it started as
			int newR = trunc(float(newClr.Red())   / float((1 << 8) - 1) * float((1 << bitDepth) - 1) + 0.5f);
			int newG = trunc(float(newClr.Green()) / float((1 << 8) - 1) * float((1 << bitDepth) - 1) + 0.5f);
			int newB = trunc(float(newClr.Blue())  / float((1 << 8) - 1) * float((1 << bitDepth) - 1) + 0.5f);

			// And finally create a string containing these new colours
			wxString clrString = wxString::Format("%02X", newR) + "," + wxString::Format("%02X", newG) + "," + wxString::Format("%02X", newB);
			_hexGrid->SetCellValue(row, col, clrString);
		}
	}
}

/* String panel functions
 */
void HexerFrame::onChangeStringTable(wxCommandEvent &event) {
	// Prompt the user to open a rom file
	wxFileDialog open(this, _("Open String Table File"), "", "", "", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	// If they decide to cancel, just return
	if (open.ShowModal() == wxID_CANCEL) {
		return;
	}

	wxString path = open.GetPath();

	if (!wxFile::Exists(path)) {
		wxLogError("Cannot open file");
		return;
	}

	wxTextFile stringTable(path);
	if (!stringTable.Open()) {
		wxLogError(wxString("Could not load string table file"));
		return;

	} else {
		_hexTable->_stringTable.clear();
		int byteSize = 1;
		for (wxString line = stringTable.GetFirstLine(); !stringTable.Eof(); line = stringTable.GetNextLine()) {
			wxStringTokenizer lineTokenizer(line, "=");
			
			wxString key = lineTokenizer.GetNextToken();
			byteSize = key.size() / 2;
			wxString value = lineTokenizer.GetNextToken();
			if (value.Cmp("EQU") == 0) {
				value = "=";
			}

			_hexTable->_stringTable[key] = value;

		}
		_hexTable->_stringByteSize = byteSize;
		_hexTable->_stringCtrl->SetValue(byteSize);
	}
	refreshEditor();
}

void HexerFrame::onStringByteSizeChanged(wxSpinEvent &event) {
	_hexTable->_stringByteSize = event.GetPosition();
	if (_hexTable->_viewType == kViewTypeChars) {
		refreshEditor();
	}
}

/* Palette panel functions
 */
void HexerFrame::onColourPickerChanged(wxColourPickerEvent &event) {
	int red = (event.GetColour()).GetRed();
	int green = (event.GetColour()).GetGreen();
	int blue = (event.GetColour()).GetBlue();
}

void HexerFrame::onFormatChanged(wxSpinEvent &event) {
	_hexTable->_palByteSize = calcByteSize(_hexTable->_formatCtrl->GetValue() * 3);
	if (_hexTable->_viewType == kViewTypePal) {
		refreshEditor();
	}
}

void HexerFrame::onLoadIndexedPalette(wxCommandEvent &event) {
	loadPalette(_hexTable->_indexedPalette);
}

void HexerFrame::onColourSearch(wxCommandEvent &event) {}

/* Gfx panel functions
 */
void HexerFrame::onGfxBitdepthChanged(wxSpinEvent &event) {
	_hexTable->_gfxByteSize = calcByteSize(_hexTable->_gfxCtrl->GetValue() * 64);
	if (_hexTable->_viewType == kViewTypeGfx) {
		refreshEditor();
	}
}

void HexerFrame::onChangeGfxPalette(wxCommandEvent &event) {
	loadPalette(_hexTable->_gfxPalette);
}

void HexerFrame::onGfxPalChanged(wxSpinEvent &event) {
	_hexGrid->ForceRefresh();
}

void HexerFrame::onGfxRefresh(wxCommandEvent &event) {
	_hexGrid->ForceRefresh();
}

/* Controls panel functions
 */
void HexerFrame::onArrowUp(wxCommandEvent &event) {
	if ((_hexTable->_offset - 16) >= 0) {
		goToOffset(_hexTable->_offset - 16);
	}
}

void HexerFrame::onArrowDown(wxCommandEvent &event) {
	if ((_hexTable->_offset + 16) < _hexTable->_rom->_rom->Length()) {
		goToOffset(_hexTable->_offset + 16);
	}
}

void HexerFrame::onArrowLeft(wxCommandEvent &event) {
	if ((_hexTable->_offset - 1) >= 0) {
		goToOffset(_hexTable->_offset - 1);
	}
}

void HexerFrame::onArrowRight(wxCommandEvent &event) {
	if ((_hexTable->_offset + 1) < _hexTable->_rom->_rom->Length()) {
		goToOffset(_hexTable->_offset + 1);
	}
}






