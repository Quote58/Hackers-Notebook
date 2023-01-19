#include "romEditor.h"

// This function is just to make the code easier to read and avoid small errors
int getOffset(int row, int col, int offset, int byteWidth) {
	return offset + ((row - (offset / (16 * byteWidth))) * (16 * byteWidth)) + (byteWidth * (col - 1));
}

// When returning values from a selection, we want to do a lot of turning things into a string version of a byte
wxString printByte(int b) {
	return wxString::Format("%02X", b);
}

/* The number of rows for the grid depends
 * on the size of the data type, so we use
 * a switch statement to get the right return.
 */
int RomEditorTable::GetNumberRows() {
	switch (_viewType) {
	case kViewTypeChars:
		return _size / (16 * _stringByteSize);

	case kViewTypePal:
		return _size / (16 * _palByteSize);

	case kViewTypeGfx:
		return _size / (16 * _gfxByteSize);

	// Bytes just use the default size
	case kViewTypeBytes:
	default:
		return _size / 16;
	}
}

/* Whenever any cell is shown or
 * selected, a value must be returned from the table
 */
wxString RomEditorTable::GetValue(int row, int col) {
	// Column 0 is the offset
	if (col == 0) {
		// To calculate the size we need for the offset string, we need to use the biggest number we'll get
		wxString sizeString = wxString::Format("%X", (int) _size);
		int offset = 16;

		// Now we get the actual offset by multiplying by the size of the data type
		switch (_viewType) {
		case kViewTypeChars:
			offset *= _stringByteSize;
			break;

		case kViewTypePal:
			offset *= _palByteSize;
			break;

		case kViewTypeGfx:
			offset *= _gfxByteSize;
			break;

		// Bytes just use the default size
		case kViewTypeBytes:
		default:
			break;
		}

		/* Because we can't use GetFirstVisibleRow() (we're in the table, not the grid),
		 * we have to manually determine the difference between the first visible offset
		 * and our current row offset. This is done by getting the difference between
		 * the amount before the offset (_offset / offset) in rows, and the current row.
		 * Then we can simply multiply by the size of a row, and add that to the current
		 * offset to get the offset at our current row.
		 */
		offset = _offset + ((row - (_offset / offset)) * offset);
		wxString offsetString = wxString::Format("%X", offset);

		// We also pad the string out with zeroes so it looks cleaner
		offsetString.Pad(sizeString.Len() - offsetString.Len(), '0', false);
		return offsetString;

	// Any other column is data
	} else {

		// All view types need to know where they are in the table
		int byteIndex = 0;

		// Depending on the view type, we want to return a different set of data
		// This is also isn't a switch statement because we need to set up some variables depending on which view is active

		/*** Bytes ***
		 */
		if (_viewType == kViewTypeBytes) {
			// Bytes are always 1 byte large, obviously
			byteIndex = getOffset(row, col, _offset, 1);

			// For regular bytes, we just return a two byte string of the hex representation of the byte
			return printByte(_rom->getByte(byteIndex));

		/*** Characters ***
		 */
		} else if (_viewType == kViewTypeChars) {
			// For strings, it gets slightly more complicated, because we might need to return more than one byte
			// To start, we need to know where we are in the table
			byteIndex = getOffset(row, col, _offset, _stringByteSize);

			// First check if we are at the start of a character, or if the data set ends before this square
			if ((byteIndex + _stringByteSize) > _size) {
				// If the offset + the size of the character is past the end of the rom,
				// then we are in part of the final char, not a new one
				return "><";
			}

			// Otherwise we want to grab the first byte, which will be used to get the string table equivalent
			wxString byte = printByte(_rom->getByte(byteIndex));

			// Then, we add another byte to the string for as many bytes as the string needs
			for (int i = 1; i < _stringByteSize; i++) {
				byte << printByte(_rom->getByte(byteIndex + i));
			}

			// And finally we return the equivalent string from our string table dictionary
			return _stringTable[byte];

		/*** Palettes ***
		 */
		} else if (_viewType == kViewTypePal) {
			// First we need to set up some data

			// Like where we are in the table
			byteIndex = getOffset(row, col, _offset, _palByteSize);

			// Just like characters, check if this is the start of a colour or if it doesn't fit in the final squares
			if ((byteIndex + _palByteSize) > _size) {
				return "><";
			}

			// And the bitdepth of the colour data
			int bitDepth = _formatCtrl->GetValue();

			// We need to extract the red/green/blue bits based on the bitdepth
			int redBits   = bitDepth;
			int greenBits = bitDepth;
			int blueBits  = bitDepth;

			int red   = 0;
			int green = 0;
			int blue  = 0;

			wxByte clrByte = 0;
			wxByte bit = 0;

			// Palettes need to return a string representation of their colour (so the colour picker can use it, you can copy/paste, etc.)
			// So we need to extract the colour out of the bytes here
			for (int i = 0; i < _palByteSize; i++) {
				clrByte = _rom->getByte(byteIndex + i);

				for (int b = 0; b < 8; b++) {
					if (redBits > 0) {
						bit = clrByte & 1;
						bit <<= (bitDepth - redBits);
						red |= bit;
						redBits--;

					} else if (greenBits > 0) {
						bit = clrByte & 1;
						bit <<= (bitDepth - greenBits);
						green |= bit;
						greenBits--;
					
					} else if (blueBits > 0) {
						bit = clrByte & 1;
						bit <<= (bitDepth - blueBits);
						blue |= bit;
						blueBits--;
					}
					clrByte >>= 1;	
				}
			}

			// Return a string made of the colours delineated by spaces
			return printByte(red) + "," + printByte(green) + "," + printByte(blue);

		/*** Graphics ***
		 */
		} else if (_viewType == kViewTypeGfx) {
			// For graphics, the copied data should eventually be an actual image type or something
			// But for now it will be the raw byte data of the graphics

			byteIndex = getOffset(row, col, _offset, _gfxByteSize);

			// First check if we are at the start of a tile, or if the data set ends before this square
			if ((byteIndex + _gfxByteSize) > _size) {
				return "><";
			}

			// The gfx data is made up of many bytes of data, so we want to add to this first byte
			wxString bytes = printByte(_rom->getByte(byteIndex));

			// Then, we add another byte to the string for as many bytes as the gfx data has
			for (int i = 1; i < _gfxByteSize; i++) {
				bytes << printByte(_rom->getByte(byteIndex + i));
			}

			return bytes;

		/*** In any other case, just return the non-data symbol ***
		 */
		} else {
			return "><";
		}
	}
}

