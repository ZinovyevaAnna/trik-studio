#include <QtWidgets/QApplication>
#include <QtGui/QClipboard>

#include "editor/copyPaste/clipboardHandler.h"
#include "editor/copyPaste/pasteGroupCommand.h"
#include "editor/editorViewScene.h"
#include "mainWindow/mainWindow.h"

using namespace qReal;

ClipboardHandler::ClipboardHandler(EditorViewScene &scene, Controller &controller)
	: mScene(scene)
	, mController(controller)
{
}

void ClipboardHandler::cut()
{
	copy();
	mScene.deleteSelectedItems();
}

void ClipboardHandler::copy()
{
	QList<NodeElement *> nodes = getNodesForCopying();

	QList<NodeData> nodesData = getNodesData(nodes);
	QList<EdgeData> edgesData = getEdgesData();

	pushDataToClipboard(nodesData, edgesData);
}

QList<NodeData> ClipboardHandler::getNodesData(QList<NodeElement *> const &nodes)
{
	QList<NodeData> nodesData;
	for (NodeElement * const node : nodes) {
		nodesData << node->data();
	}

	return nodesData;
}

QList<NodeElement *> ClipboardHandler::getNodesForCopying()
{
	QList<NodeElement *> nodes;
	for (QGraphicsItem * const item : mScene.selectedItems()) {
		NodeElement *node = dynamic_cast<NodeElement *>(item);
		if (node && !mScene.selectedItems().contains(node->parentItem())) {
			nodes << node;
		}
	}

	for (NodeElement * const node : nodes) {
		addChildren(node, nodes);
	}

	return nodes;
}

void ClipboardHandler::addChildren(NodeElement *node, QList<NodeElement *> &nodes)
{
	foreach (QGraphicsItem *item, node->childItems()) {
		NodeElement *child = dynamic_cast<NodeElement *>(item);
		if (child && !nodes.contains(child)) {
			nodes << child;
			addChildren(child, nodes);
		}
	}
}

QList<EdgeData> ClipboardHandler::getEdgesData()
{
	QList<EdgeData> edgesData;
	for (QGraphicsItem *item : mScene.selectedItems()) {
		EdgeElement *edge = dynamic_cast<EdgeElement *>(item);
		if (edge) {
			edgesData << edge->data();
		}
	}

	return edgesData;
}

void ClipboardHandler::pushDataToClipboard(QList<NodeData> const &nodesData, QList<EdgeData> const &edgesData)
{
	QByteArray data;
	QDataStream stream(&data, QIODevice::WriteOnly);

	stream << nodesData;
	stream << edgesData;

	QMimeData *mimeData = new QMimeData();
	mimeData->setData("application/x-real-uml-model-data", data);

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setMimeData(mimeData);
}

void ClipboardHandler::paste(bool isGraphicalCopy)
{
	commands::PasteGroupCommand *pasteCommand = new commands::PasteGroupCommand(&mScene, isGraphicalCopy);
	if (!pasteCommand->isEmpty()) {
		mController.execute(pasteCommand);
	}
}
