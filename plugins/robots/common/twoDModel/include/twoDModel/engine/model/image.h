/* Copyright 2016 CyberTech Labs Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#pragma once

#include <QtCore/QScopedPointer>
#include <QtCore/QString>
#include <QtCore/QRect>

#include <qrutils/imagesCache.h>

class QDomElement;
class QSvgRenderer;
class QImage;
class QPainter;

namespace twoDModel {
namespace model {

/// Represents external or internal image. External image is a path to image file, internal image -- memorized
/// image bytes.
class Image
{
	Q_DISABLE_COPY(Image)
public:
	explicit Image(const QString &id);
	Image(const QString &path, bool memorize);
	~Image();

	/// Reads image from XML-representation.
	static QSharedPointer<Image> deserialize(const QDomElement &element);

	/// Returns true if image was successfully loaded.
	bool isValid() const;

	/// Returns the size of the image.
	QSize preferedSize() const;

	/// Saves this image into a given XML-element.
	void serialize(QDomElement &target) const;

	/// Returns true if this image is somewhere in environment (not embedded).
	bool external() const;

	/// Memorizes or forgets image bytes.
	void setExternal(bool external);

	/// Returns true if this item is embedded into save.
	QString path() const;

	/// Loads from file.
	void loadFrom(const QString &path);

	/// Draws image with \a painter inside the \a rect considering \a zoom.
	void draw(QPainter &painter, const QRect &rect, qreal zoom = 1.0) const;

	/// Returns imageId for this image.
	QString imageId() const;

private:
	QSize preferedSvgSize() const;

	bool mExternal { true };
	bool mIsSvg { false };
	QString mPath;
	QString mImageId;
	QScopedPointer<QImage> mImage;
	QByteArray mSvgBytes;
	QScopedPointer<QSvgRenderer> mSvgRenderer;
	const QSharedPointer<utils::ImagesCache> mImagesCache;
};

}
}
