#ifndef HEXER_ROM_H
#define HEXER_ROM_H

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
 
#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/vector.h>
#include <wx/wfstream.h>
#include <wx/filename.h>

/* Hexer Rom handler
 * This class handles the actual I/O
 * for the rom being edited
 */
class Rom {
public:
	Rom(wxString path);
	~Rom();

	wxFile *_rom;							// The Rom itself
	wxByte *_dataBuffer;					// A mutable buffer of the rom data
	wxString _name;							// The name of the rom file

	void saveToRom();						// Replaces the rom contents with the dataBuffer contents, ie. Applies the changes
	wxByte getByte(int offset);				// Gets a single byte from the rom at offset
	void setByte(int offset, wxByte byte);	// Sets the byte at offset in the buffer to byte
	void setBytes(int offset, wxVector<wxByte> bytes);	// Sets the bytes at offset in the buffer to bytes
	 int searchByte(wxByte);				// Search for a single byte, returns -1 if not found, offset if found
	 int searchBytes(wxVector<wxByte>);		// Search for an array of bytes, returns -1 if not found, offset if found
	void mountUndoCodeRead();
};

#endif