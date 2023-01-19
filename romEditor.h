#ifndef HEXER_ROMEDITOR_H
#define HEXER_ROMEDITOR_H

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
 
#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/vector.h>
#include "wx/grid.h"

#include "wx/generic/gridsel.h"
#include "wx/generic/gridctrl.h"
#include "wx/generic/grideditors.h"
#include "wx/headerctrl.h"
#include "wx/generic/gridctrl.h"
#include "wx/generic/grideditors.h"
#include <wx/spinctrl.h>
#include <wx/tokenzr.h>

#include "rom.h"

WX_DECLARE_STRING_HASH_MAP(wxString, StringTable);

enum ViewType {
	kViewTypeBytes,
	kViewTypeChars,
	kViewTypePal,
	kViewTypeGfx
};

enum GfxType {
	kGfxTypePlanar,
	kGfxTypePlanarComp,
	kGfxTypeLinear,
	kGfxTypeLinearRev,
	kGfxTypeRGB
};

class RomEditorGfxRenderer : public wxGridCellStringRenderer {
public:
    virtual void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) wxOVERRIDE;
};


class RomEditorPalRenderer : public wxGridCellStringRenderer {
public:
    virtual void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) wxOVERRIDE;
};


class RomEditorTable : public wxGridTableBase {
public:
	long _size;
	int _offset = 0;

	int _viewType = kViewTypeBytes;

	int _stringByteSize = 1;
	int _palByteSize = 1;
	int _gfxByteSize = 8;

	StringTable _stringTable;
	wxVector<wxColour> _indexedPalette;
	wxVector<wxColour> _gfxPalette;

	double _fontSizeBytes = 0;
	double _fontSizeChars = 0;

	wxSize _byteSize;
	wxSize _offsetLabelSize;

	int _cellSizeBytes;
	int _cellSizeGfx;
	
	wxSpinCtrl *_stringCtrl;

	wxSpinCtrl *_formatCtrl;
	wxCheckBox *_clrIndexCheck;

	wxSpinCtrl *_gfxCtrl;
	wxSpinCtrl *_gfxPalCtrl;
	  wxChoice *_gfxType;

	wxBitmap *_tileImage;				// Will be used later

	Rom *_rom = nullptr;

	RomEditorTable(long size, Rom *rom, int viewType) {
		_rom = rom;
		_size = size;
		_viewType = viewType;
	}

	int GetNumberRows() wxOVERRIDE;
	int GetNumberCols() wxOVERRIDE { return 17; }
	wxString GetValue(int row, int col) wxOVERRIDE;
	void SetValue(int row, int col, const wxString &value) wxOVERRIDE;
	bool IsEmptyCell(int row, int col) wxOVERRIDE { return false; }
};

#endif