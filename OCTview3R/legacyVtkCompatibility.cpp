#include "legacyVtkCompatibility.h"

#include <QFile>
#include <QVector>
#include <QtGlobal>

#include <limits>

namespace
{
	constexpr qint64 MaximumConvertibleFileSize =
		static_cast<qint64>(std::numeric_limits<int>::max());

	struct CellSection
	{
		int start = -1;
		int end = -1;
		QByteArray replacement;
	};

	bool isWhitespace(char value)
	{
		return value == ' ' || value == '\t' || value == '\r' || value == '\n';
	}

	int skipWhitespace(const QByteArray& data, int position)
	{
		while (position < data.size() && isWhitespace(data.at(position)))
			++position;
		return position;
	}

	QList<QByteArray> lineTokens(const QByteArray& data, int start, int end)
	{
		return data.mid(start, end - start).simplified().split(' ');
	}

	int lineEnd(const QByteArray& data, int start)
	{
		const int newline = data.indexOf('\n', start);
		if (newline < 0)
			return data.size();
		return newline > start && data.at(newline - 1) == '\r'
			? newline - 1
			: newline;
	}

	int nextLineStart(const QByteArray& data, int start)
	{
		const int newline = data.indexOf('\n', start);
		return newline < 0 ? data.size() : newline + 1;
	}

	bool parseNonNegativeCount(const QByteArray& token, qint64& value)
	{
		bool ok = false;
		value = token.toLongLong(&ok);
		return ok && value >= 0;
	}

	bool parseVersion(
		const QByteArray& firstLine,
		int& major,
		int& minor,
		int& versionStart,
		int& versionLength)
	{
		const QByteArray marker("# vtk DataFile Version ");
		if (!firstLine.startsWith(marker))
			return false;

		versionStart = marker.size();
		int contentLength = firstLine.size();
		while (contentLength > 0 &&
			(firstLine.at(contentLength - 1) == '\r' ||
				firstLine.at(contentLength - 1) == '\n'))
			--contentLength;
		versionLength = contentLength - versionStart;
		const QByteArray version = firstLine.mid(versionStart, versionLength);
		const int dot = version.indexOf('.');
		if (dot <= 0 || dot == version.size() - 1)
			return false;

		bool majorOk = false;
		bool minorOk = false;
		major = version.left(dot).toInt(&majorOk);
		minor = version.mid(dot + 1).toInt(&minorOk);
		return majorOk && minorOk;
	}

	bool parseAsciiIntegers(
		const QByteArray& data,
		int position,
		qint64 count,
		QVector<qint64>& values,
		int& end,
		QString& error)
	{
		if (count > std::numeric_limits<int>::max())
		{
			error = QStringLiteral("The cell array is too large for this VTK build.");
			return false;
		}

		values.clear();
		values.reserve(static_cast<int>(count));
		for (qint64 index = 0; index < count; ++index)
		{
			position = skipWhitespace(data, position);
			const int tokenStart = position;
			while (position < data.size() && !isWhitespace(data.at(position)))
				++position;
			if (tokenStart == position)
			{
				error = QStringLiteral("The cell array ends before all values were read.");
				return false;
			}

			bool ok = false;
			const qint64 value = data.mid(tokenStart, position - tokenStart).toLongLong(&ok);
			if (!ok)
			{
				error = QStringLiteral("The cell array contains a non-integer value.");
				return false;
			}
			values.append(value);
		}

		end = position;
		return true;
	}

	QVector<int> integerWidths(const QByteArray& typeName)
	{
		const QByteArray type = typeName.toLower();
		if (type == "vtktypeint64" || type == "vtktypeuint64" ||
			type == "long_long" || type == "unsigned_long_long")
			return { 8 };
		if (type == "vtktypeint32" || type == "vtktypeuint32" ||
			type == "int" || type == "unsigned_int")
			return { 4 };
		// The legacy 'long' and 'vtkIdType' names are build-dependent. Try the
		// common 64-bit layout first, then 32-bit, and validate against the next
		// textual header and the topology values.
		if (type == "long" || type == "unsigned_long" || type == "vtkidtype")
			return { 8, 4 };
		return {};
	}

	bool isUnsignedType(const QByteArray& typeName)
	{
		const QByteArray type = typeName.toLower();
		return type.startsWith("unsigned_") || type.startsWith("vtktypeuint");
	}