void RomEditorTable::SetValue(int row, int col, const wxString &value) {
	/* This needs to break up the selection data into multiple chunks and apply the data by calling this function recursively
	 *
	 */

	wxStringTokenizer selection = wxStringTokenizer(value, ' ');

	wxString lineString = selection.GetNextToken().Trim();
	wxStringTokenizer line = wxStringTokenizer(lineString, '\t');

	selection = wxStringTokenizer(value, ' ');

	int selectCount = selection.CountTokens();
	int lineCount = line.CountTokens();

	// Multiple lines of cells to apply
	if (selectCount > 1) {
		// Push the offset because we'll want to restore it after
		for (int i = 0; i < selectCount; i++) {
			SetValue(row, col, lineString);
			lineString = selection.GetNextToken().Trim();
			_offset += 16 * _palByteSize;
			_offset -= (lineCount * _palByteSize);
		}

		_offset -= ((16 * _palByteSize) * (selectCount - 1)) + (lineCount * _palByteSize);

	// Multiple cells to apply
	} else if (lineCount > 1) {
		for (int i = 0; i < lineCount; i++) {
			_offset += i;
			SetValue(row, col, line.GetNextToken().Trim());
		}

	// Single cell to apply
	} else {
		// All view types need to know where they are in the table
		int byteIndex = 0;

		if (_viewType == kViewTypeBytes) {
			byteIndex = getOffset(row, col, _offset, 1);
			int byte = -1;
			
			// Use ssccanf to get only the first two characters of hexadecimal in the input string
			sscanf(value.c_str(), "%2x", &byte);

			// If it was able to get a two digit hexadecimal value, set the byte in the rom to that
			if (byte != -1) {
				_rom->setByte(byteIndex, byte);
			}

		} else if (_viewType == kViewTypeChars) {
			// ***** Ideally, the cell editor would not appear if the cell is not representing complete data *****
			byteIndex = getOffset(row, col, _offset, _stringByteSize);

			// First check if we are at the start of a character, or if the data set ends before this square
			if (!((byteIndex + _stringByteSize) > _size)) {
				// For strings, we want to get the equivalent value,
				wxString key = "";
				int byte = -1;

				// so we start by looping through the string table looking for the input value
				for (StringTable::iterator it = _stringTable.begin(); it != _stringTable.end(); ++it) {
					// If the input string is one of the table entries, we set key to be that value
					if (it->second.Cmp(value) == 0) {
						key = it->first;
					}
				}

				// If the key is not still empty, then we know it's a sequence of bytes from the table
				if (key != "") {
					for (int i = 0; i < _stringByteSize; i++) {
						// For every byte of the string, we get the hexadecimal value (don't need 2X because we only grab 2 chars)
						sscanf(key.Mid((i * 2), 2).c_str(), "%x", &byte);
						
						if (byte != -1) {
							_rom->setByte(byteIndex + i, byte);						
						
						} else {
							std::cout << "found a key, but it isn't a 2 digit hexadecimal value?!" << std::endl;
							std::cout << byte << std::endl;
						}
					}
				}
			}

		} else if (_viewType == kViewTypePal) {
			wxStringTokenizer partsTokenizer = wxStringTokenizer(value, ",");
			int numParts = partsTokenizer.CountTokens();

			if (numParts != 3) {
				// This should also support adding the colours as a single byte/word/etc. <-- yes, it should
				std::cout << "this colour value was not formatted correctly " << numParts << std::endl;
				return;
			}

			// Gotta make sure we're at the start of a colour
			byteIndex = getOffset(row, col, _offset, _palByteSize);
			if ((byteIndex + _palByteSize) > _size) {
				return;
			}

			/* The palette can be entered either with the colour picker by double clicking,
			 * or by two regular clicks to get to the normal cell editor. Either way, we
			 * will end up here with a string as the value, so we need to extract the colour
			 * data, and then add the stream of bits into the rom.
			 */
			int red   = 0;
			int green = 0;
			int blue  = 0;

			sscanf(partsTokenizer.GetNextToken().c_str(), "%2x", &red);
			sscanf(partsTokenizer.GetNextToken().c_str(), "%2x", &green);
			sscanf(partsTokenizer.GetNextToken().c_str(), "%2x", &blue);

			int bitDepthMax = _formatCtrl->GetValue() * 3;
			int bitDepth = _formatCtrl->GetValue();
			int bit = 0;

			wxByte clrByte = 0;

			// For every byte in the colour
			for (int i = 0; i < _palByteSize; i++) {
				clrByte = 0;

				// For every bit in the byte
				for (int b = 0; b < 8; b++) {
					// If we're in the first third of the bits, it's red
					if (bitDepthMax > (bitDepth * 2)) {
						bit = (red & 1) << b;
						red >>= 1;

					// Second third is green
					} else if (bitDepthMax > bitDepth) {
						bit = (green & 1) << b;
						green >>= 1;
					
					// Last third is blue
					} else {
						bit = (blue & 1) << b;
						blue >>= 1;
					}

					// If we are still adding colour bits, we add the bit to the colour byte
					if (bitDepthMax > 0) {
						bitDepthMax--;
						clrByte |= bit;
					}
				}
				_rom->setByte(byteIndex + i, clrByte);
			}

		} else if (_viewType == kViewTypeGfx) {
			byteIndex = getOffset(row, col, _offset, _gfxByteSize);

			// First check if we are at the start of a character, or if the data set ends before this square
			if (!((byteIndex + _gfxByteSize) > _size)) {
				// For strings, we want to get the equivalent value,
				int byte = -1;
				for (int i = 0; i < _gfxByteSize; i++) {
					// For every byte of the string, we get the hexadecimal value (don't need 2X because we only grab 2 chars)
					sscanf(value.Mid((i * 2), 2).c_str(), "%x", &byte);
					
					if (byte != -1) {
						_rom->setByte(byteIndex + i, byte);						
					
					} else {
						std::cout << "the gfx are not being returned as a set of 2 digit hexadecimal values" << std::endl;
						std::cout << byte << std::endl;
					}
				}
			}
		}
	}
}

