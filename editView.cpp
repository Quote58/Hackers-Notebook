#include "hexer.h"
#include "rom.h"

// ------------------------------------------------------------------
/* ****               ****
 * **** The Edit View ****
 * ****               ****
 * -> Frame 				[Frame]
 * -> ToolBar				[ToolBar]
 * -> mainSizer 			[BoxSizer V]
 *  \-> EditView 			[Panel]
 *		-> EditViewSizer 	[BoxSizer V]
 *		 \-> EditBook 		[NoteBook]
 *		   \-> EditGrid 	[Grid]
 *		-> LowerSizer		[BoxSizer H]
 *		 \-> AddPatch		[Button]
 *		 \-> ChangeFile		[Button]
 *		 \-> RomName		[StaticText]
 */
void HexerFrame::populateEditViewDefault() {
	wxBoxSizer *defaultSizer = new wxBoxSizer(wxVERTICAL);
	wxStaticText *loadRom = new wxStaticText(_editView, wxID_ANY, "Load a rom to view available patches");
	defaultSizer->AddStretchSpacer();
	defaultSizer->Add(loadRom, 0, wxALIGN_CENTER_HORIZONTAL);
	defaultSizer->AddStretchSpacer();
	defaultSizer->Layout();
	_editView->SetSizer(defaultSizer);
}

