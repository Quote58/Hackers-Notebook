#include "hexer.h"

// ------------------------------------------------------------------
/* ****               	  ****
 * **** The Document View ****
 * ****               	  ****
 * -> Frame 				[Frame]
 * -> ToolBar				[ToolBar]
 * -> mainSizer 			[BoxSizer V]
 *  \-> DocsView 			[Panel]
 *	  |\-> DocsViewSizer 	[BoxSizer V]
 *	  |  \-> DocsBook 		[Notebook]
 *	  |    \-> Panel		[Panel]
 *	  |	     \-> PageSizer	[BoxSizer V]
 *	  |	       \-> Grid		[Grid]
 *	  |	       \-> Delete	[Button] (hidden)
 *	  |	       \-> Edit		[Button] (hidden)
 *	  |	       \-> MoreInfo	[StaticText] (hidden)
 *	  \--> LowerSizer 		[BoxSizer H]
 *		 \-> AddEntry 		[Button]
 */
void HexerFrame::populateDocsView() {
	// Since we can load a new documentation file after one is already loaded, we need to clear everything from the docs view panel first
	_docsView->DestroyChildren();

	// We need to load the tweaks file before populating the panel in case the file can't be loaded
	wxTextFile _docsFile(wxString("local/docsLocal"));
	if (!_docsFile.Open()) {
		wxLogError(wxString("Could not load local docs file"));
		_docsView->Show(false);
		return;
	}

	// We are starting a new ViewData
	_docsData = new ViewData();

	// Primary sizer for the panel
	wxBoxSizer *docsViewSizer = new wxBoxSizer(wxVERTICAL);

	// We need a sizer for the notebook + side panel, and one for the lower buttons
	wxBoxSizer *midSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *lowerSizer = new wxBoxSizer(wxHORIZONTAL);

	// Firstly, we need a notebook
	_docsData->_noteBook = new wxNotebook(_docsView, wxID_ANY);
	_docsData->_noteBook->Bind(wxEVT_BOOKCTRL_PAGE_CHANGING, &HexerFrame::onNotebookChange, this);

	createSidePanel(_docsData, _docsView);

	// And then sizers for the inside of the panel
	wxStaticBoxSizer *docsOptionsExport = new wxStaticBoxSizer(wxVERTICAL, _docsData->_sidePanel->GetStaticBox(), "Export");

	// And on the side panel, we have several things
	wxButton *addToNB = new wxButton(_docsData->_sidePanel->GetStaticBox(), wxID_ANY, "Add Document");
	wxButton *exportPage = new wxButton(docsOptionsExport->GetStaticBox(), wxID_ANY, "Export Page");
	wxButton *exportAll = new wxButton(docsOptionsExport->GetStaticBox(), wxID_ANY, "Export Notebook");

	docsOptionsExport->Add(exportPage, 0, wxGROW | wxBOTTOM, 15);
	docsOptionsExport->Add(exportAll, 0, wxGROW | wxBOTTOM, 5);

	_docsData->_sidePanel->Insert(0, addToNB, 0, wxGROW | wxBOTTOM, 15);
	_docsData->_sidePanel->Add(docsOptionsExport, 0, wxGROW | wxBOTTOM, 5);

	// On the lower sizer we just have two buttons
	wxButton *loadNewDocs = new wxButton(_docsView, wxID_ANY, "New docs file...", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "New Entry");

	// This has to be set to proportion '1' so that it fills the empty space in the panel
	midSizer->Add(_docsData->_noteBook, 1, wxGROW | wxTOP, kMacMargins);
	midSizer->Add(_docsData->_sidePanel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 15);
	midSizer->Add(_docsData->_showSidePanel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5);

	lowerSizer->Add(loadNewDocs);

	docsViewSizer->Add(midSizer, 1, wxGROW | wxLEFT, kMacMargins + 5);
	docsViewSizer->Add(lowerSizer, 0, wxALIGN_RIGHT | wxALL, kMacMargins + 5);

	/* 
	 * ---- Part 1 ----
	 */
	// The first line in the local files define the categories
	wxString categories = _docsFile.GetFirstLine();
	debug(categories);

	// In C++, to break up a string we need a tokenizer
	wxStringTokenizer catTokenizer(categories, "|");
	int i = 0;
	while (catTokenizer.HasMoreTokens()) {
		// The page names are the category names
		createPage(_docsData, catTokenizer.GetNextToken(), 1, 7);
		i++;
	}

	// Now that the grids are created, we can make the category text control use the default category label
	_docsData->_catName->SetLabel(_docsData->_catNames[0]);

	/* 
	 * ---- Part 2 ----
	 */

	// Okay, now we can start to iterate over the entire file line by line
	int numPages = _docsData->_pages.size();
	wxVector<int> lineID(numPages);
	_docsEntries = wxVector2D<Entry>(numPages);

	// Now we need to fill in the 0 line, which is the header
	for (int i = 0; i < _docsData->_grids.size(); i++) {
		if (_docsData->_catNames[i] != "Bitflags") {
			_docsData->_grids[i]->SetCellValue(0, 3, "Address");
			_docsData->_grids[i]->SetCellValue(0, 4, "Size (b)");
			_docsData->_grids[i]->SetCellValue(0, 5, "Type");
			_docsData->_grids[i]->SetCellValue(0, 6, "Description");
		}
	}

	// For the bitflags page, we need a monotype font
	wxFont fontBitFlags = _docsData->_grids[0]->GetDefaultCellFont();
	fontBitFlags.SetFamily(wxFONTFAMILY_TELETYPE);
	fontBitFlags.SetStyle(wxFONTSTYLE_NORMAL);
	fontBitFlags.SetWeight(wxFONTWEIGHT_NORMAL);

	for (wxString line = _docsFile.GetNextLine(); !_docsFile.Eof(); line = _docsFile.GetNextLine()) {
		// Each line is stored in the 2d Vector within a struct
		Entry lineDocEntry;

		// Each line is defined as: category|title|description|offsets|new bytes|old bytes
		wxStringTokenizer lineTokenizer(line, "|");

		// The category is an int, so we have to convert it
		int lineCat = wxAtoi(lineTokenizer.GetNextToken());
	
		// The rest of the components are all strings
		lineDocEntry._cat = lineCat;
		lineDocEntry._name = lineTokenizer.GetNextToken();
		lineDocEntry._addr = lineTokenizer.GetNextToken();
		lineDocEntry._size = lineTokenizer.GetNextToken();
		lineDocEntry._type = lineTokenizer.GetNextToken();
		lineDocEntry._desc = lineTokenizer.GetNextToken();

		// To properly capture multi-line descriptions, we will use the system newline chars
		// Which means the whole string is contained in "" instead of always being one line per entry
		if (lineDocEntry._desc[0] == '\"') {
			wxString longLine = line;
			while (longLine[longLine.size() - 1] != '\"') {
				longLine = _docsFile.GetNextLine();
				lineDocEntry._desc << '\n' << longLine;
			}
		}

		// Finally we can add the document line to the vector
		_docsEntries[lineCat].push_back(lineDocEntry);

		lineID[lineCat]++;

		/* Bitflags is a special category that displays it's information very differently
		 */
		if (_docsData->_catNames[lineCat] == "Bitflags") {
			wxString addrLine = "";
			wxString zeroLine = "";
			int size = wxAtoi(lineDocEntry._size) * 4;	// 4 bits in a byte, size is number of bytes
			prepBitflagDraw(size, lineDocEntry._type, lineDocEntry._addr, addrLine, zeroLine);
			drawBitflagDiagram(size, lineDocEntry._desc, lineCat, lineID[lineCat], fontBitFlags, addrLine, zeroLine, lineDocEntry._name);
			lineID[lineCat] += 3 + size;

		/* The rest of the categories just get added like normal
		 */
		} else {
			_docsData->_grids[lineCat]->InsertRows(lineID[lineCat], 1, true);
			if (lineDocEntry._desc != "") {
				_docsData->_grids[lineCat]->SetCellValue(lineID[lineCat], 2, "?");
			}
			_docsData->_grids[lineCat]->SetCellValue(lineID[lineCat], 3, lineDocEntry._addr);
			_docsData->_grids[lineCat]->SetCellValue(lineID[lineCat], 4, lineDocEntry._size);
			_docsData->_grids[lineCat]->SetCellValue(lineID[lineCat], 5, lineDocEntry._type);
			_docsData->_grids[lineCat]->SetCellValue(lineID[lineCat], 6, lineDocEntry._name);
		}
	}

	// And now just shrink everything to fit the contents
	for (int i = 0; i < _docsData->_grids.size(); i++) {
		_docsData->_grids[i]->AutoSize();
	}

	// And the sizer to the panel
	docsViewSizer->Layout();
	_docsView->SetSizer(docsViewSizer);
}

