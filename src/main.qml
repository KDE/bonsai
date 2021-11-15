import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.7 as Kirigami

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


    Component
    {
        id: _newDialogComponent

        Maui.Dialog
        {
            title: i18n("New Repo")
            maxWidth: 350

            page.padding: Maui.Style.space.medium
            spacing: Maui.Style.space.medium
            persistent: false
            defaultButtons: false
            rejectButton.visible : false
            acceptButton.visible: false

            Maui.ListBrowserDelegate
            {
                Layout.fillWidth: true
                implicitHeight: 80

                template.headerSizeHint: iconSizeHint + Maui.Style.space.big
                iconSizeHint: Maui.Style.iconSizes.big
                iconSource: "vcs-diff"
                label1.text: i18n("Clone")
                label2.text: i18n("Clone a repository")

                onClicked: cloneDialog.open()
            }


            Maui.ListBrowserDelegate
            {
                Layout.fillWidth: true
                implicitHeight: 80

                template.headerSizeHint: iconSizeHint + Maui.Style.space.big
                iconSizeHint: Maui.Style.iconSizes.big
                iconSource: "folder-new"
                label1.text: i18n("Create")
                label2.text: i18n("Create a new repository")

            }

            Maui.ListBrowserDelegate
            {
                Layout.fillWidth: true
                implicitHeight: 80

                template.headerSizeHint: iconSizeHint + Maui.Style.space.big
                iconSizeHint: Maui.Style.iconSizes.big
                iconSource: "document-open"
                label1.text: i18n("Open")
                label2.text: i18n("Open a local repository")

                onClicked: openLocalRepo()
            }
        }
    }


    sideBar: Maui.AbstractSideBar
    {
        collapsible: true
        visible: true
        collapsed: !root.isWide
        preferredWidth: Kirigami.Units.gridUnit * 18
        BrowserView
        {
            id: _browserView
            anchors.fill: parent
        }
    }

Maui.Page
{
    anchors.fill: parent

    headBar.rightContent: ToolButton
    {
        icon.name: "list-add"
        onClicked:
        {
            _dialogLoader.sourceComponent = _newDialogComponent
            dialog.open()
        }
    }

    headBar.farLeftContent: Loader
    {
        asynchronous: true
        sourceComponent: ToolButton
        {
            icon.name: sideBar.visible ? "sidebar-collapse" : "sidebar-expand"
            onClicked: sideBar.toggle()
            checked: sideBar.visible
            ToolTip.delay: 1000
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            ToolTip.text: i18n("Toogle SideBar")
        }
    }

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

    Maui.TabView
    {
        id: _tabView
//        mobile: true
        anchors.fill: parent
        holder.title : i18n("Let's Start")
        holder.body: i18n("Open or clone an existing repository, or create a new one.")
        holder.emoji: "qrc:/assets/assets/folder-add.svg"

        holder.actions:[ Action
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
                onTriggered: openLocalRepo()
            }
        ]
    }

}

    Component
    {
        id: _projectPageComponent

        ProjectView
        {

        }
    }

    function openLocalRepo()
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

    function openProject(url)
    {
        _projectManager.addProject(url)

        const index = tabIndex(url)
        if(index > -1)
        {
            _tabView.setCurrentIndex(index)
            return
        }

        _tabView.addTab(_projectPageComponent, {'url' : url})
    }

    function tabIndex(path) //find the tab index for a path
    {
        if(path.length === 0)
        {
            return -1
        }

        for(var i = 0; i < _tabView.count; i++)
        {
            const tab =  _tabView.contentModel.get(i)
            console.log("FIN TAB INDEX", i, tab.url, path)
            if(tab.url.toString() === path)
            {
                return i
            }
        }
        return -1
    }
}
