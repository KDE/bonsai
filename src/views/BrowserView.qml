import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import org.mauikit.controls 1.3 as Maui
import org.mauikit.filebrowsing 1.3 as FB

StackView
{
    id: control

    ProjectView
    {
        id: _projectPage
        visible: StackView.status === StackView.Active
        headBar.farLeftContent: ToolButton
        {
            icon.name: "go-previous"
            onClicked: control.pop()
        }
    }

    initialItem: Maui.AltBrowser
    {
        id: _browser
        gridView.itemSize: 220
        gridView.cellHeight: 100
        viewType: root.isWide ? Maui.AltBrowser.Grid : Maui.AltBrowser.List
        showCSDControls: true
        holder.visible: count === 0
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
                onTriggered: openRepo()
            }
        ]

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

                    onClicked: openRepo()
                }

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

        headBar.middleContent: Maui.TextField
        {
            Layout.fillWidth: true
            Layout.maximumWidth: 500
            placeholderText: i18np("Filter", "Filter %1 repos", _browser.model.list.count);
            onAccepted: _browser.model.filter = text
            onCleared:  _browser.model.filter = text
        }

        headBar.rightContent:[
            Maui.ToolButtonMenu
            {
                id: _viewMenu
                icon.name: _browser.viewType === Maui.AltBrowser.List ? "view-list-details" : "view-list-icons"

                Maui.MenuItemActionRow
                {
                    Action
                    {
                        text: i18n("List")
                        icon.name: "view-list-details"
                        checked: _browser.viewType === Maui.AltBrowser.List
                        checkable: true
                        onTriggered: _browser.viewType = Maui.AltBrowser.List
                    }

                    Action
                    {
                        text: i18n("Grid")
                        icon.name: "view-list-icons"
                        checked: _browser.viewType === Maui.AltBrowser.Grid
                        checkable: true
                        onTriggered: _browser.viewType = Maui.AltBrowser.Grid
                    }
                }

                MenuSeparator {}

                Maui.LabelDelegate
                {
                    width: parent.width
                    isSection: true
                    label: i18n("Sort by")
                }

                Action
                {
                    text: i18n("Date")
                    checked: _projectsModel.sort === "lastread"
                    checkable: true
                    onTriggered:
                    {
                        _projectsModel.sort = "lastread"
                        _projectsModel.sortOrder = Qt.DescendingOrder
                    }
                }

                Action
                {
                    text: i18n("Modified")
                    checked: _projectsModel.sort === "modified"
                    checkable: true
                    onTriggered:
                    {
                        _projectsModel.sort = "modified"
                        _projectsModel.sortOrder = Qt.DescendingOrder
                    }
                }

                Action
                {
                    text: i18n("Name")
                    checked: _projectsModel.sort === "title"
                    checkable: true

                    onTriggered:
                    {
                        _projectsModel.sort = "title"
                        _projectsModel.sortOrder =  Qt.AscendingOrder
                    }
                }
            }
            ,
            ToolButton
            {
                icon.name: "list-add"
                onClicked:
                {
                    _dialogLoader.sourceComponent = _newDialogComponent
                    dialog.open()
                }
            }

        ]

        model: Maui.BaseModel
        {
            id: _projectsModel
            list: _projectManager.projectsModel

            sort: "title"
            sortOrder: Qt.AscendingOrder
            recursiveFilteringEnabled: true
            sortCaseSensitivity: Qt.CaseInsensitive
            filterCaseSensitivity: Qt.CaseInsensitive
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
                imageSource: model.artwork
                template.headerSizeHint: 64
                iconSizeHint: 32
                template.imageSizeHint: iconSizeHint
                label1.font.pointSize: Maui.Style.fontSizes.huge
                label1.font.bold: true
                label1.font.weight: Font.ExtraBold
                label1.wrapMode: Text.Wrap
                label1.text: model.title
                label2.text: model.label

                onClicked:
                {
                    _browser.currentIndex = index
                    if(Maui.Handy.singleClick || Kirigami.Settings.isMobile)
                    {
                        openProject(model.url)
                    }
                }

                onDoubleClicked:
                {
                    _browser.currentIndex = index
                    if(!Maui.Handy.singleClick)
                    {
                        openProject(model.url)
                    }
                }
            }
        }

        listDelegate:  Maui.ListBrowserDelegate
        {
            width: ListView.view.width
            height: implicitHeight + Maui.Style.space.big
            iconSource: model.icon
            imageSource: model.artwork
            template.headerSizeHint: 48
            iconSizeHint: 32
            template.imageSizeHint: iconSizeHint
            label1.font.pointSize: Maui.Style.fontSizes.large
            label1.font.bold: true
            label1.font.weight: Font.ExtraBold
            label1.text: model.title
            label2.text: model.url

            label3.text : i18np("%1 item", "%1 items", model.count)
            label4.text: Maui.Handy.formatDate(model.lastread, "MM/dd/yyyy")

        }
    }

    function openProject(url)
    {
        _projectManager.addProject(url)
        control.push(_projectPage)
        control.currentItem.url = url
    }
}