void HexerFrame::onMouseInDocsGrid(wxMouseEvent& event) {
	if (_editMode) {
		wxWindow *w = (wxWindow *) event.GetEventObject();
		wxGrid *grid = (wxGrid *) w->GetParent();
		wxPoint pos = event.GetPosition();
		int row = YToRowGood(grid, pos.y);
		int cat = _docsData->_noteBook->GetSelection();

		if (_docsData->_catNames[cat] != "Bitflags") {
			if ((_prevRow != row) && (row != -1) && (row > 0)) {
				if (_prevRow != -1) {
					grid->SetCellValue(_prevRow, 0, "");
					grid->SetCellValue(_prevRow, 1, "");
				}
				grid->SetCellValue(row, 0, "X");
				grid->SetCellValue(row, 1, "E");
				_prevRow = row;
			}
		}
	}
}

void HexerFrame::onDocsGridDoubleClick(wxGridEvent &event) {
	if ((event.GetCol() == 3) && (_rom != nullptr)) {
		int cat = _docsData->_noteBook->GetSelection();
		_curRow = event.GetRow();

		if (_curRow == 0) {
			return;
		
		} else {
			_curRow--;
		}

		wxGrid *grid = (wxGrid *) event.GetEventObject();
		if (_docsData->_catNames[cat] != "Bitflags") {
			wxString address = _docsEntries[cat][_curRow]._addr;
			if (address.Find('$') != wxNOT_FOUND) {
				address = address.Mid(1);
				debug(address);
			}
			int offset = 0;
			sscanf(address.c_str(), "%x", &offset);
			// This function is primarily used by the hex editor, but it is a method of the frame so that other
			// things like this can change the offset of the editor
			goToOffset(offset);

			// Now that the offset is set up, we need to switch the view to the editor
			for (int i = 0; i < 3; i++) {
				if (i == kViewHex) {
					_views[i]->Show(true);
					_toggleViews[i]->SetValue(true);
					_view = i;
				} else {
					_views[i]->Show(false);
					_toggleViews[i]->SetValue(false);
				}
				_prevRow = -1;
			}
			_mainSizer->Layout();
		}
	}
}