void HexerFrame::populateEditView() {
	// Since we can load a new rom after one is already loaded, we need to clear everything from the edit view panel first
	_editView->DestroyChildren();

	// We need to load the tweaks file before populating the panel in case the file can't be loaded
	wxTextFile _editFile(wxString("local/editLocal"));
	if (!_editFile.Open()) {
		wxLogError(wxString("Could not load local edit file"));
		_editView->Show(false);
		return;
	}

	debug("starting to populate edit view");

	_editData = new ViewData();

	// Now we can start re-populating the panel

	/* The important part is that the notebook and controls under it
	 * are in different sizers, so that the controls can be right-aligned.
	 * In this case the notebook has vertical, the controls have horizontal.
	 * As of right now, I don't think I need to reference these sizers later.
	 * If I do find a need for that, I will make them class members.
	 */
	wxBoxSizer *editViewSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *midSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *lowerSizer = new wxBoxSizer(wxHORIZONTAL);

	// Firstly, we need a notebook
	_editData->_noteBook = new wxNotebook(_editView, wxID_ANY);
	_editData->_noteBook->Bind(wxEVT_BOOKCTRL_PAGE_CHANGING, &HexerFrame::onNotebookChange, this);

	createSidePanel(_editData, _editView);

	wxStaticText *romName = new wxStaticText(_editView, wxID_ANY, "Rom: " + _rom->_name);
		wxButton *changeFile = new wxButton(_editView, wxID_ANY, "New Patches File...", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Load new local patches file");

	midSizer->Add(_editData->_noteBook, 1, wxGROW | wxTOP, kMacMargins);
	midSizer->Add(_editData->_sidePanel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 15);
	midSizer->Add(_editData->_showSidePanel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5);

	lowerSizer->Add(romName, 0, wxALIGN_CENTER_VERTICAL);
	lowerSizer->AddStretchSpacer();
	lowerSizer->Add(changeFile, 0, wxGROW);

	editViewSizer->Add(midSizer, 1, wxGROW | wxLEFT, kMacMargins + 5);
	editViewSizer->Add(lowerSizer, 0, wxGROW | wxALL, kMacMargins + 5);

	/* Populating the edit page is done in two parts:
	 * 1. Split up the categories into individual strings (stored in a vector), and use that to determine
	 *    the names and number of pages in the notebook. Then create the notebook where each page
	 *    is a grid.
	 * 2. Iterate over the file line by line, breaking each line up into it's components, which get stored
	 *    in structures which are contained in vectors. At the same time,
	 *	  create the grid that will display all of the data.
	 */

	/* 
	 * ---- Part 1 ----
	 */
	// The first line in the local files define the categories
	wxString categories = _editFile.GetFirstLine();
	debug(categories);

	// In C++, to break up a string we need a tokenizer
	wxStringTokenizer catTokenizer(categories, "|");
	int i = 0;
	while (catTokenizer.HasMoreTokens()) {

		// The page names are the category names
		createPage(_editData, catTokenizer.GetNextToken(), 0, 5);

		// For the edit view, we need a checkbox inside the grid
		wxGridCellAttr* checkBoxAttr = new wxGridCellAttr();
		checkBoxAttr->SetReadOnly(false);
		checkBoxAttr->SetEditor(new ThreeStateBoolEditor());
		checkBoxAttr->SetRenderer(new ThreeStateBoolRenderer());
		checkBoxAttr->SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
		_editData->_grids[i]->SetColAttr(3, checkBoxAttr);
		i++;
	}

	// Now that the grids are created, we can make the category text control use the default category label
	_editData->_catName->SetLabel(_editData->_catNames[0]);

	/* 
	 * ---- Part 2 ----
	 */

	// We need to keep track of the line id by category so that we can add each entry to the right vector
	int numPages = _editData->_pages.size();

	wxVector<int> lineID(numPages);
	_editPatches = wxVector2D<Entry>(numPages);

	for (wxString line = _editFile.GetNextLine(); !_editFile.Eof(); line = _editFile.GetNextLine()) {
		// Each line is stored in the hashmap within a struct
		Entry lineEditPatch;

		// Each line is defined as: category|title|description|offsets|new bytes|old bytes
		wxStringTokenizer lineTokenizer(line, "|");

		// The category is an int, so we have to convert it from the string, the rest are strings
		int lineCat = wxAtoi(lineTokenizer.GetNextToken());
		   lineEditPatch._cat = lineCat;
		  lineEditPatch._name = lineTokenizer.GetNextToken();
		 wxString lineOffsets = lineTokenizer.GetNextToken();
		wxString lineNewBytes = lineTokenizer.GetNextToken();
		wxString lineOldBytes = lineTokenizer.GetNextToken();
		  lineEditPatch._desc = lineTokenizer.GetNextToken();
		
		// To properly capture multi-line descriptions, we will use the system newline chars
		// Which means the whole string is contained in "" instead of always being one line per entry
		if (lineEditPatch._desc[0] == '\"') {
			wxString longLine = line;
			while (longLine[longLine.size() - 1] != '\"') {
				longLine = _editFile.GetNextLine();
				lineEditPatch._desc << '\n' << longLine;
			}
		}

		// Add the patch name to the grid by adding a new line to it
		_editData->_grids[lineCat]->InsertRows(lineID[lineCat], 1, true);
		_editData->_grids[lineCat]->SetCellValue(lineID[lineCat], 3, "1");
		_editData->_grids[lineCat]->SetCellValue(lineID[lineCat], 4, lineEditPatch._name);

		// Each of the offset/byte lines is deliniated by commas
		wxStringTokenizer offsetTokenizer(lineOffsets, ",");
		wxStringTokenizer newByteTokenizer(lineNewBytes, ",");
		wxStringTokenizer oldByteTokenizer(lineOldBytes, ",");

		// Now we can start to extract the offset and byte data from the strings
		while (offsetTokenizer.HasMoreTokens()) {

			// We need another struct for the bytes to keep everything organized
			PatchBytes offsetBytes;
			wxVector<wxByte> crb;
			
			// This is a little weird looking, but it's how we can grab a hex offset in C++
			// Can this use printf instead? Probably...
			sscanf(offsetTokenizer.GetNextToken().c_str(), "%x", &offsetBytes._offset);

			// Now that we have an offset, we want to break up the new and old bytes
 			wxString newBytes = newByteTokenizer.GetNextToken();
			wxString oldBytes = oldByteTokenizer.GetNextToken();

			for (int i = 0; i < newBytes.size(); i += 2) {
				int newByte;
				sscanf(newBytes.SubString(i, i + 1).c_str(), "%x", &newByte);
				offsetBytes._newBytes.push_back(newByte);

				int oldByte;
				sscanf(oldBytes.SubString(i, i + 1).c_str(), "%x", &oldByte);
				offsetBytes._oldBytes.push_back(oldByte);

				// Current bytes are what are in the rom when loaded, and we need them so we can know whether the patch will be useable or not
				int currentByte = _rom->getByte(offsetBytes._offset + (i / 2));
				crb.push_back(currentByte);

				// If the current bytes in the rom are not the new bytes, we want it to be set to off,
				// and if none of the bytes match, we want it to be set to a third state to warn the user
				if ((currentByte != newByte) && (currentByte != oldByte)) {
					// Since the checkbox is being rendered by the grid, there's no third option currently
					_editData->_grids[lineCat]->SetCellValue(lineID[lineCat], 3, "2");

				} else if (currentByte != newByte) {
					_editData->_grids[lineCat]->SetCellValue(lineID[lineCat], 3, "0");
				}
			}

			// Now that we have the bytes, we can add them to patch for this line
			lineEditPatch._bytes.push_back(offsetBytes);
		}
		// Finally we can add the patch line to the vector
		_editPatches[lineCat].push_back(lineEditPatch);

		// If there is a description for the patch, add a question mark button to the left of it
		if (lineEditPatch._desc != "") {
			_editData->_grids[lineCat]->SetCellValue(lineID[lineCat], 2, "?");
		}
		
		lineID[lineCat]++;
	}

	// Now we just need to shrink the grid to the contents
	for (int i = 0; i < numPages; i++) {
		_editData->_grids[i]->AutoSize();
	}

	// Add the sizer to the panel and render it with layout()
	_editView->SetSizer(editViewSizer);
	editViewSizer->Layout();
}

void HexerFrame::onMouseInEditGrid(wxMouseEvent& event) {
	if (_editMode) {
		wxWindow *w = (wxWindow *) event.GetEventObject();
		wxGrid *grid = (wxGrid *) w->GetParent();

		wxPoint pos = event.GetPosition();
		int row = YToRowGood(grid, pos.y);

		if ((_prevRow != row) && (row != -1)) {
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

void HexerFrame::onEditGridLeftClick(wxGridEvent& event) {
	/* Left click (or double click) event:
	 * We want to do two things here. 1. We want to
	 * eat the click event. This way, the grid doesn't
	 * try to do any sort of selection on the content.
	 * 2. Check/uncheck the checkbox, bring up more info,
	 * or bring up the delete dialog.
	 */
	int cat = _editData->_noteBook->GetSelection();
	_curRow = event.GetRow();

	if (event.GetCol() == 2) {	
		// Click was on the moreInfo button
		wxString s = _editPatches[cat][_curRow]._desc;
		if (s != "") {
			moreInfo(s, false);
		}

	} else if ((event.GetCol() == 0) && _editMode) {
		// Click was on the delete patch button
		wxMessageDialog *askDelete = new wxMessageDialog(this, "You can't undo this action.", "Are you sure you want to permenantly destroy the item?", wxYES_NO | wxCENTRE | wxICON_WARNING, wxDefaultPosition);
		askDelete->Bind(wxEVT_WINDOW_MODAL_DIALOG_CLOSED, &HexerFrame::askDeleteClosedEdit, this);
		askDelete->ShowWindowModal();

	} else if ((event.GetCol() == 1) && _editMode) {
		// Click was on the edit patch button
		addEntry(false, _editPatches[cat][_curRow]._name, _editPatches[cat][_curRow]._desc, _editPatches[cat][_curRow]._size, _editPatches[cat][_curRow]._type, "", "", "");

	} else {
		// Click was on the checkbox or the name
		wxGrid *grid = (wxGrid *) event.GetEventObject();
		wxString value = grid->GetCellValue(_curRow, 3);

		bool old = false;

		if ((value == "") || value == "0") {
			grid->SetCellValue(_curRow, 3, "1");
			// Apply the patch
			debug("set, applying the new bytes");

		} else {
			grid->SetCellValue(_curRow, 3, "0");
			// Reverse the patch
			debug("unset, applying the old bytes");
			old = true;
		}

		for (int i = 0; i < _editPatches[cat][_curRow]._bytes.size(); i++) {
			if (old) {
				_rom->setBytes(_editPatches[cat][_curRow]._bytes[i]._offset, _editPatches[cat][_curRow]._bytes[i]._oldBytes);
			} else {
				_rom->setBytes(_editPatches[cat][_curRow]._bytes[i]._offset, _editPatches[cat][_curRow]._bytes[i]._newBytes);
			}
		}

		grid->Refresh();
	}
}

void HexerFrame::askDeleteClosedEdit(wxWindowModalDialogEvent &event) {
	wxDialog *dialog = event.GetDialog();
	int cat = _editData->_noteBook->GetSelection();
	switch (dialog->GetReturnCode()) {
		case wxID_YES:
			debug("Deleting entry");
			_editPatches[cat].erase(_editPatches[cat].begin() + _curRow);
			_editData->_grids[cat]->DeleteRows(_curRow, 1);
			_editData->_grids[cat]->ForceRefresh();
			saveLocalEdit();
			break;
		case wxID_NO:
			debug("Not deleting");
			break;
		default:
			debug("What??");
	}
	delete dialog;
}

void HexerFrame::onLoadNewPatchFile() {

}
// ------------------------------------------------------------------