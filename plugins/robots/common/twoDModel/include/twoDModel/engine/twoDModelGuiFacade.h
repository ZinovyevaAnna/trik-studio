#pragma once

#include <QtWidgets/QWidget>

#include "twoDModel/twoDModelDeclSpec.h"

namespace twoDModel {

namespace view {
class TwoDModelWidget;
}

namespace engine {

/// Implemented facade pattern. This class allows to search and manage objects of plugin`s user interface
/// from scripting language by their object names, parents and types.
class TWO_D_MODEL_EXPORT TwoDModelGuiFacade : public QObject
{
	Q_OBJECT

public:
	explicit TwoDModelGuiFacade(view::TwoDModelWidget &d2RobotWidget);

	/// Searches and returns widget by type and object name.
	Q_INVOKABLE QWidget *widget(const QString &type, const QString &name) const;

	/// Returns 2D model window widget.
	Q_INVOKABLE QWidget *twoDModelWidget() const;

	/// Returns the underlying widget of the 2D model scene.
	Q_INVOKABLE QWidget *twoDModelSceneViewport() const;

	/// Returns a reference to 2D model widget if it is hosted in separete window at the moment.
	/// Otherwise returns nullptr.
	Q_INVOKABLE QWidget *separateTwoDModelWindow() const;

private:
	view::TwoDModelWidget &mD2ModelWidget;
};

}
}
