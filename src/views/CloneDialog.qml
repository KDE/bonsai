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
        columns: 1
        TextField
        {
            id: _urlField
            Layout.fillWidth: true
            placeholderText: i18n("URL")
        }
    }


    Maui.SectionItem
    {
        label1.text: i18n("Name")
        label2.text: i18n("Remote name")
        columns: 1

        TextField
        {
            id: _nameField
            text: _urlField.text.length ? _urlField.text.split("/").pop() : ""
            Layout.fillWidth: true
            placeholderText: i18n("Name")
        }
    }

    Maui.SectionItem
    {
        label1.text: i18n("Location")
        label2.text: i18n("Local location to clone")
        columns: 1

        TextField
        {
            id: _pathField
            Layout.fillWidth: true
            placeholderText: i18n("Path")
            text: Bonsai.Settings.cloneDir
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
        openProject( _pathField.text+ "/" +_nameField.text, _urlField.text)
        control.close()
    }
}
