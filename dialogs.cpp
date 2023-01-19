#include "hexer.h"
#include "rom.h"

/* ****              	 ****
 * **** Dialog Functions ****
 * ****              	 ****
 */

void HexerFrame::moreInfo(wxString description, bool big) {
	/* 'More Info' is a message dialog that can be brought up to display
	 * further description of a given item.
	 * It is structured like this: Dialog [ DialogSizer [ StaticText, TextCtrl, ButtonSizer ] ]
	 */

	// If the description is a multi-line, we want to remove the double quotes. If not, we want to leave it as is
	if (description.Find('\n') != wxNOT_FOUND) {
		description = description.Mid(1, description.Len() - 2);
	}

	// First we need a generic dialog, which has a close box (for non-modal supporting platforms)
	wxDialog *moreInfoDialog = new wxDialog(this, wxID_ANY, "More Info", wxDefaultPosition, wxDefaultSize, wxCLOSE_BOX, wxEmptyString);

	// With a normal vertical sizer
	wxBoxSizer *moreInfoSizer = new wxBoxSizer(wxVERTICAL);

	// Now we make a static text, text control, and the default cancel button
	wxStaticText *title = new wxStaticText(moreInfoDialog, wxID_ANY, "More Info", wxDefaultPosition, wxDefaultSize, 0, wxEmptyString);	
	// We want the title to be bigger than the normal text
	title->SetFont(title->GetFont().Scale(1.5));

	wxTextCtrl *infoText = new wxTextCtrl(moreInfoDialog, wxID_ANY, description , wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE | wxTE_WORDWRAP | wxNO_BORDER, wxDefaultValidator, wxEmptyString);
	// The text control by default has a background, so this just removes it by making it transparent
	infoText->SetBackgroundColour(wxColour(0,0,0,0));

	wxSizer *buttonSizer = moreInfoDialog->CreateButtonSizer(wxCLOSE);

	// Now we can add all three to the sizer
	moreInfoSizer->Add(title, 0, wxALIGN_CENTER | wxTOP, 20);
	moreInfoSizer->Add(infoText, 1, wxGROW | wxRIGHT | wxTOP | wxLEFT, 20);
	moreInfoSizer->Add(buttonSizer, 0, wxALIGN_CENTER);
	moreInfoSizer->Layout();
	moreInfoDialog->SetSizer(moreInfoSizer);

	// And if whatever is calling this wants a bigger dialog, we will scale the width and height a little bit
	if (big) {
		moreInfoDialog->SetClientSize(wxSize(moreInfoDialog->GetClientSize().GetWidth()*1.5, moreInfoDialog->GetClientSize().GetHeight()*2));
	}

	moreInfoDialog->ShowWindowModal();
}

/* Add Entry dialog
 * -> AddDialog 			 [Dialog]
 *  \-> AddDialogSizer 		 [BoxSizer]
 *    \-> DetailsSizer 		 [StaticBoxSizer]
 *		\-> DetailsGrid		 [FlexGridSizer]
 *		  \-> StrTitle		 [StaticText]
 *		  \-> Title			 [TextCtrl]
 *		  \-> StrCategory	 [StaticText]
 *		  \-> Category		 [TextCtrl]
 *		  \-> StrDescription [StaticText]
 *		  \-> Description	 [TextCtrl]
 *		  \-> StrSize 		 [StaticText]
 *		  \-> Size	 		 [TextCtrl]
 *		  \-> StrType 		 [StaticText]
 *		  \-> Type	 		 [TextCtrl]
 *    \-> BytesSizer 		 [StaticBoxSizer]
 *		  \-> StrAddress	 [StaticText]
 *		  \-> Address		 [TextCtrl]
 *		  \-> StrOldBytes	 [StaticText]
 *		  \-> OldBytes		 [TextCtrl]
 *		  \-> StrNewBytes 	 [StaticText]
 *		  \-> NewBytes		 [TextCtrl]
 *    \-> ButtonSizer 		 [Sizer]
 *		\-> Cancel 			 [Button]
 *		\-> Apply 			 [Button]
 */
