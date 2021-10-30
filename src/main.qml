import QtQuick 2.15
import QtQuick.Controls 2.15
import org.mauikit.controls 1.3 as Maui
import org.mauikit.filebrowsing 1.3 as FB

Maui.ApplicationWindow {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    headBar.leftContent: ToolButton
    {
        icon.name: "go-previous"
        onClicked: _browser.goBack()
    }

    FB.FileBrowser
    {
        id: _browser
        anchors.fill: parent
        currentPath: FB.FM.homePath()

        onItemClicked:
        {
            const item = currentFMModel.get(index)

                openItem(index)
        }
    }
}