void HexerFrame::onDocsGridLeftClick(wxGridEvent &event) {
	/* Left click event:
	 * We want to do two things here. 1. We want to
	 * eat the click event. This way, the grid doesn't
	 * try to do any sort of selection on the content.
	 * 2. Handle moreInfo/delete buttons
	 */
	int cat = _docsData->_noteBook->GetSelection();
	_curRow = event.GetRow();

	if (_curRow == 0) {
		return;
	
	} else {
		_curRow--;
	}
	
	wxGrid *grid = (wxGrid *) event.GetEventObject();

	if (_docsData->_catNames[cat] != "Bitflags") {
		// Bitflags will need to be handled separately and uniquely because only certain rows are useful
		if (event.GetCol() == 2) {
			// Click was on the moreInfo button
			wxString s = _docsEntries[cat][_curRow]._desc;
			if (s != "") {
				moreInfo(s, true);
			}

		} else if ((event.GetCol() == 0) && _editMode) {
			// Click was on the delete entry button
			wxMessageDialog *askDelete = new wxMessageDialog(this, "You can't undo this action.", "Are you sure you want to permenantly destroy the item?", wxYES_NO | wxCENTRE | wxICON_WARNING, wxDefaultPosition);
			askDelete->Bind(wxEVT_WINDOW_MODAL_DIALOG_CLOSED, &HexerFrame::askDeleteClosedDocs, this);
			askDelete->ShowWindowModal();

		} else if ((event.GetCol() == 1) && _editMode) {
			// Click was on the edit entry button
			addEntry(false, _docsEntries[cat][_curRow]._name, _docsEntries[cat][_curRow]._desc, _docsEntries[cat][_curRow]._size, _docsEntries[cat][_curRow]._type, _docsEntries[cat][_curRow]._addr, "", "");
		}
	}
}