	bool readBigEndianInteger(
		const QByteArray& data,
		int position,
		int width,
		bool isUnsigned,
		qint64& value)
	{
		if (width != 4 && width != 8)
			return false;
		if (position < 0 || position > data.size() - width)
			return false;

		quint64 raw = 0;
		for (int byte = 0; byte < width; ++byte)
		{
			raw = (raw << 8) |
				static_cast<unsigned char>(data.at(position + byte));
		}

		if (isUnsigned)
		{
			if (raw > static_cast<quint64>(std::numeric_limits<qint64>::max()))
				return false;
			value = static_cast<qint64>(raw);
			return true;
		}

		if (width == 4)
		{
			value = static_cast<qint32>(static_cast<quint32>(raw));
			return true;
		}

		if ((raw & (quint64(1) << 63)) == 0)
		{
			value = static_cast<qint64>(raw);
			return true;
		}

		const quint64 magnitude = (~raw) + 1;
		if (magnitude == (quint64(1) << 63))
			value = std::numeric_limits<qint64>::min();
		else
			value = -static_cast<qint64>(magnitude);
		return true;
	}

	bool parseBinaryIntegers(
		const QByteArray& data,
		int position,
		qint64 count,
		const QByteArray& typeName,
		int width,
		QVector<qint64>& values,
		int& end,
		QString& error)
	{
		if (count > std::numeric_limits<int>::max())
		{
			error = QStringLiteral("The cell array is too large for this VTK build.");
			return false;
		}
		if (count > (data.size() - position) / width)
		{
			error = QStringLiteral("The binary cell array is truncated.");
			return false;
		}

		values.clear();
		values.reserve(static_cast<int>(count));
		const bool unsignedValues = isUnsignedType(typeName);
		for (qint64 index = 0; index < count; ++index)
		{
			qint64 value = 0;
			if (!readBigEndianInteger(
				data,
				position + static_cast<int>(index) * width,
				width,
				unsignedValues,
				value))
			{
				error = QStringLiteral("The binary cell array contains an invalid integer.");
				return false;
			}
			values.append(value);
		}

		end = position + static_cast<int>(count) * width;
		return true;
	}

	bool parseArrayHeader(
		const QByteArray& data,
		int position,
		const QByteArray& expectedName,
		QByteArray& typeName,
		int& dataStart)
	{
		position = skipWhitespace(data, position);
		const int end = lineEnd(data, position);
		const QList<QByteArray> tokens = lineTokens(data, position, end);
		if (tokens.size() != 2 || tokens.at(0).toUpper() != expectedName)
			return false;
		typeName = tokens.at(1);
		dataStart = nextLineStart(data, position);
		return dataStart <= data.size();
	}

	bool validateTopology(
		const QVector<qint64>& offsets,
		const QVector<qint64>& connectivity,
		QString& error)
	{
		if (offsets.isEmpty() || offsets.first() != 0)
		{
			error = QStringLiteral("The cell offsets must start at zero.");
			return false;
		}
		if (offsets.last() != connectivity.size())
		{
			error = QStringLiteral("The final cell offset does not match the connectivity size.");
			return false;
		}

		const qint64 maximum = std::numeric_limits<qint32>::max();
		for (int index = 1; index < offsets.size(); ++index)
		{
			if (offsets.at(index) < offsets.at(index - 1) ||
				offsets.at(index) - offsets.at(index - 1) > maximum)
			{
				error = QStringLiteral("The cell offsets are not monotonic or exceed 32-bit limits.");
				return false;
			}
		}
		for (qint64 pointId : connectivity)
		{
			if (pointId < 0 || pointId > maximum)
			{
				error = QStringLiteral(
					"The mesh uses 64-bit point IDs that cannot be represented by VTK 8.2.");
				return false;
			}
		}
		return true;
	}

	void appendBigEndianInt32(QByteArray& output, qint64 value)
	{
		const quint32 raw = static_cast<quint32>(value);
		output.append(static_cast<char>((raw >> 24) & 0xff));
		output.append(static_cast<char>((raw >> 16) & 0xff));
		output.append(static_cast<char>((raw >> 8) & 0xff));
		output.append(static_cast<char>(raw & 0xff));
	}

	QByteArray packedTopology(
		const QByteArray& sectionName,
		const QVector<qint64>& offsets,
		const QVector<qint64>& connectivity,
		bool binary)
	{
		const qint64 cellCount = offsets.size() - 1;
		const qint64 packedValueCount = cellCount + connectivity.size();
		QByteArray output;
		output.append(sectionName);
		output.append(' ');
		output.append(QByteArray::number(cellCount));
		output.append(' ');
		output.append(QByteArray::number(packedValueCount));
		output.append('\n');

		if (binary)
		{
			if (packedValueCount <= std::numeric_limits<int>::max() / 4)
				output.reserve(output.size() + static_cast<int>(packedValueCount) * 4);
			for (int cell = 0; cell < offsets.size() - 1; ++cell)
			{
				const qint64 begin = offsets.at(cell);
				const qint64 end = offsets.at(cell + 1);
				appendBigEndianInt32(output, end - begin);
				for (qint64 index = begin; index < end; ++index)
					appendBigEndianInt32(output, connectivity.at(static_cast<int>(index)));
			}
			output.append('\n');
			return output;
		}

		for (int cell = 0; cell < offsets.size() - 1; ++cell)
		{
			const qint64 begin = offsets.at(cell);
			const qint64 end = offsets.at(cell + 1);
			output.append(QByteArray::number(end - begin));
			for (qint64 index = begin; index < end; ++index)
			{
				output.append(' ');
				output.append(QByteArray::number(connectivity.at(static_cast<int>(index))));
			}
			output.append('\n');
		}
		return output;
	}

