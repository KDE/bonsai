import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.3

import org.mauikit.controls 1.3 as Maui
import org.mauikit.filebrowsing 1.3 as FB

import org.maui.bonsai 1.0 as Bonsai

Maui.SettingsDialog
{
    id: control

    Maui.SectionGroup
    {
        title: i18n ("Global")

        Maui.SectionItem
        {
            label1.text: i18n("Clone Directory")
            label2.text: i18n("Pick a default clone directory.")
            columns: 1

            RowLayout
            {
                spacing: parent.columnSpacing
                Layout.fillWidth: true

                TextField
                {
                    Layout.fillWidth: true
                    placeholderText: i18n("New location.")
                    text: Bonsai.Settings.cloneDir
                }

                Button
                {
                    text: i18n("Pick")
                    onClicked: pickLocation()
                }
            }
        }
    }


    function pickLocation()
    {
        _fmDialogLoader.sourceComponent = _openFileDialogComponent
        fmDialog.singleSelection = true
        fmDialog.settings.onlyDirs = true
        fmDialog.callback = function(paths)
        {
            Bonsai.Settings.cloneDir = paths[0]
        }

        fmDialog.open()
    }

}