/* Render the data as graphics for any given cell
 */
void RomEditorGfxRenderer::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) {
	// We need the table of the grid first
	RomEditorTable *table = (RomEditorTable *) grid.GetTable();

	// If the cell is not the start of a tile, we want to use the string renderer to show the generic 'not part of the rom' symbol
	if (grid.GetCellValue(row, col) == "><") {
		wxGridCellStringRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);

	// If the cell is currently selected, just paint it the selection colour instead of drawing gfx
	} else if (grid.IsThisEnabled() && isSelected && grid.HasFocus()) {
		wxColour clr = grid.GetSelectionBackground();
		dc.SetBackgroundMode( wxBRUSHSTYLE_SOLID );
		dc.SetBrush(clr);
		dc.SetPen( *wxTRANSPARENT_PEN );
		dc.DrawRectangle(rect);

	// If the cell is not selected, render the bytes as gfx
	} else {
		// First thing we need is the offset of the current cell
		int offset = getOffset(row, col, table->_offset, table->_gfxByteSize);

		// We need the bitdepth of the gfx
		int bitDepth = table->_gfxCtrl->GetValue();

		// And the brightness for when we display them
		int brightness = 8 - table->_gfxPalCtrl->GetValue();

		int gfxType = table->_gfxType->GetSelection();

		int yOffset = 0;
		int byte = 0;
		int nyble = 0;
		int bit = 0;
		int index = 0;

		wxImage *tile = new wxImage(8, 8, true);

		// Planar gfx split the gfx data into separate bitplanes, each being a binary representation
		if (gfxType == kGfxTypePlanar || gfxType == kGfxTypePlanarComp) {

			int composite = (gfxType == kGfxTypePlanarComp) ? 2 : 1;
			int yInc = 0;
			int rowInc = 0;
			// Handle odd numbered composite bitdepths here

			// the bits in a bitplane represent a tile in the same order you see the pixels
			for (int y = 0; y < 8; y++) {

				// Which, counter intuitively, means they are stored from most to least significant
				for (int x = 7; x >= 0; x--) {

					index = 0;

					// For every pixel, we have anywhere from 1 to 8 bitplanes to combine
					for (int bp = 0; bp < bitDepth; bp++) {

						// And now where the magic happens
						yInc = y * composite;
						rowInc = (bp / composite) * (8 * composite);
						yOffset = (bp & 1) * (composite - 1);

						// With the 3 increment factors calculated, we can add them to byte offset and get the byte from the rom
						byte = table->_rom->getByte(offset + yInc + rowInc + yOffset);

						// Now we can extract the bit at the X position within the byte
						bit = (byte & (1 << x)) >> x;
						bit <<= bp;

						// And add it to the index, which will be our pixel
						index |= bit;
					}

					// Finally, the pixel is placed at x,y, and uses the pixel as an index to get a colour from the current palette
					tile->SetRGB((7 - x), y, table->_gfxPalette[index].Red() << brightness, table->_gfxPalette[index].Green() << brightness, table->_gfxPalette[index].GetBlue() << brightness);
				}
			}

		} else if (gfxType == kGfxTypeLinear || gfxType == kGfxTypeLinearRev) {
			for (int y = 0; y < 8; y++) {
				for (int x = 7; x >= 0; x--) {

					// This is just, super complicated tbh
					nyble = 2 - ((bitDepth - 1) / 4);
					yOffset = (((x & 1) * 4) * (nyble - 1));
					byte = table->_rom->getByte(offset + (((y * 8) + x) / nyble));
					index = (byte & ((((int) pow(2, bitDepth) - 1) << ((8 / nyble) - bitDepth)) << yOffset)) >> yOffset;

					// And this time, we don't have to reverse the pixel drawing order
					tile->SetRGB(x, y, table->_gfxPalette[index].Red() << brightness, table->_gfxPalette[index].Green() << brightness, table->_gfxPalette[index].GetBlue() << brightness);
				}
			}
		}
		wxBitmap *gfx = new wxBitmap(tile->Scale(rect.width, rect.height, wxIMAGE_QUALITY_NORMAL));
		dc.DrawBitmap(*gfx, rect.x, rect.y, false);
	}

}