void HexerFrame::askDeleteClosedDocs(wxWindowModalDialogEvent &event) {
	wxDialog *dialog = event.GetDialog();
	int cat = _docsData->_noteBook->GetSelection();
	switch (dialog->GetReturnCode()) {
		case wxID_YES:
			debug("Deleting entry");
			_docsEntries[cat].erase(_docsEntries[cat].begin() + _curRow);
			_docsData->_grids[cat]->DeleteRows(_curRow + 1, 1);
			_docsData->_grids[cat]->ForceRefresh();
			saveLocalDocs();
			break;
		case wxID_NO:
			debug("Not deleting");
			break;
		default:
			debug("What??");
	}
	delete dialog;
}

void HexerFrame::prepBitflagDraw(int size, wxString t, wxString a, wxString &addrLine, wxString &zeroLine) {
	int width = wxAtoi(t);
	int addr = 0;
	sscanf(a.c_str(), "%x", &addr);

	if (size == 1) {
		addrLine = "  $" + a;

	} else {
		for (int i = 0; i < (size / width); i++) {
			wxString addrString = "";
			wxString zeroes = "";
			addrString.Printf("%X", ((addr + ((size / width) - 1) - i)));

			if ((int(log10(addr))) & 1) {
				addrString.Pad(1, '0', false);
			}

			wxString space = "  ";
			//space.Pad(size / width, ' ');
			zeroes.Pad(4, '0');

			int zeroNum = width / 4;

			addrLine += space + "$" + addrString + space + " ";
			for (int i = 0; i < zeroNum; i++) {
				zeroLine += zeroes + " ";
			}
		}
	}	
}

void HexerFrame::drawBitflagDiagram(int size, wxString desc, int cat, int lineNum, wxFont f, wxString addrLine, wxString zeroLine, wxString name) {
	wxStringTokenizer BitNamesTokenizer(desc, ",");

	_docsData->_grids[cat]->InsertRows(lineNum, 3, true);
	_docsData->_grids[cat]->SetCellValue(lineNum, 	3, name);
	_docsData->_grids[cat]->SetCellValue(lineNum + 1, 3, addrLine);
	_docsData->_grids[cat]->SetCellValue(lineNum + 2, 3, zeroLine);

	_docsData->_grids[cat]->SetCellFont(lineNum + 1, 3, f);
	_docsData->_grids[cat]->SetCellFont(lineNum + 2, 3, f);

	_docsData->_grids[cat]->InsertRows(lineNum + 3, size, true);
	for (int i = 0; i < size; i++) {
		wxString dashesLine = "";
		for (int j = 1; j < (size + 1); j++) {
			if (j - 1 == (size - 1) - i) {
				dashesLine += "\\";

			} else if (j - 1 < (size - 1) - i) {
				dashesLine += "|";

			} else {
				dashesLine += "-";
			}

			if ((j % 4 == 0) && (j != size)) {
				if (j - 1 < (size - 1) - i) {
					dashesLine += " ";
				} else {
					dashesLine += "-";
				}
			}
		}
		_docsData->_grids[cat]->SetCellValue(lineNum + 3 + i, 3, dashesLine + "- " + BitNamesTokenizer.GetNextToken());
		_docsData->_grids[cat]->SetCellFont(lineNum + 3 + i, 3, f);
	}
	_docsData->_grids[cat]->InsertRows(lineNum + 3 + size, 1, true);
}

// ------------------------------------------------------------------