	bool parseCellHeader(
		const QByteArray& data,
		int position,
		QByteArray& sectionName,
		qint64& offsetCount,
		qint64& connectivityCount,
		int& afterHeader)
	{
		const int end = lineEnd(data, position);
		const QList<QByteArray> tokens = lineTokens(data, position, end);
		if (tokens.size() != 3)
			return false;

		sectionName = tokens.at(0).toUpper();
		if (sectionName != "VERTICES" && sectionName != "LINES" &&
			sectionName != "POLYGONS" && sectionName != "TRIANGLE_STRIPS")
			return false;

		if (!parseNonNegativeCount(tokens.at(1), offsetCount) ||
			!parseNonNegativeCount(tokens.at(2), connectivityCount))
			return false;
		afterHeader = nextLineStart(data, position);
		return true;
	}

	bool parseAsciiCellSection(
		const QByteArray& data,
		int position,
		CellSection& section,
		QString& error)
	{
		QByteArray sectionName;
		qint64 offsetCount = 0;
		qint64 connectivityCount = 0;
		int cursor = 0;
		if (!parseCellHeader(
			data, position, sectionName, offsetCount, connectivityCount, cursor))
			return false;

		QByteArray offsetsType;
		if (!parseArrayHeader(data, cursor, "OFFSETS", offsetsType, cursor))
			return false;

		QVector<qint64> offsets;
		if (!parseAsciiIntegers(
			data, cursor, offsetCount, offsets, cursor, error))
			return false;

		QByteArray connectivityType;
		if (!parseArrayHeader(
			data, cursor, "CONNECTIVITY", connectivityType, cursor))
		{
			error = QStringLiteral("A CONNECTIVITY header is missing after the cell offsets.");
			return false;
		}

		QVector<qint64> connectivity;
		if (!parseAsciiIntegers(
			data, cursor, connectivityCount, connectivity, cursor, error))
			return false;
		if (!validateTopology(offsets, connectivity, error))
			return false;

		section.start = position;
		section.end = cursor;
		section.replacement =
			packedTopology(sectionName, offsets, connectivity, false);
		return true;
	}

	bool parseBinaryCellSection(
		const QByteArray& data,
		int position,
		CellSection& section,
		QString& error)
	{
		QByteArray sectionName;
		qint64 offsetCount = 0;
		qint64 connectivityCount = 0;
		int cursor = 0;
		if (!parseCellHeader(
			data, position, sectionName, offsetCount, connectivityCount, cursor))
			return false;

		QByteArray offsetsType;
		if (!parseArrayHeader(data, cursor, "OFFSETS", offsetsType, cursor))
			return false;

		const QVector<int> offsetWidths = integerWidths(offsetsType);
		if (offsetWidths.isEmpty())
		{
			error = QStringLiteral("Unsupported binary OFFSETS type: %1")
				.arg(QString::fromLatin1(offsetsType));
			return false;
		}

		QVector<qint64> offsets;
		QVector<qint64> connectivity;
		int sectionEnd = -1;
		QString lastError;
		for (int offsetWidth : offsetWidths)
		{
			int afterOffsets = 0;
			if (!parseBinaryIntegers(
				data,
				cursor,
				offsetCount,
				offsetsType,
				offsetWidth,
				offsets,
				afterOffsets,
				lastError))
				continue;

			QByteArray connectivityType;
			int connectivityStart = 0;
			if (!parseArrayHeader(
				data,
				afterOffsets,
				"CONNECTIVITY",
				connectivityType,
				connectivityStart))
				continue;

			const QVector<int> connectivityWidths = integerWidths(connectivityType);
			if (connectivityWidths.isEmpty())
			{
				lastError = QStringLiteral("Unsupported binary CONNECTIVITY type: %1")
					.arg(QString::fromLatin1(connectivityType));
				continue;
			}

			for (int connectivityWidth : connectivityWidths)
			{
				if (!parseBinaryIntegers(
					data,
					connectivityStart,
					connectivityCount,
					connectivityType,
					connectivityWidth,
					connectivity,
					sectionEnd,
					lastError))
					continue;
				if (validateTopology(offsets, connectivity, lastError))
					break;
				sectionEnd = -1;
			}
			if (sectionEnd >= 0)
				break;
		}

		if (sectionEnd < 0)
		{
			error = lastError.isEmpty()
				? QStringLiteral("The binary cell topology is malformed.")
				: lastError;
			return false;
		}

		section.start = position;
		section.end = sectionEnd;
		section.replacement =
			packedTopology(sectionName, offsets, connectivity, true);
		return true;
	}

