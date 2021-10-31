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

    headBar.leftContent: ToolButton
    {
        icon.name: "go-previous"
        onClicked: _browser.goBack()
    }

    Bonsai.GitOperations
    {
        id: _gitOperations
    }

    Maui.NewDialog
    {
        id: cloneDialog
        title: i18n("Git URL")
        textEntry.placeholderText: i18n("URL")

        onFinished: _gitOperations.clone(text, "file:///home/camilo/Documents/basket")
    }

    Maui.Holder
    {
        anchors.fill: parent
        title : i18n("Let's Start")
        body: i18n("Open or clone an existing repository, or create a new one.")
        emoji: "qrc:/assets/assets/folder-add.svg"

        Action
        {
            text: "Clone"
            onTriggered: cloneDialog.open()
        }

        Action
        {
            text: "Create"
        }


        Action
        {
            text: "Open"
        }
    }
}
