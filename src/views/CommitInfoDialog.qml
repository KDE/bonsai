import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import org.mauikit.controls 1.3 as Maui

import org.maui.bonsai 1.0 as Bonsai

Maui.Dialog
{
    id: control

    property string commitId
    readonly property var info : _project.commitAuthor(commitId)

    defaultButtons: false
    persistent: true

    title: i18n("Commit Info")

    maxWidth: 500
    hint: 1

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
    }

    Maui.SectionGroup
    {
        title: i18n("Repo")

        Maui.SectionItem
        {
            label1.text: i18n("Local Path")
            label2.text: _project.url
        }

        Maui.SectionItem
        {
            label1.text: i18n("Branch")
            label2.text: info.branch
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
                    control.commitId = link.replace("hash:", "")
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
                    control.commitId = link.replace("hash:", "")
                }
            }
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
