#pragma once

#include <memory>
#include <vector>

struct ImageData;

class DocumentModel final
{
public:
	using Document = std::unique_ptr<ImageData>;
	using Documents = std::vector<Document>;

	DocumentModel() = default;
	~DocumentModel();

	DocumentModel(const DocumentModel&) = delete;
	DocumentModel& operator=(const DocumentModel&) = delete;

	ImageData& create();
	Document takeAt(int index);
	void clear();

	ImageData* at(int index) const;
	ImageData* active() const;
	int activeIndex() const;
	void setActiveIndex(int index);
	int indexOf(const ImageData* document) const;

	int size() const;
	bool empty() const;
	const Documents& documents() const;

private:
	Documents m_documents;
	int m_activeIndex = -1;
};
