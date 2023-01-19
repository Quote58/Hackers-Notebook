#include "rom.h"

Rom::Rom(wxString path) {
	_rom = new wxFile(path, wxFile::read_write);
	
	if (_rom->IsOpened()) {
		_dataBuffer = (wxByte *) malloc(_rom->Length());
		_rom->Read(_dataBuffer, _rom->Length());

	} else {
		wxLogError("File could not be opened!");
	}

	wxFileName name(path);
	_name = name.GetName();
}

void Rom::saveToRom() {
	_rom->Seek(0);
	_rom->Write(_dataBuffer, _rom->Length());
	std::cout << "data buffer written over the rom" << std::endl;
}

wxByte Rom::getByte(int offset) {
	if (offset < _rom->Length()) {
		return _dataBuffer[offset];

	} else {
		std::cout << "invalid offset! Can't access offset, returning 0xFF instead " << offset << std::endl;
		return 0xFF;
	}
}

void Rom::setByte(int offset, wxByte byte) {
	if (offset >= _rom->Length()) {
		std::cout << "invalid offset! Can't access offset " << offset << std::endl;
		return;
	}

	_dataBuffer[offset] = byte;
}

void Rom::setBytes(int offset, wxVector<wxByte> bytes) {
	if ((offset + bytes.size()) >= _rom->Length()) {
				std::cout << "invalid offset! Can't access offset and/or number of bytes " << offset << std::endl;

		return;
	}

	for (int i = 0; i < bytes.size(); i++) {
		_dataBuffer[offset + i] = bytes[i];
	}
}

int Rom::searchByte(wxByte b) {

	// Could not find byte
	return -1;
}

int Rom::searchBytes(wxVector<wxByte> bytes) {

	// Could not find byte
	return -1;
}

void Rom::mountUndoCodeRead() {
	wxLogError(wxString("This is a great name for a function"));
}
















