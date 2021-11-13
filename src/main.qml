import QtQuick 2.15
import QtQuick.Controls 2.15
import org.mauikit.controls 1.3 as Maui
import org.mauikit.filebrowsing 1.3 as FB
import org.maui.bonsai 1.0 as Bonsai

import "views"

Maui.ApplicationWindow
{
    id: root
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    headBar.visible: false

    Bonsai.GitOperations
    {
        id: _gitOperations
    }

    Bonsai.ProjectManager
    {
        id: _projectManager
    }

    Maui.NewDialog
    {
        id: cloneDialog
        title: i18n("Git URL")
        textEntry.placeholderText: i18n("URL")

        onFinished: _gitOperations.clone(text, "file:///home/camilo/Documents/basket")
    }

    property alias dialog : _dialogLoader.item

    Loader
    {
        id: _dialogLoader
    }

    Component
    {
        id: _openFileDialogComponent

        FB.FileDialog
        {

        }
    }

    BrowserView
    {
        id: _browserView
        anchors.fill: parent
    }

    function openRepo()
    {
        _dialogLoader.sourceComponent = _openFileDialogComponent
        dialog.singleSelection = true
        dialog.callback = function(paths)
        {
            console.log("Paths", paths)
            const url = paths[0]
            _projectManager.addProject(url)
            _browserView.openProject(url)
        }

        dialog.open()
    }

}
