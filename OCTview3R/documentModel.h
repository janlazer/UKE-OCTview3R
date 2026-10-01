#pragma once

#include <memory>
#include <vector>

struct ImageData;

/**
 * @brief GUI-thread owner of datasets, ordered like the document tabs.
 *
 * Returned pointers/references borrow model-owned objects. Creating another
 * document does not move an ImageData object, but removing/clearing it ends its
 * lifetime unless takeAt() transfers ownership to the caller. Renderer props
 * and widget observers must be detached before a document is destroyed.
 * This class has no rendering side effects and is not thread-safe.
 */
class DocumentModel final
{
public:
	using Document = std::unique_ptr<ImageData>;
	using Documents = std::vector<Document>;

	DocumentModel() = default;
	~DocumentModel();

	DocumentModel(const DocumentModel&) = delete;
	DocumentModel& operator=(const DocumentModel&) = delete;

	/// Allocate VTK resources, append a document, and make it active; no file is loaded yet.
	ImageData& create();
	/// Transfer ownership; invalid indices return empty. Select the next/last remaining index.
	Document takeAt(int index);
	/// Destroy all documents; call ViewerController::clear() or shutdown() first.
	void clear();

	/// Borrow a document, or nullptr for an out-of-range index.
	ImageData* at(int index) const;
	/// Borrow the selected document, or nullptr when no valid selection exists.
	ImageData* active() const;
	/// Return the selected tab index, or -1 for no selection.
	int activeIndex() const;
	/// Store the index; invalid values clear the selection without deleting data.
	void setActiveIndex(int index);
	/// Find a borrowed pointer by identity; return -1 if it is not owned here.
	int indexOf(const ImageData* document) const;

	int size() const;
	bool empty() const;
	/// Borrow the container for iteration; structural changes invalidate its iterators.
	const Documents& documents() const;

private:
	Documents m_documents;
	int m_activeIndex = -1;
};
