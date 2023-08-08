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

        onActionFinished:
        {
            root.notify("dialog-warning", ok ? i18n("Action Finished") : i18n("Action Failed"), message)
        }
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
            icon.name: "folder-remote"
            onClicked:
            {
                _dialogLoader.sourceComponent = _remoteDialogComponent
                control.dialog.open()
            }
        },

        Switch{},

        ToolButton
        {
            visible: String(_project.readmeFile).length > 0
            icon.name: "documentinfo"
            onClicked:
            {
                _dialogLoader.sourceComponent = _readmeDialogComponent
                control.dialog.open()
            }
        }
    ]

    Loader
    {
        id: _dialogLoader
    }

    Component
    {
        id: _remoteDialogComponent

        Maui.PopupPage
        {
            persistent: false
            maxWidth: 800
            maxHeight: 600

            page.content: Maui.ProgressIndicator
            {
                width: parent.width
                anchors.bottom: parent.bottom
                visible:  _project.remotesModel.status === Bonsai.RemotesModel.Loading
            }

            Repeater
            {
                model: _project.remotesModel
                delegate:  Maui.SectionGroup
                {
                    title: model.name

                    Maui.SectionItem
                    {
                        label1.text: i18n("Default Branch")
                        label2.text: model.headBranch
                    }

                    Maui.SectionItem
                    {
                        label1.text: i18n("Push Url")
                        label2.text: model.pushUrl
                    }

                    Maui.SectionItem
                    {
                        label1.text: i18n("Fetch Url")
                        label2.text: model.fetchUrl
                    }
                }
            }


        }
    }

    Component
    {
        id: _readmeDialogComponent

        Maui.PopupPage
        {
            maxHeight: 800
            maxWidth: 900

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
        CommitInfoDialog
        {

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

    StackView
    {
        id: _stackView
        anchors.fill: parent

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
                        template.imageSource: _project.logo
                        template.imageSizeHint: Maui.Style.iconSizes.big

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
                }

                Maui.ProgressIndicator
                            {
                                width: parent.width
                                anchors.bottom: parent.bottom
                                visible:  _project.commitsModel.status === Bonsai.CommitsModel.Loading
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
                }

                headBar.leftContent: [

                    ToolButton
                    {
                        visible: _browser.currentPath !== control.project.url
                        icon.name: "go-up"
                        onClicked:
                        {
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

                footBar.rightContent: ToolButtonOp
                    {
                        id: _switchButton
                        icon.name: "vcs-branch"
                        text: i18n("Switch: %1", selectedBranch)
                        property string selectedBranch : _project.currentBranch

                        onClicked:
                        {
                            _project.checkout(selectedBranch)
                        }

                        Repeater
                        {
                            model: _project.allBranches
                            MenuItem
                            {
                                autoExclusive: true
                                checked: _project.currentBranch === text
                                text: modelData
                                onTriggered: _switchButton.selectedBranch = text
                            }
                        }

                        MenuSeparator{}

                        MenuItem
                        {
                            id: _forceOption
                            text: i18n("Force")
                        }

                        MenuItem
                        {
                            id: _remote
                            text: i18n("Remote")
                        }
                    }

                footBar.leftContent: [

                    ToolButtonOp
                    {
                        id: _pullOp
                        icon.name: "vcs-pull"
                        text: i18n("Pull")
                        display: ToolButton.TextBesideIcon


                        property string remote : control.project.headBranch.remote
                        property string branch: control.project.headBranch.name

                        onClicked:
                        {
                            control.project.pull(remote, branch)
                        }

                        Menu
                        {
                            title: i18n("Branch")
                            Repeater
                            {
                                model: control.project.allBranches
                                delegate: MenuItem
                                {
                                    autoExclusive: true
                                    text: modelData
                                    checkable: true
                                    checked: modelData === _pullOp.branch
                                }

                            }
                        }

                        Menu
                        {
                            title: i18n("Remote")
                            Repeater
                            {
                                model: control.project.remoteBranches
                                delegate: MenuItem
                                {
                                    autoExclusive: true
                                    text: modelData
                                    checkable: true
                                    checked: modelData === _pullOp.remote
                                }

                            }
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

                        onClicked: _project.stash()
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
