import QtQuick 2.15
import QtQuick.Controls 2.15
import org.mauikit.controls 1.3 as Maui
import org.mauikit.filebrowsing 1.3 as FB
import org.maui.bonsai 1.0 as Bonsai

Maui.ApplicationWindow
{
    id: root
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    headBar.leftContent: Maui.ToolButtonMenu
    {
        icon.name: "application-menu"
        MenuItem
        {
            text: i18n("Settings")
            icon.name: "settings-configure"
            onTriggered: openConfigDialog()
        }

        MenuItem
        {
            text: i18n("About")
            icon.name: "documentinfo"
            onTriggered: root.about()
        }
    }

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

    Maui.AltBrowser
    {
        anchors.fill: parent

        gridView.itemSize: 200
        gridView.cellHeight: 120
viewType: Maui.AltBrowser.Grid

holder.visible: count === 0
        holder.title : i18n("Let's Start")
        holder.body: i18n("Open or clone an existing repository, or create a new one.")
        holder.emoji: "qrc:/assets/assets/folder-add.svg"

        holder.actions:[Action
            {
                text: "Clone"
                onTriggered: cloneDialog.open()
            },

            Action
            {
                text: "Create"
            },

            Action
            {
                text: "Open"
                onTriggered:
                {
                    _dialogLoader.sourceComponent = _openFileDialogComponent
                    dialog.singleSelection = true
                    dialog.callback = function(paths)
                    {
                        console.log("Paths", paths)
                        _projectManager.openProject(paths[0])
                    }

                    dialog.open()
                }
            }
        ]

        model: Maui.BaseModel
        {
            list: _projectManager.projectsModel
        }

        gridDelegate: Item
        {
            width: GridView.view.cellWidth
            height: GridView.view.cellHeight

            Maui.ListBrowserDelegate
            {
                anchors.fill: parent
                anchors.margins: Maui.Style.space.medium

                iconSource: model.icon
                template.headerSizeHint: 32
                iconSizeHint: 22
                label1.text: model.title
                label2.text: model.url
            }
        }
    }


}