void HexerFrame::addEntry(bool addOrEdit, wxString n, wxString d, wxString s, wxString t, wxString a, wxString nb, wxString ob) {
	/* AddEntry does a lot of things depending on what you give it.
	 * You can call this function 3 ways:
	 * 1. Adding an entry with information
	 * 2. Adding a blank entry
	 * 3. Editing an existing entry
	 */

	// So first step is to fill in any information already given
	// The title/description/size/type are all just normal strings
	wxString sTitle = n;
	wxString sDesc  = d;
	
	// If the description is a multi-line, we want to remove the double quotes. If not, we want to leave it as is
	if (sDesc.Find('\n') != wxNOT_FOUND) {
		sDesc = sDesc.Mid(1, sDesc.Len() - 2);
	}

	wxString sSize  = s;
	wxString sType  = t;

	// However the category is an int, but represented and edited as a string
	wxString sCat;
	wxString sAddress;
	wxString sOldBytes;
	wxString sNewBytes;

	// These bool values need to be used in the Apply function, so we get their value from the function call here
	_addOrEdit = addOrEdit;

	switch (_view) {
		case kViewEdit:
			_editOrDoc = true;
			break;
		case kViewDocs:
			_editOrDoc = false;
			break;
		default:
			debug("huh? This shouldn't be called from hex yet");
	}

	// If we are adding an entry, we want to start with the current category, and blank the address + bytes
	if (addOrEdit) {
		if (_editOrDoc) {
			sCat = _editData->_catNames[_editData->_noteBook->GetSelection()];
		} else {
			sCat = _docsData->_catNames[_docsData->_noteBook->GetSelection()];
		}
		sAddress = a;
		sOldBytes = ob;
		sNewBytes = nb;

	// If we are instead editing an entry, we need to get information out of the entry
	} else {
		Entry *entry;
		if (_editOrDoc) {
			// For instance, the category int which we use to show the category name
			int cat = _editData->_noteBook->GetSelection();
			entry = &_editPatches[cat][_curRow];
			sCat = _editData->_catNames[cat];
			sAddress = "";

		} else {
			// And for documents, the address is already given by the function call
			int cat = _docsData->_noteBook->GetSelection();
			entry = &_docsEntries[cat][_curRow];
			sCat = _docsData->_catNames[cat];
			sAddress = a;
		}

		// The bytes are stored as ints (so that the rom can edit with them more efficiently), so we need to unpack them for display
		wxString sOffsets = "";
		sOldBytes = "";
		sNewBytes = "";
		for (int i = 0; i < entry->_bytes.size(); i++) {
			// For every offset, we add an offset to the offset string
			sOffsets += wxString::Format("%X", entry->_bytes[i]._offset);
			for (int j = 0; j < entry->_bytes[i]._oldBytes.size(); j++) {
				// And for every byte string, we add the byte string to old/newbytes
				sOldBytes += wxString::Format("%02X", entry->_bytes[i]._oldBytes[j]);
				sNewBytes += wxString::Format("%02X", entry->_bytes[i]._newBytes[j]);
			}
			if (i != entry->_bytes.size() - 1) {
				// Separated by commas
				sOffsets << ",";
				sOldBytes << ",";
				sNewBytes << ",";
			}
		}

		// If the address is not given, then it is the same as the offsets
		if (sAddress == "") {
			sAddress += sOffsets;
		}
	}

	// Now we need a dialog
	wxDialog *addDialog = new wxDialog(this, wxID_ANY, "Add Entry", wxDefaultPosition, wxDefaultSize, wxCLOSE_BOX, wxEmptyString);

	// And the primary sizer for the dialog
	wxBoxSizer *addDialogSizer = new wxBoxSizer(wxVERTICAL);

	// The first boxsizer is for the details about the patch or doc entry
	wxStaticBoxSizer *detailsSizer = new wxStaticBoxSizer(wxVERTICAL, addDialog, "Details");
	// Title
	wxStaticText *strTitle = new wxStaticText(detailsSizer->GetStaticBox(), wxID_ANY, "Title: ");
	  wxTextCtrl *title    = new wxTextCtrl(detailsSizer->GetStaticBox(), ID_DTitle, sTitle);
	// Category
	wxStaticText *strCat   = new wxStaticText(detailsSizer->GetStaticBox(), wxID_ANY, "Category: ");
	  wxTextCtrl *cat      = new wxTextCtrl(detailsSizer->GetStaticBox(), ID_DCat, sCat);
	// Description
	wxStaticText *strDesc  = new wxStaticText(detailsSizer->GetStaticBox(), wxID_ANY, "Description: ");
	  wxTextCtrl *desc     = new wxTextCtrl(detailsSizer->GetStaticBox(), ID_DDesc, sDesc, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE, wxDefaultValidator, wxEmptyString);
	// Size
	wxStaticText *strSize  = new wxStaticText(detailsSizer->GetStaticBox(), wxID_ANY, "Size: ");
	  wxTextCtrl *size     = new wxTextCtrl(detailsSizer->GetStaticBox(), ID_DSize, sSize);
	// Type
	wxStaticText *strType  = new wxStaticText(detailsSizer->GetStaticBox(), wxID_ANY, "Type: ");
	  wxTextCtrl *type     = new wxTextCtrl(detailsSizer->GetStaticBox(), ID_DType, sType);
	
	// The second boxsizer is for the address and bytes
	wxStaticBoxSizer *bytesSizer = new wxStaticBoxSizer(wxVERTICAL, addDialog, "Bytes");
	// Offset
	wxStaticText *strAddr     = new wxStaticText(bytesSizer->GetStaticBox(), wxID_ANY, "Address(s): ");
	  wxTextCtrl *addr        = new wxTextCtrl(bytesSizer->GetStaticBox(), ID_DAddress, sAddress);
	// Old Bytes
	wxStaticText *strOldBytes = new wxStaticText(bytesSizer->GetStaticBox(), wxID_ANY, "Old Byte(s): ");
	  wxTextCtrl *oldBytes    = new wxTextCtrl(bytesSizer->GetStaticBox(), ID_DOldBytes, sOldBytes);
	// New Bytes
	wxStaticText *strNewBytes = new wxStaticText(bytesSizer->GetStaticBox(), wxID_ANY, "New Byte(s): ");
	  wxTextCtrl *newBytes    = new wxTextCtrl(bytesSizer->GetStaticBox(), ID_DNewBytes, sNewBytes);

	// Now we need a grid to organize the text and the text controls
	// First for the details
	wxFlexGridSizer *detailsGrid = new wxFlexGridSizer(2, 5, 5);
	detailsGrid->Add(strTitle);
	detailsGrid->Add(title, 0, wxGROW);
	detailsGrid->Add(strCat);
	detailsGrid->Add(cat, 0, wxGROW);
	detailsGrid->Add(strDesc, 0, wxALIGN_CENTER_VERTICAL);
	detailsGrid->Add(desc, 0, wxGROW);
	detailsGrid->AddGrowableRow(2, 0);
	detailsGrid->AddGrowableCol(1, 0);
	detailsGrid->Add(strSize);
	detailsGrid->Add(size, 0, wxGROW);
	detailsGrid->Add(strType);
	detailsGrid->Add(type, 0, wxGROW);
	detailsSizer->Add(detailsGrid, 0, wxGROW);

	// And second for the bytes
	wxFlexGridSizer *bytesGrid = new wxFlexGridSizer(2, 5, 5);
	bytesGrid->AddGrowableCol(1);
	bytesGrid->Add(strAddr);
	bytesGrid->Add(addr, 0, wxGROW);
	bytesGrid->Add(strOldBytes);
	bytesGrid->Add(oldBytes, 0, wxGROW);
	bytesGrid->Add(strNewBytes);
	bytesGrid->Add(newBytes, 0, wxGROW);
	bytesSizer->Add(bytesGrid, 0, wxGROW);

	// Lastly, we need a buttonSizer to hold the confirm and cancel buttons
	wxSizer *buttonSizer = addDialog->CreateButtonSizer(wxCANCEL);

	// However the apply button needs to do something custom, so it's created separately and added to the sizer
	wxButton *applyButton = new wxButton(addDialog, wxID_APPLY, "Apply");
	applyButton->Bind(wxEVT_BUTTON, &HexerFrame::onAddEntryApply, this);
	buttonSizer->Add(applyButton, 0, wxALIGN_CENTER);

	// And finally we can add everything to the sizer, stretching between the boxes and buttons
	addDialogSizer->Add(detailsSizer, 0, wxGROW | wxALL, 15);
	addDialogSizer->Add(bytesSizer, 0, wxGROW | wxALL, 15);
	addDialogSizer->AddStretchSpacer();
	addDialogSizer->Add(buttonSizer, 0, wxALIGN_CENTER);
	addDialogSizer->Layout();
	addDialog->SetSizer(addDialogSizer);

	// Calculate the minimum size of the controls from the main sizer, and then make the dialog that size
	wxSize minSize = addDialogSizer->CalcMin();
	addDialog->SetMinClientSize(minSize);

	// Except that since most of the controls are text controls, we want extra horizontal space to not look ugly
	if (addOrEdit) {
		addDialog->SetClientSize(wxSize((minSize.x * 2), minSize.y));
			
	} else {
		addDialog->SetClientSize(wxSize((minSize.x * 1.5), minSize.y));
	}

	// Now show the dialog in windowModal mode
	addDialog->ShowWindowModal();
}