	bool convertCellSections(
		const QByteArray& source,
		bool binary,
		QByteArray& converted,
		QString& error)
	{
		converted.clear();
		converted.reserve(source.size());
		int copyStart = 0;
		int lineStart = 0;
		bool convertedAnySection = false;
		bool foundCellSection = false;

		while (lineStart < source.size())
		{
			QByteArray sectionName;
			qint64 offsetCount = 0;
			qint64 connectivityCount = 0;
			int afterHeader = 0;
			foundCellSection = foundCellSection || parseCellHeader(
				source,
				lineStart,
				sectionName,
				offsetCount,
				connectivityCount,
				afterHeader);

			CellSection section;
			QString sectionError;
			const bool parsed = binary
				? parseBinaryCellSection(source, lineStart, section, sectionError)
				: parseAsciiCellSection(source, lineStart, section, sectionError);

			if (parsed)
			{
				converted.append(source.constData() + copyStart, section.start - copyStart);
				converted.append(section.replacement);
				copyStart = section.end;
				lineStart = section.end;
				convertedAnySection = true;
				continue;
			}

			if (!sectionError.isEmpty())
			{
				error = sectionError;
				return false;
			}
			lineStart = nextLineStart(source, lineStart);
		}

		if (!convertedAnySection)
		{
			if (!foundCellSection)
			{
				converted = source;
				return true;
			}
			error = QStringLiteral(
				"No VTK 5.x OFFSETS/CONNECTIVITY cell topology was found.");
			return false;
		}

		converted.append(source.constData() + copyStart, source.size() - copyStart);
		return true;
	}
}

legacyVtkCompatibility::PreparedInput
legacyVtkCompatibility::preparePolyDataForVtk82(const QString& fileName)
{
	PreparedInput result;
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly))
	{
		result.mode = InputMode::Error;
		result.error = QStringLiteral("Cannot open the selected VTK file: %1")
			.arg(file.errorString());
		return result;
	}

	const QByteArray firstLine = file.readLine();
	int major = 0;
	int minor = 0;
	int versionStart = 0;
	int versionLength = 0;
	if (!parseVersion(firstLine, major, minor, versionStart, versionLength))
	{
		result.mode = InputMode::Error;
		result.error = QStringLiteral("The selected file has no valid legacy VTK version header.");
		return result;
	}

	if (major < 4 || (major == 4 && minor <= 2))
		return result;
	if (major != 5 || minor > 1)
	{
		result.mode = InputMode::Error;
		result.error = QStringLiteral(
			"Legacy VTK version %1.%2 is newer than the supported 5.1 format. "
			"Please export the mesh as VTP or legacy VTK 4.2/5.1.")
			.arg(major)
			.arg(minor);
		return result;
	}

	file.readLine(); // Descriptive header.
	const QByteArray encoding = file.readLine().trimmed().toUpper();
	const bool binary = encoding == "BINARY";
	if (!binary && encoding != "ASCII")
	{
		result.mode = InputMode::Error;
		result.error = QStringLiteral("The legacy VTK encoding must be ASCII or BINARY.");
		return result;
	}
	if (file.size() > MaximumConvertibleFileSize)
	{
		result.mode = InputMode::Error;
		result.error = QStringLiteral(
			"This VTK 5.1 file is too large for the compatibility reader. "
			"Please convert it to VTP with a current VTK version.");
		return result;
	}

	if (!file.seek(0))
	{
		result.mode = InputMode::Error;
		result.error = QStringLiteral("Cannot rewind the selected VTK file.");
		return result;
	}
	const QByteArray source = file.readAll();
	if (source.size() != file.size())
	{
		result.mode = InputMode::Error;
		result.error = QStringLiteral("The selected VTK file could not be read completely.");
		return result;
	}

	QString conversionError;
	if (!convertCellSections(source, binary, result.data, conversionError))
	{
		result.mode = InputMode::Error;
		result.error = QStringLiteral("Cannot convert legacy VTK %1.%2 topology: %3")
			.arg(major)
			.arg(minor)
			.arg(conversionError);
		return result;
	}

	result.data.replace(versionStart, versionLength, "4.2");
	result.mode = InputMode::UseConvertedData;
	return result;
}
