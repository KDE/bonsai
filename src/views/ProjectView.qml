import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import org.mauikit.controls 1.3 as Maui
import org.mauikit.filebrowsing 1.3 as FB
import org.mauikit.texteditor 1.0 as TE
import org.mauikit.terminal 1.0 as Term

import org.maui.bonsai 1.0 as Bonsai

Maui.Page
{
    id: control

    Maui.TabViewInfo.tabTitle: title
    Maui.TabViewInfo.tabToolTipText: url

    property alias project: _project
    property alias url : _project.url

    property alias dialog : _dialogLoader.item

    headBar.visible: false

    Bonsai.Project
    {
        id: _project
    }

    title: project.title
    showTitle: false

    footBar.leftContent: Maui.ToolActions
    {
        id: _actionViews
        expanded: true
        autoExclusive: true
        display: ToolButton.IconOnly

        Action
        {
            icon.name: "deep-history"
            text: i18n("Commits")
            checked:  _stackView.depth === 1
            onTriggered: _stackView.pop()
        }

        Action
        {
            icon.name: "folder"
            text: i18n("Browser")
            checked:  _stackView.depth === 2
            onTriggered:
            {
                if(_stackView.depth === 1)
                {
                    _stackView.push(_browserViewComponent)
                }
            }
        }
    }

    footBar.rightContent: [

        ToolButton
        {
            icon.name: "utilities-terminal-symbolic"
            checked: _terminal.visible
            onClicked: _terminal.visible = !_terminal.visible
        },

        ToolButton
        {
            icon.name: "folder-remote"
            onClicked:
            {
                _dialogLoader.sourceComponent = _remoteDialogComponent
                control.dialog.open()
            }
        },

        ToolButton
        {
            visible: String(_project.readmeFile).length > 0
            icon.name: "documentinfo"
            onClicked:
            {
                _dialogLoader.sourceComponent = _readmeDialogComponent
                control.dialog.open()
            }
        },

        Maui.ToolButtonMenu
        {

            icon.name: "vcs-branch"
            Repeater
            {
                model: _project.allBranches
                MenuItem
                {
                    autoExclusive: true
                    checked: _project.currentBranch === text
                    text: modelData
                    onTriggered: _project.currentBranch = text
                }
            }
        }]

    Loader
    {
        id: _dialogLoader
    }

    Component
    {
        id: _remoteDialogComponent

        Maui.Dialog
        {
            defaultButtons: false
            persistent: false
            maxWidth: 800
            maxHeight: 600

            Maui.SectionGroup
            {
                title: i18n("Head")
                Maui.SectionItem
                {
                    label1.text: _project.headBranch.name
                    label2.text: _project.headBranch.upstreamRemoteName

                    CheckBox
                    {
                        visible: _project.headBranch.isCurrentBranch
                        checked: visible
                        checkable: false
                    }
                }

                Maui.SectionItem
                {
                    label1.text: _project.headBranch.prefix
                    label2.text: _project.headBranch.isLocal

                }

            }

            Maui.SectionGroup
            {
                title: i18n("Remotes")
                Repeater
                {
                    model: _project.remotesModel
                    delegate: Maui.SectionItem
                    {
                        property string remoteName : modelData.name
                        label1.text: remoteName
                        label2.text: modelData.url
                        enabled: modelData.isValid

                        Maui.CheckBoxItem
                        {
                            checked: _project.currentBranchRemote.name === remoteName
                            visible: checked
                        }
                    }
                }
            }

            Maui.SectionGroup
            {
                title: i18n("Remote Branches")

                Repeater
                {
                    model: _project.branches.localBranches
                    delegate: Maui.SectionItem
                    {
                        label1.text: modelData
                        label2.text: _project.branches.upstreamRemote(modelData)
                    }
                }
            }

        }
    }

    Component
    {
        id: _readmeDialogComponent

        Maui.Dialog
        {
            maxHeight: 800
            maxWidth: 900
            defaultButtons: false
            hint: 1
            title: "README.MD"
            stack: TE.TextEditor
            {
                Layout.fillWidth: true
                Layout.fillHeight: true
                fileUrl: _project.readmeFile
                //                    document.isRich: true
                body.textFormat: TextEdit.MarkdownText
                body.readOnly: true
            }
        }
    }

    Component
    {
        id: _commitInfoDialogComponent

        Maui.Dialog
        {
            property string commitId
            id: _infoDialog
            readonly property var info : _project.commitAuthor(commitId)
            defaultButtons: false
            persistent: false
            maxWidth: 500
            //            maxHeight: 400

            Maui.SectionGroup
            {
                title: i18n("Author")

                Maui.SectionItem
                {
                    label1.text: i18n("Full Name")
                    label2.text: info.fullName
                }

                Maui.SectionItem
                {
                    label1.text: i18n("Email")
                    label2.text: info.email

                    ToolButton
                    {
                        icon.name: "mail-message"
                        onClicked: Qt.openUrlExternally("mailto:"+info.email)
                    }
                }

                Maui.SectionItem
                {
                    label1.text: i18n("ID")
                    label2.text: commitId

                    ToolButton
                    {
                        icon.name: "edit-copy"
                        onClicked: Maui.Handy.copyTextToClipboard(commitId)
                    }
                }

                Maui.SectionItem
                {
                    label1.text: i18n("Message")
                    label2.text: info.message

                    ToolButton
                    {
                        icon.name: "edit-copy"
                        onClicked: Maui.Handy.copyTextToClipboard(info.message)
                    }
                }

                Maui.SectionItem
                {
                    label1.text: i18n("Date")
                    label2.text: Qt.formatDateTime(info.date, "dd mm yyyy")
                }

                Maui.SectionItem
                {
                    id: _parentCommitField
                    label1.text: i18n("Parent Commit")
                    label2.text: info.parentCommits
                    label2.textFormat: Text.AutoText
                    Connections
                    {
                        target: _parentCommitField.label2
                        function onLinkActivated(link)
                        {
                            _infoDialog.commitId = link.replace("hash:", "")
                        }
                    }
                }

                Maui.SectionItem
                {
                    id: _childCommitField
                    label1.text: i18n("Child Commit")
                    label2.text: info.childCommits
                    label2.textFormat: Text.AutoText
                    Connections
                    {
                        target: _childCommitField.label2
                        function onLinkActivated(link)
                        {
                            _infoDialog.commitId = link.replace("hash:", "")
                        }
                    }
                }
            }

            Maui.SectionGroup
            {
                title: i18n("Repo")
                Maui.SectionItem
                {
                    label1.text: i18n("Local Path")
                    label2.text: _project.url
                }
            }


            Maui.SectionGroup
            {
                title: i18n("Files")
                Maui.SectionItem
                {
                    label1.text: i18n("Changed Files")
                    columns: 1

                    ColumnLayout
                    {
                        Layout.fillWidth: true

                        Repeater
                        {
                            model: info.changedFiles
                            delegate: Label
                            {
                                Layout.fillWidth: true
                                text: modelData.url
                                color: modelData.color
                            }
                        }
                    }
                }
            }
        }
    }

    Maui.Holder
    {
        anchors.fill: parent
        title:
        {
            switch(project.status.code)
            {
            case Bonsai.StatusMessage.Loading: return i18n("Loading...");
            case Bonsai.StatusMessage.Error: return i18n("Error");
            }
        }

        emoji: "indicator-messages"
        body: project.status.message
    }

    Maui.SplitView
    {
        anchors.fill: parent
        orientation: Qt.Vertical

        StackView
        {
            id: _stackView
            SplitView.fillWidth: true
            SplitView.fillHeight: true
            clip: true
            visible: project.status.code === Bonsai.StatusMessage.Ready

            initialItem: Maui.Page
            {

                headBar.forceCenterMiddleContent: root.isWide
                headBar.middleContent: Maui.SearchField
                {
                    Layout.alignment: Qt.AlignCenter
                    Layout.maximumWidth: 500
                    Layout.fillWidth: true
                    placeholderText: i18n("Filter commit by id, message or author")
                    onAccepted: _commitsListView.model.filters = text.split(",")
                    onCleared: _commitsListView.model.clearFilters()
                }

                headBar.rightContent: Maui.ToolButtonMenu
                {
                    icon.name: "view-sort"
                    autoExclusive: true

                    MenuItem
                    {
                        text: i18n("Author")
                    }

                    MenuItem
                    {
                        text: i18n("Date")
                    }

                    MenuSeparator{}

                    MenuItem
                    {
                        text: i18n("Asc")
                    }

                    MenuItem
                    {
                        text: i18n("Desc")
                    }
                }


                Maui.ListBrowser
                {
                    id: _commitsListView
                    currentIndex: -1
                    anchors.fill: parent


                    flickable.header: Column
                    {
                        width: parent.width

                        Maui.SectionHeader
                        {
                            topPadding: Maui.Style.space.big
                            bottomPadding: topPadding + _commitsListView.topPadding
                            padding: _commitsListView.padding

                            width: parent.width
                            template.iconSource: _project.logo
                            template.iconSizeHint: Maui.Style.iconSizes.big

                            label1.text: _project.title
                            label2.text: _project.currentBranch
                            //                        template.label3.text: _project.currentBranchRemote.name
                            //                        template.label4.text: _project.currentBranchRemote.url

                            template.content:[ Maui.GridItemTemplate
                                {
                                    implicitWidth: 58
                                    //                            implicitHeight: 64

                                    iconSource: "vcs-commit"
                                    iconSizeHint: Maui.Style.iconSizes.small
                                    label1.text:  _commitsListView.count
                                    label1.font.pointSize: Maui.Style.fontSizes.small
                                },
                                Maui.GridItemTemplate
                                {
                                    implicitWidth: 58
                                    iconVisible: true
                                    //                            implicitHeight: 64

                                    iconSizeHint: Maui.Style.iconSizes.small

                                    iconSource: "vcs-branch"
                                    label1.text:   _project.allBranches.length
                                    label1.font.pointSize: Maui.Style.fontSizes.small
                                }

                            ]
                        }

                        //                    Maui.SectionHeader
                        //                    {
                        //                        topPadding: Maui.Style.space.big
                        //                        bottomPadding: topPadding + _commitsListView.topPadding
                        //                        padding: _commitsListView.padding
                        //                        width: parent.width
                        //                        label1.text: _project.url
                        //                        label2.text: _project.currentBranch
                        //                    }
                    }

                    model: project.commitsModel

                    delegate: Maui.ListBrowserDelegate
                    {
                        isCurrentItem: ListView.isCurrentItem
                        width: ListView.view.width
                        height:  ListView.isCurrentItem ? implicitHeight : Math.min(100, implicitHeight)
                        label1.text: model.subject
                        label3.text: model.hash
                        label2.text: model.author
                        label4.text: Qt.formatDateTime(model.date, "dd MM yyyy")
                        rightLabels.visible: true

                        onClicked:
                        {
                            _commitsListView.currentIndex = index
                        }

                        onDoubleClicked:
                        {
                            _commitsListView.currentIndex = index
                            openCommitInfoDialog(model.hash)
                        }


                        onRightClicked:
                        {
                            _commitsListView.currentIndex = index
                            _commitMenu.show()
                        }
                    }

                    Maui.ContextualMenu
                    {
                        id: _commitMenu

                        MenuItem
                        {
                            text: i18n("Checkout")
                        }

                        MenuItem
                        {
                            text: i18n("Info")
                            onTriggered: openCommitInfoDialog(_commitsModel.get(_commitsListView.currentIndex).id)
                        }


                        MenuItem
                        {
                            text: i18n("Copy ID")
                            onTriggered: Maui.Handy.copyTextToClipboard(_commitsModel.get(_commitsListView.currentIndex).id)
                        }
                    }
                }
            }

            Component
            {
                id:  _browserViewComponent

                Maui.Page
                {
                    id: _browserView
                    title: _browser.title
                    showTitle: true

                    FB.FileBrowser
                    {
                        id: _browser
                        anchors.fill: parent
                        currentPath: control.project.url
                        onItemClicked: openItem(index)
                        settings.viewType: FB.FMList.LIST_VIEW
                        settings.showHiddenFiles: true
                        browser.delegateInjector: Rectangle
                        {
                            radius: Maui.Style.radiusV
                            color: Maui.Theme.backgroundColor
                            Maui.Icon
                            {
                                visible: itemData.isdir
                                source: _project.fileStatusIcon(itemData.url)
                                anchors.centerIn: parent
                            }
                        }

                        //            gridItemSize: 100
                    }

                    headBar.leftContent: [

                        ToolButton
                        {
                            icon.name: "go-up"
                            onClicked:
                            {
                                //                        if(_browser.currentFMList.parentPath)
                                _browser.goUp()
                            }
                        }

                    ]

                    headBar.rightContent: [

                        Maui.ToolButtonMenu
                        {
                            icon.name: "list-add"
                            MenuItem
                            {
                                text: i18n("New Branch")
                                icon.name: "branch"
                            }

                            MenuItem
                            {
                                text: i18n("New Tag")
                                icon.name: "tag"
                            }

                            MenuSeparator{}

                            MenuItem
                            {
                                text: i18n("New File")
                                icon.name: "document-new"
                            }
                        }

                    ]

                    footBar.enabled: !_terminal.session.hasActiveProcess
                    footBar.leftContent: [

                        ToolButtonOp
                        {
                            icon.name: "vcs-pull"
                            text: i18n("Pull")
                            display: ToolButton.TextBesideIcon

                            onClicked:
                            {
                                runCommand("git pull")
                                //                                control.project.pull()
                            }

                            Menu
                            {
                                title: i18n("Branch")
                            }

                            Menu
                            {
                                title: i18n("Remote")
                            }

                            MenuSeparator {}

                            Column
                            {
                                width: ListView.view.width
                                spacing: Maui.Style.defaultSpacing
                                MenuItem
                                {
                                    width: parent.width
                                    text: i18n("No Commit")
                                    checkable: true
                                }

                                MenuItem
                                {
                                    width: parent.width
                                    text: i18n("Prune")
                                    checkable: true
                                }

                                MenuItem
                                {
                                    width: parent.width
                                    text: i18n("Tags")
                                    checkable: true

                                }

                                MenuItem
                                {
                                    width: parent.width
                                    text: i18n("Squash")
                                    checkable: true
                                }
                            }

                            MenuSeparator{}

                            Menu
                            {
                                title: i18n("Rebase")

                                MenuItem
                                {
                                    text: i18n("Unset")
                                    checkable: true
                                    autoExclusive: true
                                }

                                MenuItem
                                {
                                    text: i18n("Yes")
                                    checkable: true
                                    autoExclusive: true
                                }

                                MenuItem
                                {
                                    text: i18n("No")
                                    checkable: true
                                    autoExclusive: true
                                }

                                MenuItem
                                {
                                    text: i18n("Only")
                                    checkable: true
                                    autoExclusive: true
                                }
                            }

                            Menu
                            {
                                title: i18n("Fast Forward")

                                MenuItem
                                {
                                    text: i18n("Unset")
                                    checkable: true
                                    autoExclusive: true
                                }

                                MenuItem
                                {
                                    text: i18n("Yes")
                                    checkable: true
                                    autoExclusive: true
                                }

                                MenuItem
                                {
                                    text: i18n("No")
                                    checkable: true
                                    autoExclusive: true
                                }

                                MenuItem
                                {
                                    text: i18n("Only")
                                    checkable: true
                                    autoExclusive: true
                                }
                            }
                        },

                        ToolButton
                        {
                            icon.name: "vcs-pull"
                        },

                        ToolButton
                        {
                            icon.name: "vcs-push"
                        },

                        ToolButton
                        {
                            icon.name: "vcs-merge"
                        },

                        ToolButton
                        {
                            icon.name: "vcs-commit"
                        },
                        ToolButton
                        {
                            icon.name: "vcs-diff"
                        },

                        ToolButtonOp
                        {
                            icon.name: "vcs-stash"
                            text: i18n("Stash")

                            onClicked: runCommand("git stash")
                        },

                        ToolButtonOp
                        {
                            icon.name: "vcs-stash-pop"
                            text: i18n("Stash Pop")

                            onClicked: runCommand("git stash pop")
                        }
                    ]
                }
            }
        }

        Term.Terminal
        {
            id: _terminal
            readOnly: true
            SplitView.maximumHeight: 300
            SplitView.minimumHeight: 100
            SplitView.fillWidth: true
            Maui.Theme.colorSet: Maui.Theme.Header
            Maui.Theme.inherit: false
            session.initialWorkingDirectory: control.project.url.replace("file://", "")
            kterminal.colorScheme: "Adaptive"
        }
    }

    Maui.ProgressIndicator
    {
        width: parent.width
        visible: project.status.code === Bonsai.StatusMessage.Loading
        anchors.bottom: parent.bottom
    }

    Label
    {
        color: "orange"
        text: project.status.code + " ///////////////////////// " + project.status.message
    }

    function openCommitInfoDialog(id)
    {
        _dialogLoader.sourceComponent = _commitInfoDialogComponent
        control.dialog.commitId = id
        control.dialog.open()
    }

    function runCommand(command)
    {
        if(_terminal.session.hasActiveProcess)
        {
            return;
        }

        _terminal.session.sendText(command+ "\n")
    }
}