void HexerFrame::onAddEntryApply(wxCommandEvent &event) {
	wxButton *button = (wxButton *) event.GetEventObject();
	wxDialog *dialog = (wxDialog *) button->GetParent();

	Entry *e;
	ViewData *data;

	int curGridRow = _curRow;

	if (_editOrDoc) {
		data = _editData;
	} else {
		data = _docsData;
		curGridRow++;
	}

	if (_addOrEdit) {
		e = new Entry();
	} else {
		debug("editing...");
		if (_editOrDoc) {
			e = &_editPatches[_editData->_noteBook->GetSelection()][_curRow];
		} else {
			e = &_docsEntries[_docsData->_noteBook->GetSelection()][_curRow];
		}
	}

	e->_name = ((wxTextCtrl *) dialog->FindWindow(ID_DTitle))->GetValue();
	e->_size = ((wxTextCtrl *) dialog->FindWindow(ID_DSize))->GetValue();
	e->_type = ((wxTextCtrl *) dialog->FindWindow(ID_DType))->GetValue();
	e->_addr = ((wxTextCtrl *) dialog->FindWindow(ID_DAddress))->GetValue();
	wxString oldBytes = ((wxTextCtrl *) dialog->FindWindow(ID_DOldBytes))->GetValue();
	wxString newBytes = ((wxTextCtrl *) dialog->FindWindow(ID_DNewBytes))->GetValue();

	// We don't want the user to have to care about adding "" to their long strings, so we do it here	
	e->_desc = ((wxTextCtrl *) dialog->FindWindow(ID_DDesc))->GetValue();
	
	// In case the user somehow enters a real double quote (heh) character, we want it removed before it gets written to the file
	for (int i = 0; i < e->_desc.Len(); i++) {
		if (e->_desc == '\n') {
			e->_desc.erase(i);
		}
	}

	// Check if there are any newline characters, and if so give the entire string double quotes to define it as multi-line
	if (e->_desc.Find('\n') != wxNOT_FOUND) {
		wxString descWithQuotes = "\"";
		e->_desc = descWithQuotes << e->_desc << "\"";
	}

	if (e->_name == "") {
		wxMessageBox("Please input a name for this entry", "Entry must have a title", wxICON_WARNING);
		delete e;
		return;
	}

	if (_editOrDoc) {
		if ((e->_addr == "") || (oldBytes == "") || (newBytes == "")) {
			wxMessageBox("Please make sure the patch has at least one offset and at least one pair of old and new bytes", "A patch requires an address and byte changes", wxICON_WARNING);
			delete e;
			return;
		}
	}

	wxString sCat = ((wxTextCtrl *) dialog->FindWindow(ID_DCat))->GetValue();

	wxVector<wxString> s = data->_catNames;

	int cat = -1;
	for (int i = 0; i < s.size(); i++) {
		if (s[i].Cmp(sCat) == 0) {
			cat = i;
			break;
		}
	}

	// * Needs functionality for moving an entry from one category to another by simply renaming the category on the edit *

	if (cat == -1) {
		e->_cat = s.size();
		if (_editOrDoc) {
			createPage(data, sCat, 0, 5);
			_editPatches.resize(_editPatches.size() + 1);
			// For the edit view, we need a checkbox inside the grid
			wxGridCellAttr* checkBoxAttr = new wxGridCellAttr();
			checkBoxAttr->SetReadOnly(false);
			checkBoxAttr->SetEditor(new ThreeStateBoolEditor());
			checkBoxAttr->SetRenderer(new ThreeStateBoolRenderer());
			checkBoxAttr->SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
			data->_grids[e->_cat]->SetColAttr(3, checkBoxAttr);
		} else {
			createPage(data, sCat, 1, 7);
			_docsEntries.resize(_docsEntries.size() + 1);
			data->_grids[e->_cat]->SetCellValue(0, 3, "Address");
			data->_grids[e->_cat]->SetCellValue(0, 4, "Size (b)");
			data->_grids[e->_cat]->SetCellValue(0, 5, "Type");
			data->_grids[e->_cat]->SetCellValue(0, 6, "Description");
		}
		data->_noteBook->SetSelection(e->_cat);

	} else {
		e->_cat = cat;
	}

	int row = curGridRow;

	if (_addOrEdit) {
		row = data->_grids[e->_cat]->GetNumberRows();
		data->_grids[e->_cat]->InsertRows(row, 1, true);
		if (_editOrDoc) {
			_editData->_grids[e->_cat]->SetCellValue(row, 3, "1");
		}
	}

	wxString oldBytesString = ((wxTextCtrl *) dialog->FindWindow(ID_DOldBytes))->GetValue();
	wxString newBytesString = ((wxTextCtrl *) dialog->FindWindow(ID_DNewBytes))->GetValue();

	debug(e->_addr);
	debug(oldBytesString);
	debug(newBytesString);

	wxStringTokenizer offsetTokenizer(e->_addr, ",");
	wxStringTokenizer oldByteTokenizer(oldBytesString, ",");
	wxStringTokenizer newByteTokenizer(newBytesString, ",");

	e->_bytes.clear();

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
			int newByte = -1;
			sscanf(newBytes.SubString(i, i + 1).c_str(), "%x", &newByte);
			offsetBytes._newBytes.push_back(newByte);

			int oldByte = -1;
			sscanf(oldBytes.SubString(i, i + 1).c_str(), "%x", &oldByte);
			offsetBytes._oldBytes.push_back(oldByte);

			if ((newByte == -1) || (oldByte == -1)) {
				wxMessageBox("Please input only Hexadecimal bytes", "Bytes must be Hexadecimal", wxICON_WARNING);
				data->_grids[e->_cat]->DeleteRows(row, 1);
				delete e;
				return;
			}
			if (_editOrDoc) {
				// Current bytes are what are in the rom when loaded, and we need them so we can know whether the patch will be useable or not
				int currentByte = _rom->getByte(offsetBytes._offset + (i / 2));
				crb.push_back(currentByte);

				// If the current bytes in the rom are not the new bytes, we want it to be set to off,
				// and if none of the bytes match, we want it to be set to a third state to warn the user
				if ((currentByte != newByte) && (currentByte != oldByte)) {
					// Since the checkbox is being rendered by the grid, there's no third option currently
					_editData->_grids[e->_cat]->SetCellValue(row, 3, "2");

				} else if (currentByte != newByte) {
					_editData->_grids[e->_cat]->SetCellValue(row, 3, "0");
				}
			}
		}

		// Now that we have the bytes, we can add them to patch for this line
		e->_bytes.push_back(offsetBytes);
	}

	if (e->_desc != "") {
			data->_grids[e->_cat]->SetCellValue(row, 2, "?");
	
	} else {
			data->_grids[e->_cat]->SetCellValue(row, 2, "");
	}

	if (_editOrDoc) {
		data->_grids[e->_cat]->SetCellValue(row, 4, e->_name);

	} else {
		data->_grids[e->_cat]->SetCellValue(row, 3, e->_addr);
		data->_grids[e->_cat]->SetCellValue(row, 4, e->_size);
		data->_grids[e->_cat]->SetCellValue(row, 5, e->_type);
		data->_grids[e->_cat]->SetCellValue(row, 6, e->_name);
	}

	data->_grids[e->_cat]->AutoSize();
	data->_grids[e->_cat]->ForceRefresh();
	_mainSizer->Layout();

	if (_addOrEdit) {
		if (_editOrDoc) {
			_editPatches[e->_cat].push_back(*e);
			
		} else {
			_docsEntries[e->_cat].push_back(*e);
		}		
	}

	if (_editOrDoc) {
		saveLocalEdit();
	} else {
		saveLocalDocs();
	}

	delete dialog;
}














