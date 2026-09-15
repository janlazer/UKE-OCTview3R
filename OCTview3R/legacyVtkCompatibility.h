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
		/// UseOriginalFile: reader opens the path; UseConvertedData: reader consumes data.
		InputMode mode = InputMode::UseOriginalFile;
		/// Owned converted bytes, populated only for UseConvertedData; source file is not edited.
		QByteArray data;
		/// User-facing explanation when mode is Error.
		QString error;
	};

	// VTK 9 writes legacy PolyData cell arrays in the 5.1 offsets/connectivity
	// representation. VTK 8.2 only understands the packed representation used
	// through 4.2. Convert only the topology blocks and leave points and data
	// attributes byte-for-byte unchanged.
	/** @brief Inspect/prepare one legacy PolyData file for the VTK 8.2 reader.
	 * @return UseOriginalFile for pass-through, UseConvertedData for supported
	 * converted topology, or Error with an explanation. This is not a promise
	 * to support every future VTK format. Caller must keep converted bytes alive
	 * while the reader uses them. No application scene or source file is mutated.
	 */
	PreparedInput preparePolyDataForVtk82(const QString& fileName);
}
