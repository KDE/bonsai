import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import org.mauikit.controls 1.3 as Maui
import org.mauikit.filebrowsing 1.3 as FB
import org.maui.bonsai 1.0 as Bonsai

Maui.Dialog
{
    id: control
    title: i18n("Git URL")
    maxWidth: 600
    acceptButton.text: i18n("Clone")
    persistent: false

    onRejected: control.close()

    Maui.SectionItem
    {
        label1.text: i18n("Repo URL")
        label2.text: i18n("Remote repo URL")

        TextField
        {
            id: _urlField
            width: parent.parent.width
            placeholderText: i18n("URL")
        }
    }


    Maui.SectionItem
    {
        label1.text: i18n("Name")
        label2.text: i18n("Remote name")

        TextField
        {
            id: _nameField
            text: _urlField.text.length ? _urlField.text.split("/").pop() : ""
            width: parent.parent.width
            placeholderText: i18n("Name")
        }

    }

    Maui.SectionItem
    {
        label1.text: i18n("Location")
        label2.text: i18n("Local location to clone")

        TextField
        {
            id: _pathField
            width: parent.parent.width
            placeholderText: i18n("Path")
            text: FB.FM.homePath()
        }

    }

    Maui.SectionItem
    {
        label1.text: i18n("Bare")
        label2.text: i18n("Create a bare repo")

        Switch
        {
            id: _bareSwitch
        }

    }

    Maui.SectionItem
    {
        label1.text: i18n("Recursive")
        label2.text: i18n("Clone submodules")

        Switch
        {
            id: _recursiveSwitch
        }

    }

    onAccepted:
    {
        gitOperations.clone(_urlField.text, _pathField.text, _nameField.text, _bareSwitch.checked, _recursiveSwitch.checked)
    }
}
