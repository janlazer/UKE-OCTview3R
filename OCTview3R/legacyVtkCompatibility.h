#pragma once

#include <QByteArray>
#include <QString>

namespace legacyVtkCompatibility
{
	enum class InputMode
	{
		UseOriginalFile,
		UseConvertedData,
		Error
	};

	struct PreparedInput
	{
		InputMode mode = InputMode::UseOriginalFile;
		QByteArray data;
		QString error;
	};

	// VTK 9 writes legacy PolyData cell arrays in the 5.1 offsets/connectivity
	// representation. VTK 8.2 only understands the packed representation used
	// through 4.2. Convert only the topology blocks and leave points and data
	// attributes byte-for-byte unchanged.
	PreparedInput preparePolyDataForVtk82(const QString& fileName);
}