/* Render the data as a palette for any given cell
 */
void RomEditorPalRenderer::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) {
	dc.SetBackgroundMode( wxBRUSHSTYLE_SOLID );

	wxColour clr;
	if (grid.IsThisEnabled()) {
		if (isSelected) {
			if (grid.HasFocus()) {
				clr = grid.GetSelectionBackground();

			} else {
				clr = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW);
			}
		
		} else {
			// We need the table of the grid first
			RomEditorTable *table = (RomEditorTable *) grid.GetTable();

			if (grid.GetCellValue(row, col) == "><") {
				wxGridCellStringRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);
				return;
			
			} else {
				// 'Brightness' is the number of bits that the colour value can be shifted higher within a 24bit colour, effectively making it brighter
				int brightness = 8 - table->_formatCtrl->GetValue();

				// The colour gets extracted from the bytes in the getValue method, so here we just need to extract from the string
				wxStringTokenizer colourTokenizer(grid.GetCellValue(row, col), ",");

				wxString inputRed   = colourTokenizer.GetNextToken();
				wxString inputGreen = colourTokenizer.GetNextToken();
				wxString inputBlue  = colourTokenizer.GetNextToken();

				int red;
				int green;
				int blue;

				sscanf(inputRed.c_str(),   "%2x", &red);
				sscanf(inputGreen.c_str(), "%2x", &green);
				sscanf(inputBlue.c_str(),  "%2x", &blue);

				// Bit shift by the 'brightness' to fill out remaining bits to increase brightness
				clr = wxColour(red << brightness, green << brightness, blue << brightness);
			}
		}
	
	} else {
		// grey out fields if the grid is disabled
		clr = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
	}

	dc.SetBrush(clr);
	dc.SetPen( *wxTRANSPARENT_PEN );
	dc.DrawRectangle(rect);
}
