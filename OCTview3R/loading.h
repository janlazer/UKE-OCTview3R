#pragma once

#include <QObject>
#include <QMetaType>
#include <QString>

#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

#include "dataTypes.h"

class OpenData;
class OpenPoly;
class vtkAlgorithm;
class vtkObject;

Q_DECLARE_METATYPE(vtkImageData*)
Q_DECLARE_METATYPE(vtkPolyData*)

/**
 * @brief One-shot file reader moved from the GUI thread to a QThread.
 *
 * Constructors snapshot dialog parameters on the GUI thread; loadData/loadPoly
 * never access the dialogs or renderer. Success transfers one extra VTK reference
 * through the result signal. Exactly one receiving owner must adopt or release
 * that reference (the main-window result slot does so); other listeners borrow.
 * finished() follows success/failure and drives worker/thread cleanup.
 */
class loading final : public QObject
{
	Q_OBJECT

public:
	explicit loading(OpenData* openData);
	explicit loading(OpenPoly* openPoly);
	~loading() override = default;

public slots:
	/// Read a volume, apply supported TIFF calibration, and emit dataLoaded or failed.
	void loadData();
	/// Read geometry, including the legacy VTK compatibility path, and emit its result.
	void loadPoly();

signals:
	/// Reader-dependent progress in 0..100; not every VTK reader reports intermediate values.
	void updateProgress(int progress);
	/** Deliver a volume with one transfer reference, even after the worker is deleted.
	 * spacingInMillimetresMask uses bits 0/1/2 for physically calibrated X/Y/Z.
	 * Display bounds describe scalar values or a conservative RGB luminance range.
	 */
	void dataLoaded(
		vtkImageData* data,
		unsigned int spacingInMillimetresMask,
		double displayScalarMinimum,
		double displayScalarMaximum);
	/// Deliver geometry with the same single-owner transfer-reference contract as dataLoaded.
	void polyLoaded(vtkPolyData* poly);
	void failed(const QString& message);
	/// Terminal notification, including failures; this is not a cancellation acknowledgement.
	void finished();

private:
	void setData(OpenData* openData);
	void setPoly(OpenPoly* openPoly);
	void reportFailure(const QString& message);
	void reportProgress(int progress);
	void updateWithProgress(vtkAlgorithm* algorithm, int startProgress, int endProgress);
	static void vtkProgressCallback(
		vtkObject* caller,
		unsigned long eventId,
		void* clientData,
		void* callData);

	vtkSmartPointer<vtkImageData> m_data;
	vtkSmartPointer<vtkPolyData> m_poly;
	QString m_fileName;
	bitsizeType m_bitsize = bitsizeType::BIT_UNDEF;
	endianType m_endian = endianType::ENDIAN_UNDEF;
	dataType m_dataFormat = dataType::DATA_UNDEF;
	polyType m_polyFormat = polyType::POLY_UNDEF;
	int m_width = 0;
	int m_height = 0;
	int m_depth = 0;
	int m_lastProgress = -1;
	unsigned int m_spacingInMillimetresMask = 0;
};
